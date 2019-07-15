
#include <jni.h>
#include <string>
#include "pthread.h"

pthread_t pthread;

extern "C"
{
#include <libavformat/avformat.h>
}

#include "androidplatform/MyLog.h"

AVFormatContext *ifmt_ctx = NULL;
AVFormatContext *ofmt_ctx = NULL;
AVOutputFormat *ofmt = NULL;
AVPacket pkt;
double end_seconds;   //结束时间


extern "C"
JNIEXPORT jboolean JNICALL
Java_pictrue_com_reiniot_ffmpeg_FFmpeg_snapVideo(JNIEnv *env, jobject instance, jdouble startTime,
                                               jstring source_, jstring result_) {
    const char *in_filename = env->GetStringUTFChars(source_, 0);
    const char *out_filename = env->GetStringUTFChars(result_, 0);
    av_register_all();

    if (ifmt_ctx == NULL) {
        ifmt_ctx = avformat_alloc_context();
    }

    if (ofmt_ctx == NULL) {
        ofmt_ctx = avformat_alloc_context();
    }
    int errorCode = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL);
    if (errorCode != 0){
        LOGE("open file error");
        if (ofmt_ctx != NULL) {
            avformat_close_input(&ofmt_ctx);
            avformat_free_context(ofmt_ctx);
            ofmt_ctx = NULL;
        }
        if (ifmt_ctx != NULL) {
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ifmt_ctx);
            ifmt_ctx = NULL;
        }
    }

    //本质上调用了avformat_alloc_context、av_guess_format这两个函数，即创建了输出上下文，又根据输出文件后缀生成了最适合的输出容器
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    ofmt = ofmt_ctx->oformat;
    int ret;
    LOGE("ifmt_ctx = %p",ifmt_ctx);

    //复制每个流参数到输出文件中
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            if (ofmt_ctx != NULL) {
                avformat_close_input(&ofmt_ctx);
                avformat_free_context(ofmt_ctx);
                ofmt_ctx = NULL;
            }
            if (ifmt_ctx != NULL) {
                avformat_close_input(&ifmt_ctx);
                avformat_free_context(ifmt_ctx);
                ifmt_ctx = NULL;
            }
            return false;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    //
    int open = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
    LOGE("open = %d",open);

    // 写头信息
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("ffmpeg write header failed");
        if (ofmt_ctx != NULL) {
            avformat_close_input(&ofmt_ctx);
            avformat_free_context(ofmt_ctx);
            ofmt_ctx = NULL;
        }
        if (ifmt_ctx != NULL) {
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ifmt_ctx);
            ifmt_ctx = NULL;
        }
        return false;
    }

   //跳转到指定帧
    ret = av_seek_frame(ifmt_ctx, -1, startTime * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        LOGE("ffmpeg seek frame failed");
        return false;
    }
    // 根据流数量申请空间，并全部初始化为0
    int64_t *dts_start_from = (int64_t *)(malloc(sizeof(int64_t) * ifmt_ctx->nb_streams));
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    int64_t *pts_start_from =  (int64_t *)(malloc(sizeof(int64_t) * ifmt_ctx->nb_streams));
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    end_seconds = ifmt_ctx->duration / AV_TIME_BASE;

    LOGE("all time is %f",end_seconds);

    while (1) {
        AVStream *in_stream, *out_stream;

        //读取数据
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("read frame failed");
            av_packet_unref(&pkt);
            break;
        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        double end  = in_stream->duration * av_q2d(in_stream->time_base);
        double current = av_q2d(in_stream->time_base) * pkt.pts;
       // LOGE("ent time = %f,current time =%f",end,current);

        // 时间超过要截取的时间，就退出循环
        if (current > end) {
            LOGE("current > end return");
            av_packet_unref(&pkt);
            break;
        }

        // 将截取后的每个流的起始dts 、pts保存下来，作为开始时间，用来做后面的时间基转换
        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }
        /**
         * 接看代码, 它的作用是计算 "a * b / c" 的值并分五种方式来取整.
         *  则是将以 "时钟基c" 表示的 数值a 转换成以 "时钟基b" 来表示。
         */

        // 时间基转换
        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF );
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF );
        if (pkt.pts < 0) {
            pkt.pts = 0;
        }
        if (pkt.dts < 0) {
            pkt.dts = 0;
        }

        pkt.duration = (int) av_rescale_q((int64_t) pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        int64_t  pts = pkt.pts;
        int64_t  dts = pkt.pts;

        //一帧视频播放时间必须在解码时间点之后，当出现pkt.pts < pkt.dts时会导致程序异常，所以我们丢掉有问题的帧，不会有太大影响。
        if (pkt.pts < pkt.dts) {
            av_packet_unref(&pkt);
            continue;
        }

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("write failed return");
            av_packet_unref(&pkt);
            continue;
        }

        av_packet_unref(&pkt);
    }

    //释放资源
    free(pts_start_from);
    free(dts_start_from);

   //写文件尾信息
    av_write_trailer(ofmt_ctx);
    LOGE("释放 封装格式上下文");
    if (ofmt_ctx != NULL) {
        avformat_close_input(&ofmt_ctx);
        avformat_free_context(ofmt_ctx);
        ofmt_ctx = NULL;
    }
    if (ifmt_ctx != NULL) {
        avformat_close_input(&ifmt_ctx);
        avformat_free_context(ifmt_ctx);
        ifmt_ctx = NULL;
    }
    return true;
}