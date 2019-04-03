
/**
  *2019/4/2.
  *
 */

#ifndef AVONE_MYAUDIO_H
#define AVONE_MYAUDIO_H

#include "AVPacketQueue.h"
#include "PlayStatus.h"
#include "pthread.h"
extern "C"
{
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <SLES/OpenSLES.h>
    #include <SLES/OpenSLES_Android.h>
}

class MyAudio {

public:
    int streamIndex = -1;
    AVCodecParameters * codecParameters = NULL;
    AVCodecContext *codecContext = NULL;
    AVPacketQueue *queue = NULL;
    PlayStatus *playstatus = NULL;

    //重采样
    pthread_t pthread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = -1;
    uint8_t *buffer = NULL;
    int data_size = 0;

    //opensl es--------------
    SLuint32 sample_rate = 44100;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

public:
    MyAudio(int index,AVCodecParameters * codecPar,PlayStatus *playStatus,SLuint32 sampleRate);
    ~MyAudio();

    void play();
    int resampleAudio();

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(SLuint32 sample_rate);

};


#endif //AVONE_MYAUDIO_H
