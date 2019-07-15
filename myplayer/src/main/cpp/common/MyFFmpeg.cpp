#include "MyFFmpeg.h"
#include "../androidplatform/MyLog.h"
#include <unistd.h> // sleep 的头文件

extern "C" {
#include <libavutil/time.h>
}

MyFFmpeg::MyFFmpeg(PlayStatus *playStatus, const char *url, CallJava *callJava) {
    this->callJava = callJava;
    this->url = url;
    this->playStatus = playStatus;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
    exit = false;
}

void *decodeThreadCall(void *data) {
    MyFFmpeg *fmpeg = (MyFFmpeg *) data;
    fmpeg->decodeFFmepg();
    pthread_exit(&fmpeg->decode_thread);

}

void MyFFmpeg::prepare() {
    pthread_create(&decode_thread, nullptr, decodeThreadCall, this);
}

int decodeFFmepgCallback(void *ctx) {
    MyFFmpeg *fFmpeg = (MyFFmpeg *) ctx;
    if (fFmpeg->playStatus->exit) { //在release中设置为true以后，退出阻塞
        return AVERROR_EOF;
    }
    return 0; //否则 继续等待,返回0表示继续等待
}

//真正解码
void MyFFmpeg::decodeFFmepg() {
    //添加初始化锁
    pthread_mutex_lock(&init_mutex);

    //1.调用FFmpeg的注册协议、格式与编解码器的方法，确保所有的格式与编解码器都被注册到了FFmpeg框架
    av_register_all();
    //2.需要用到网络的操作，那么也应该将网络协议部分注册到FFmpeg框架
    avformat_network_init();


    if (pFormatCtx == NULL) {
        pFormatCtx = avformat_alloc_context();
    }

    pFormatCtx->interrupt_callback.callback = decodeFFmepgCallback;//超时回调
    pFormatCtx->interrupt_callback.opaque = this;

    //3.打开媒体资源
    int errorCode = -1;
    if ((errorCode = avformat_open_input(&pFormatCtx, url, NULL, NULL)) && errorCode != 0) {
        LOGE("open resource  faild ::->,%s,errorCode = %d", url, errorCode);
        char *buf = (char *) malloc(1024);
        av_strerror(-1, buf, 1024);
        LOGE("error with msg = %s", buf);
        free(buf);
        pthread_mutex_unlock(&init_mutex);
        exit = true;
        callJava->onCallError(CHILD_THREAD, -1, "can not open url");
        return;
    }
    //4.找到流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("find stream failed");
        pthread_mutex_unlock(&init_mutex);
        exit = true;
        callJava->onCallError(CHILD_THREAD, -1, "can not find stream info");
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
                myAudio = new MyAudio(i, avStream->codecpar, this->playStatus,
                                      avStream->codecpar->sample_rate, callJava);
                //总时长,换算成单位秒，pFormatCtx->duration表示有
                myAudio->duration = pFormatCtx->duration / AV_TIME_BASE;
                LOGE("duration = %d ", myAudio->duration);
                //time_base ?
                myAudio->time_base = avStream->time_base;
                //num=1,den = 14112000
                LOGE("time base num=%d,den = %d", myAudio->time_base.num, myAudio->time_base.den);
            }
        } else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            //找到视频流
            if (myVideo == NULL) {
                myVideo = new MyVideo(playStatus, callJava);
                myVideo->streamIndex = i;
                myVideo->codecpar = pFormatCtx->streams[i]->codecpar;
                myVideo->time_base = pFormatCtx->streams[i]->time_base;
            }
        }
    }

    if (myAudio != NULL) {
        getCodecContext(myAudio->codecParameters, &myAudio->codecContext);
    }
    if (myVideo != NULL) {
        getCodecContext(myVideo->codecpar, &myVideo->avCodecContext);
    }

    if (myAudio == NULL) {
        LOGE("get audio stream or stream info failed");
        pthread_mutex_unlock(&init_mutex);
        exit = true;
        return;
    }

//    //6.找到音频流的解码器
//    AVCodec *audioCodec = avcodec_find_decoder(myAudio->codecParameters->codec_id);
//    if (audioCodec == NULL) {
//        LOGE("find audio code failed");
//        pthread_mutex_unlock(&init_mutex);
//        exit = true;
//        return;
//    }
//
//    //7.找到音频流解码器上下文
//    myAudio->codecContext = avcodec_alloc_context3(audioCodec);
//    if (myAudio->codecContext == NULL) {
//        LOGE("alloc av codeContext failed");
//        pthread_mutex_unlock(&init_mutex);
//        exit = true;
//        return;
//    }
//
//    //8.把音频AVCodecParameters 复制到音频AVCodecContext中
//    if (avcodec_parameters_to_context(myAudio->codecContext, myAudio->codecParameters) < 0) {
//        LOGE("cope parameters to  codecContext failed");
//        pthread_mutex_unlock(&init_mutex);
//        exit = true;
//        return;
//    }
//
//    //9.打开音频解码器
//    if (avcodec_open2(myAudio->codecContext, audioCodec, NULL) < 0) {
//        LOGE("open audio code failed");
//        pthread_mutex_unlock(&init_mutex);
//        exit = true;
//        return;
//    }

    if (callJava != NULL) {
        if (playStatus != NULL && !playStatus->exit) {
            callJava->onCallParpared(CHILD_THREAD);
        } else {
            exit = true;
        }
    }

    pthread_mutex_unlock(&init_mutex);
}

