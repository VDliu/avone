#include "MyFFmpeg.h"
#include "../androidplatform/MyLog.h"
#include <unistd.h> // sleep 的头文件

MyFFmpeg::MyFFmpeg(PlayStatus *playStatus, const char *url,CallJava *callJava) {
    this->callJava = callJava;
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
    int errorCode = -1;
    if ((errorCode = avformat_open_input(&pFormatCtx, url, NULL, NULL) ) && errorCode != 0) {
        LOGE("open resource  faild ::->,%s,errorCode = %d", url,errorCode);
        char * buf = (char *)malloc(1024);
        av_strerror(-1, buf, 1024);
        LOGE("error with msg = %s",buf);
        free(buf);
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
                myAudio = new MyAudio(i, avStream->codecpar, this->playStatus,avStream->codecpar->sample_rate,callJava);
                //总时长,换算成单位秒
                myAudio->duration = pFormatCtx->duration / AV_TIME_BASE;
                LOGE("duration = %d ",myAudio->duration );
                myAudio->time_base = avStream->time_base;
                //num=1,den = 14112000
                LOGE("time base num=%d,den = %d",myAudio->time_base.num,myAudio->time_base.den);
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

    //7.找到音频流解码器上下文
    myAudio->codecContext = avcodec_alloc_context3(audioCodec);
    if (myAudio->codecContext == NULL) {
        LOGE("alloc av codeContext failed");
        return;
    }

    //8.把音频AVCodecParameters 复制到音频AVCodecContext中
    if (avcodec_parameters_to_context(myAudio->codecContext, myAudio->codecParameters) < 0) {
        LOGE("cope parameters to  codecContext failed");
        return;
    }

    //9.打开音频解码器
    if (avcodec_open2(myAudio->codecContext, audioCodec, NULL) < 0) {
        LOGE("open audio code failed");
        return;
    }

    //回调到java层
    callJava->onCallParpared(CHILD_THREAD);
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
    //单独线程重采样
    myAudio->play();
    //延时 模拟加载
    sleep(5);
    while (playStatus != NULL && !playStatus->exit) {
        /**
         * 1.AVPacket是FFmpeg中很重要的一个数据结构，它保存了解复用（demuxer)之后
         * 解码（decode）之前的数据（仍然是压缩后的数据）和关于这些数据的一些附加的信息，
         * 如显示时间戳（pts），解码时间戳（dts）,数据时长（duration），所在流媒体的索引（stream_index）等等。
         *
         * 2.AVFrame 为解压缩的原始数据
         *
         */
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            //判断是否是音频流
            if (avPacket->stream_index == myAudio->streamIndex) {
                myAudio->queue->putAvPacket(avPacket);
            } else {//不是音频流
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }

        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            while (playStatus != NULL && !playStatus->exit) {
                //如果队列中还有数据，需要将数据取完以后再退出
                if (myAudio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
        }

    }


    {
        LOGD("解码完成");
    }

}

void MyFFmpeg::pause() {
    if (myAudio != NULL){
        myAudio->pause();
    }

}

void MyFFmpeg::resume() {
    if (myAudio != NULL){
        myAudio->resume();
    }
}
