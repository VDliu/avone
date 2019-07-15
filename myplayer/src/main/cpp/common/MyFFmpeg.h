#ifndef AVONE_FFMPEG_H
#define AVONE_FFMPEG_H


#include "../androidplatform/callback/PrepareCallBack.h"
#include "pthread.h"
#include "MyAudio.h"
#include "PlayStatus.h"
#include "../androidplatform/callback/CallJava.h"
#include "MyVideo.h"

extern "C"
{
    #include <libavformat/avformat.h>
}

class MyFFmpeg {
public:

    CallJava *callJava;
    const char *url = NULL;
    pthread_t decode_thread;
    AVFormatContext * pFormatCtx = NULL;

    //音频
    MyAudio *myAudio = NULL;
    PlayStatus *playStatus = NULL;

    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;
    bool exit ;

    //视频
    MyVideo *myVideo = NULL;


public:
    MyFFmpeg(PlayStatus *playStatus, const char *url,CallJava *callJava);

    ~MyFFmpeg();

    void prepare();

    void decodeFFmepg();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int sec);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext);
};


#endif //AVONE_FFMPEG_H