MyFFmpeg::~MyFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);

    if (url != NULL) {
        delete url;
    }
}

//start的时候开了一个子线程
void MyFFmpeg::start() {
    if (myAudio == NULL) {
        LOGE("audio is null");
        return;
    }
    //单独线程重采样，然后播放
    myAudio->play();
    myVideo->play();
    //从数据中读取avpacket加入队列中
    while (playStatus != NULL && !playStatus->exit) {
        /**
         * 1.AVPacket是FFmpeg中很重要的一个数据结构，它保存了解复用（demuxer)之后
         * 解码（decode）之前的数据（仍然是压缩后的数据）和关于这些数据的一些附加的信息，
         * 如显示时间戳（pts），解码时间戳（dts）,数据时长（duration），所在流媒体的索引（stream_index）等等。
         *
         * 2.AVFrame 为解压缩的原始数据
         *
         */
        if (playStatus->isSeeking) {
            continue;
        }

        //队列中最多保存40个packet,
        // 1.防止队列数据过多
        // 2.防止在读取队列完毕后，队列中包含很多数据吗，此时seek会清空数据，这个时候会导致播放退出
        // 3.比如在暂停状态，播放器没有去取avpacket这个时候不设置阈值会导致内存不断增大
        if (myAudio->queue->getQueueSize() > 40) {
            continue;
        }


        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(pFormatCtx, avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (ret == 0) {
            //判断是否是音频流
            if (avPacket->stream_index == myAudio->streamIndex) {
                myAudio->queue->putAvPacket(avPacket);
            } else if (avPacket->stream_index == myVideo->streamIndex) {
                myVideo->queue->putAvPacket(avPacket);
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


    LOGD("解码完成");
    if (callJava != NULL) {
        //此时opensl还没播放完毕？？
        callJava->onCallCompelet(CHILD_THREAD);
    }
    exit = true;

}

void MyFFmpeg::pause() {
    if (myAudio != NULL) {
        myAudio->pause();
    }

}

void MyFFmpeg::resume() {
    if (myAudio != NULL) {
        myAudio->resume();
    }
}

void MyFFmpeg::release() {

    //在播放的时候设置playStatus->exit = true;
    //start方法会把exit置位true 从而就可以退出了
    playStatus->exit = true;

    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    //如果是在prepare阶段，加载网络数据的时候 还没加载成功，这个时候需要停止退出
    //等待10 秒强制退出
    while (!exit) {
        LOGE("exit = %d", exit);
        if (sleepCount > 1000) {
            exit = true;
        }

        LOGE("wait ffmpeg  exit %d", sleepCount);
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }

    LOGE("释放 Audio");

    if (myAudio != NULL) {
        myAudio->release(); //可使opensl中调用stop停止播放pcm数据
        delete (myAudio);
        myAudio = NULL;
    }

    LOGE("释放 myVideo");
    if(myVideo != NULL)
    {
        myVideo->release();
        delete(myVideo);
        myVideo = NULL;
    }

    LOGE("释放 封装格式上下文");
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }

    LOGE("释放 playstatus");
    if (playStatus != NULL) {
        playStatus = NULL;
    }

    pthread_mutex_unlock(&init_mutex);

}

void MyFFmpeg::seek(int sec) {
    if (myAudio == NULL || myAudio->duration <= 0) {
        return;
    }

    if (sec > 0 && sec <= myAudio->duration) {
        if (!playStatus->isSeeking) {
            playStatus->isSeeking = true;
            //清空queue
            myAudio->queue->clearQueue();
            myAudio->clock = 0;
            myAudio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = sec * AV_TIME_BASE;
            //移动到文件seek处
            avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playStatus->isSeeking = false;
        }
    }

}

void MyFFmpeg::setVolume(int percent) {
    if (myAudio != NULL) {
        myAudio->setVolume(percent);
    }

}

void MyFFmpeg::setMute(int mute) {
    if (myAudio != NULL) {
        myAudio->setMute(mute);
    }
}

void MyFFmpeg::setPitch(float pitch) {
    if (myAudio != NULL) {
        myAudio->setPitch(pitch);
    }
}

void MyFFmpeg::setSpeed(float speed) {
    if (myAudio != NULL) {
        myAudio->setSpeed(speed);
    }
}

int MyFFmpeg::getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext) {
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
    if (!dec) {
        LOGE("can not find decoder");
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    *avCodecContext = avcodec_alloc_context3(dec);
    if (*avCodecContext == NULL) {
        LOGE("can not alloc new decodecctx");
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if (avcodec_parameters_to_context(*avCodecContext, codecpar) < 0) {
        callJava->onCallError(CHILD_THREAD, 1005, "ccan not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if (avcodec_open2(*avCodecContext, dec, 0) != 0) {
        LOGE("cant not open audio strames");
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}

