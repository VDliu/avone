#include "MyFFmpeg.h"
#include "../androidplatform/MyLog.h"

MyFFmpeg::MyFFmpeg(PlayStatus *playStatus,PrepareCallBack *callBack, const char *url) {
    this->callBack = callBack;
    this->url = url;
    this->playStatus = playStatus;
}

void *decodeThreadCall(void *data) {
    MyFFmpeg *fmpeg = (MyFFmpeg *) data;
    fmpeg->decodeFFmepg();
    pthread_exit(&fmpeg->decode_thread);

}

void MyFFmpeg::prepare() {
    pthread_create(&decode_thread, nullptr, decodeThreadCall, this);
}

//真正解码
void MyFFmpeg::decodeFFmepg() {
    //1.调用FFmpeg的注册协议、格式与编解码器的方法，确保所有的格式与编解码器都被注册到了FFmpeg框架
    av_register_all();
    //2.需要用到网络的操作，那么也应该将网络协议部分注册到FFmpeg框架
    avformat_network_init();


    if (pFormatCtx == NULL) {
        pFormatCtx = avformat_alloc_context();
    }
    //3.打开媒体资源
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        LOGE("open resource  faild ::->,%s", url);
        return;
    }
    //4.找到流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("find stream failed");
        return;
    }

    /**
     * 上一步中已打开了媒体文件，相当于打开了一根电线，这根电线里面其实还有一条红色的线和一条蓝色的线，
     * 这就和媒体文件中的流非常类似了，红色的线代表音频流，蓝色的线代表视频流。
     * 所以这一步我们就要寻找出各个流，然后找到流中对应的解码器，并且打开它。
     */

    //5.寻找音频流

    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        AVStream *avStream = pFormatCtx->streams[i];
        if (AVMEDIA_TYPE_AUDIO == avStream->codecpar->codec_type) {
            //找到了音频流
            if (myAudio == NULL) {
                myAudio = new MyAudio(i, avStream->codecpar,this->playStatus);
            }
        }
    }

    if (myAudio == NULL) {
        LOGE("get audio stream or stream info failed");
        return;
    }

    //6.找到音频流的解码器
    AVCodec *audioCodec = avcodec_find_decoder(myAudio->codecParameters->codec_id);
    if (audioCodec == NULL) {
        LOGE("find audio code failed");
        return;
    }

    //7.找到解码器上下文
    myAudio->codecContext = avcodec_alloc_context3(audioCodec);
    if (myAudio->codecContext == NULL) {
        LOGE("alloc av codeContext failed");
        return;
    }

    //8.把AVCodecParameters 复制到AVCodecContext中
    if (avcodec_parameters_to_context(myAudio->codecContext, myAudio->codecParameters) < 0) {
        LOGE("cope parameters to  codecContext failed");
        return;
    }

    //9.打开解码器
    if (avcodec_open2(myAudio->codecContext, audioCodec, NULL) < 0) {
        LOGE("open audio code failed");
        return;
    }

    callBack->onprepared(JavaListener::CHILD_THREAD);
}

MyFFmpeg::~MyFFmpeg() {
    if (url != NULL) {
        delete url;
    }
}

void MyFFmpeg::start() {
    if (myAudio == NULL) {
        LOGE("audio is null");
        return;
    }

    int count = 0;

    while (1) {
        /**
         * AVPacket是FFmpeg中很重要的一个数据结构，它保存了解复用（demuxer)之后
         * 解码（decode）之前的数据（仍然是压缩后的数据）和关于这些数据的一些附加的信息，
         * 如显示时间戳（pts），解码时间戳（dts）,数据时长（duration），所在流媒体的索引（stream_index）等等。
         */
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            //判断是否是音频流
            if (avPacket->stream_index == myAudio->streamIndex) {
                count++;
                LOGE("this is the frame = %d",count);
                 myAudio->queue->putAvPacket(avPacket);
            }else{//不是音频流
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }

        } else {//avpack读取失败
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            break;
        }
    }

    //模拟出队
    while (myAudio->queue->getQueueSize() > 0)
    {
        AVPacket *packet = av_packet_alloc();
        myAudio->queue->getAvPacket(packet);
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }

    {
        LOGD("解码完成");
    }

}
