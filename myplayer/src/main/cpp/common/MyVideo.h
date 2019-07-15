//
// Created by yangw on 2018-5-14.
//

#ifndef MYMUSIC_WLVIDEO_H
#define MYMUSIC_WLVIDEO_H


#include <callback/CallJava.h>
#include "AVPacketQueue.h"
#include "PlayStatus.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
};

class MyVideo {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    AVPacketQueue *queue = NULL;
    PlayStatus *playstatus = NULL;
    CallJava *wlCallJava = NULL;
    AVRational time_base;


    pthread_t thread_play;



public:
    MyVideo(PlayStatus *playstatus, CallJava *wlCallJava);

    ~MyVideo();

    void play();

    void release();





};


#endif //MYMUSIC_WLVIDEO_H
