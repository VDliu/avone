
/**
  *2019/4/2.
  *
 */

#include "MyAudio.h"
#include "../androidplatform/MyLog.h"

MyAudio::MyAudio(int index, AVCodecParameters *codecPar, PlayStatus *playStatus,
                 SLuint32 sampleRate, CallJava *callJava) {
    this->streamIndex = index;
    this->codecParameters = codecPar;
    this->queue = new AVPacketQueue(playStatus);
    this->playstatus = playStatus;
    this->sample_rate = sampleRate;
    this->callJava = callJava;
    //采样率 * 声道数 * 采样点的大小（字节）
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);//相当于 sample_rate * 2 * 2 = n， n个 * 1字节元素的数组
    this->soundTouch = new SoundTouch();
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(
            sample_rate * 2 * 2)); //相当于 sample_rate * 2  = n, n个 * 2字节元素的数组

    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
}

MyAudio::~MyAudio() {

}


//重采样
void *decodePlay(void *data) {
    MyAudio *audio = (MyAudio *) data;
    audio->initOpenSLES();
    pthread_exit(&audio->pthread_play);
}

void MyAudio::play() {
    pthread_create(&pthread_play, NULL, decodePlay, this);
}

int MyAudio::resampleAudio(void **buf) {
    while (playstatus != NULL && !playstatus->exit) {

        if (queue->getQueueSize() == 0) {
            //显示加载数据
            if (!playstatus->isLoading) {
                callJava->onCallLoad(CHILD_THREAD, true);
                playstatus->isLoading = true;
                LOGE("loading ----");
            }
            continue;

        } else {
            if (playstatus->isLoading) {
                playstatus->isLoading = false;
                callJava->onCallLoad(CHILD_THREAD, false);
                LOGE("loading ----ok");
            }
        }


        avPacket = av_packet_alloc();
        if (queue->getAvPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        //把avPacket放到解码器中进行解码
        ret = avcodec_send_packet(codecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        avFrame = av_frame_alloc();
        //从解码器中接收解码后的avframe数据,获取到解码数据
        ret = avcodec_receive_frame(codecContext, avFrame);
        //重采样
        if (ret == 0) {
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                //有声道数，但是没有声道布局，设置声道布局
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                //没有声道数，但是有声道布局，设置声道数
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            //重采样上下文
            SwrContext *swr_ctx = NULL;

            // 设置冲采样的相关参数
            // 该函数的输入和输出采样率需要相同
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO, //输出的声道布局
                    AV_SAMPLE_FMT_S16,//输出的重采样位数
                    avFrame->sample_rate,//输出的采样率
                    avFrame->channel_layout,//输入声道布局
                    (AVSampleFormat) avFrame->format,//输入的重采样位数
                    avFrame->sample_rate,//输入的采样率
                    NULL, NULL
            );

            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if (swr_ctx != NULL) {
                    swr_free(&swr_ctx);
                }
                continue;
            }

            //返回每一个通道的采样个数
            // pcm大小= 采样个数 * 通道数 * 采样点的大小
            //一般情况下 nb = avFrame->nb_samples
            nb = swr_convert(
                    swr_ctx,
                    &buffer, //转码后输出pcm的数据(pcm裸流)
                    avFrame->nb_samples,//输出采样个数 需要一致
                    (const uint8_t **) avFrame->data,//原始数据
                    avFrame->nb_samples);//输入采样个数 需要一致

            LOGD("nb sample is %d", nb);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            //获取到本次冲采样大小 data_size
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //fwrite要写入内容的单字节数,要进行写入size字节的数据项的个数
            // fwrite(buffer, 1, data_size, outFile);
            LOGD("avFrame--pts =%ld", avFrame->pts);
            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            *buf = buffer;//获取到重采样的数据

            LOGD("data_size is %d,cuurent time = %f", data_size, clock);
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            //每当冲采样成功一次，交给opensl es播放，播放完毕以后再继续重新采样
            break;

        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }

    }

    return data_size;
}

//opensl会自动调用pcmBufferCallBack去取出数据
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    MyAudio *audio = (MyAudio *) context;
    if (audio != NULL) {
        //播放重采样数据
        int buffersize = audio->getSoundTouchData();
        LOGD("bufferSize = %d,playstatus->exit = %d,queue size = %d", buffersize,
             audio->playstatus->exit, audio->queue->getQueueSize());
        if (buffersize > 0) {
            //pts时间+当前帧播放需要的时间
            audio->clock += buffersize / ((double) (audio->sample_rate * 2 * 2));
            if (audio->clock - audio->last_time >= 0.1) {
                audio->last_time = audio->clock;
                //回调应用层
                audio->callJava->onCallTimeInfo(CHILD_THREAD, audio->clock, audio->duration);
            }
            //把要播放的buffer入队，播放完毕后会自动调用pcmBufferCallBack方法继续获取buffer
            (*audio->pcmBufferQueue)->Enqueue(audio->pcmBufferQueue, (char *) audio->sampleBuffer,
                                              buffersize * 2 * 2);
        }
    }
}

