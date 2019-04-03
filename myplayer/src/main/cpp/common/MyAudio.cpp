
/**
  *2019/4/2.
  *
 */

#include "MyAudio.h"
#include "../androidplatform/MyLog.h"

MyAudio::MyAudio(int index, AVCodecParameters *codecPar, PlayStatus *playStatus,SLuint32 sampleRate) {
    this->streamIndex = index;
    this->codecParameters = codecPar;
    this->queue = new AVPacketQueue(playStatus);
    this->playstatus = playStatus;
    this->sample_rate = sampleRate;
    buffer = (uint8_t *) av_malloc(44100 * 2 * 2);
}

MyAudio::~MyAudio() {
    if (queue != NULL) {
        delete queue;
    }

    if (buffer != NULL) {
        free(buffer);
    }
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

int MyAudio::resampleAudio() {
    while (playstatus != NULL && !playstatus->exit) {
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
        //从解码器中接收解码后的avframe数据
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
            int nb = swr_convert(
                    swr_ctx,
                    &buffer, //转码后输出pcm的数据
                    avFrame->nb_samples,//输出采样个数
                    (const uint8_t **) avFrame->data,//原始数据
                    avFrame->nb_samples);//输入采样个数

            LOGD("nb sample is %d", nb);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //fwrite要写入内容的单字节数,要进行写入size字节的数据项的个数
           // fwrite(buffer, 1, data_size, outFile);

            LOGD("data_size is %d", data_size);
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
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

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    MyAudio *audio = (MyAudio *) context;
    if(audio != NULL)
    {
        //播放重采样数据
        int buffersize = audio->resampleAudio();
        LOGE("bufferSize = %d,playstatus->exit = %d,queue size = %d",buffersize,audio->playstatus->exit,audio->queue->getQueueSize());
        if(buffersize > 0)
        {
            (* audio-> pcmBufferQueue)->Enqueue( audio->pcmBufferQueue, (char *) audio-> buffer, buffersize);
        }
    }
}

void MyAudio::initOpenSLES() {

    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 1, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);


}

SLuint32 MyAudio::getCurrentSampleRateForOpensles(SLuint32 sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
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
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

