#ifndef AVONE_FFMPEG_H
#define AVONE_FFMPEG_H


#include "../androidplatform/callback/PrepareCallBack.h"
#include "pthread.h"
#include "MyAudio.h"
#include "PlayStatus.h"
#include "../androidplatform/callback/CallJava.h"

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
};


#endif //AVONE_FFMPEG_H