void MyAudio::initOpenSLES() {

    SLresult result;
    //创建引擎
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率，设置不对可能会导致播放声速变化
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 3,
                                       ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//  得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    //得到音量接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);

    //获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);

//   注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//   获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    setVolume(volume_percent);
    //调用播放方法
    pcmBufferCallBack(pcmBufferQueue, this);
}

SLuint32 MyAudio::getCurrentSampleRateForOpensles(SLuint32 sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void MyAudio::pause() {

    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }

}

void MyAudio::resume() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void MyAudio::stop() {
    if (pcmPlayerPlay != NULL) {
        //停止调用入队函数
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void MyAudio::release() {
    stop();
    last_time = 0;
    clock = 0;

    if (queue != NULL) {
        delete queue;
    }

    if (buffer != NULL) {
        free(buffer);
    }

    //释放播放器
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }

    //释放混音器
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    //释放引擎
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    //释放解码上下文
    if (codecContext != NULL) {
        avcodec_free_context(&codecContext);
        codecContext = NULL;
    }

    //ffmpeg传进来的 不给与清空
    if (playstatus != NULL) {
        playstatus = NULL;
    }

    //ffmpeg传进来的 不给与清空
    if (callJava != NULL) {
        callJava = NULL;
    }

    if (soundTouch != NULL) {
        delete soundTouch;
        soundTouch = NULL;
    }

    if (sampleBuffer != NULL) {
        free(sampleBuffer);
        sampleBuffer = NULL;
    }
}

void MyAudio::setVolume(int percent) {
    volume_percent = percent;
    if (pcmVolumePlay != NULL) {
        if (percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void MyAudio::setMute(int mute) {
    this->mute = mute;
    if (pcmMutePlay != NULL) {
        if (mute == 0)//right
        {   // 0 右声道  1 左声道
            // ture 表示打开  false表示关闭对应的通道
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        } else if (mute == 1)//left
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        } else if (mute == 2)//center
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
    }

}

int MyAudio::getSoundTouchData() {
//    while (playstatus != NULL && !playstatus->exit) {
//        out_buffer = NULL;
//        if (finish) {
//            finish = false;
//            //得到重采样后的数据,bit数据
//            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
//            if (data_size > 0) {
//                //由于ffmpeg得到的是uint8的数据，soundtouch 的输入数据类型为short型
//                //为16位，因此需要做一下转化，把两个8位 拼接成一个16位
//                for (int i = 0; i < data_size / 2 + 1; i++) {
//                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
//                }
//                //输入数据到soundtouch，第二个参数为采样点个数，虽然数据由8位转换到了16位
//                //此处的采样点个数没有发生变化
//                soundTouch->putSamples(sampleBuffer, nb);
//                //返回采样点个数，第二个参数为一次最多可以接收的采样点个数，需要多次获取，才能够把putSample放进去的数据获取完毕
//                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
//                LOGE("+++++++++++++++++++++++++size of num1  = %d,size/4 = %d",num,data_size / 4);
//            } else {
//                soundTouch->flush();
//            }
//        }
//
//
//        if (num == 0) {
//            finish = true;
//            LOGE("----continue--------");
//            continue;
//        } else {
//            if (out_buffer == NULL) {
//                LOGE("================is finish =%d ,num = %d",finish,num);
//                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
//                LOGE("2222222222222222222222222222size of num2  = %d,size/4 = %d",num,data_size / 4);
//                if (num == 0) {
//                    finish = true;
//                    continue;
//                }
//
//            }
//            LOGE("return --- num = %d,finish = %d",num,finish);
//            return num;
//        }
//
//    }
//    return 0;



    while (playstatus != NULL && !playstatus->exit) {
        out_buffer = NULL;
        //得到重采样后的数据,bit数据
        data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
        if (data_size > 0) {
            //由于ffmpeg得到的是uint8的数据，soundtouch 的输入数据类型为short型
            //为16位，因此需要做一下转化，把两个8位 拼接成一个16位
            for (int i = 0; i < data_size / 2 + 1; i++) {
                sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
            }
            //输入数据到soundtouch，第二个参数为采样点个数，虽然数据由8位转换到了16位
            //此处的采样点个数没有发生变化
            soundTouch->putSamples(sampleBuffer, nb);
            //返回采样点个数，第二个参数为一次最多可以接收的采样点个数，需要多次获取，才能够把putSample放进去的数据获取完毕
            //此处一次性读取完毕
            num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            if (num > 0) {
                return num;
            }
        } else {
            soundTouch->flush();
        }
    }
    return 0;
}

void MyAudio::setPitch(float pitch) {
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void MyAudio::setSpeed(float speed) {
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}




