#ifndef AVONE_FFMPEG_H
#define AVONE_FFMPEG_H


#include "../androidplatform/callback/PrepareCallBack.h"
#include "pthread.h"
#include "MyAudio.h"
#include "PlayStatus.h"

extern "C"
{
    #include <libavformat/avformat.h>
}

class MyFFmpeg {
public:
    PrepareCallBack *callBack = NULL;
    const char *url = NULL;
    pthread_t decode_thread;
    AVFormatContext * pFormatCtx = NULL;

    //音频
    MyAudio *myAudio = NULL;
    PlayStatus *playStatus = NULL;

public:
    MyFFmpeg(PlayStatus *playStatus,PrepareCallBack *callBack, const char *url);

    ~MyFFmpeg();

    void prepare();

    void decodeFFmepg();

    void start();
};


#endif //AVONE_FFMPEG_H
