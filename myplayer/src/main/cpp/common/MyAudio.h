
/**
  *2019/4/2.
  *
 */

#ifndef AVONE_MYAUDIO_H
#define AVONE_MYAUDIO_H

#include "AVPacketQueue.h"
#include "PlayStatus.h"
#include "pthread.h"
#include "../androidplatform/callback/OnLoadCallBack.h"
#include "../androidplatform/callback/CallJava.h"
#include "SoundTouch.h"
using namespace soundtouch;

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
    //callback
    CallJava *callJava;

    //重采样
    pthread_t pthread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = -1;
    uint8_t *buffer = NULL;
    int data_size = 0; //字节数

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

    //声音
    SLVolumeItf  pcmVolumePlay = NULL;

    //声道
    SLMuteSoloItf  pcmMutePlay = NULL;

    int volume_percent = 50 ;

    int mute;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;
    //opensl es--------------

    //时间显示
    int duration = 0;
    AVRational time_base;
    double clock;//总的播放时长
    double now_time;//当前frame时间
    double last_time; //上一次调用时间
    //时间显示

    //变速 变调
    //SoundTouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool finish = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;

public:
    MyAudio(int index,AVCodecParameters * codecPar,PlayStatus *playStatus,SLuint32 sampleRate,CallJava *callJava);
    ~MyAudio();

    void play();

    int resampleAudio(void **buf);

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(SLuint32 sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float seed);

};


#endif //AVONE_MYAUDIO_H
