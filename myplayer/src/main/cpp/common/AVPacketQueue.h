
/**
  *2019/4/2.
  *
 */

#ifndef AVONE_MYQUEUE_H
#define AVONE_MYQUEUE_H

#include "queue"
#include "pthread.h"
#include "PlayStatus.h"

extern "C"{
    #include "libavcodec/avcodec.h"
};


class AVPacketQueue {

public:
    std::queue<AVPacket *> avPackQueue;
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
    PlayStatus *playStatus = NULL;

public:
    AVPacketQueue(PlayStatus *playStatus);
    ~AVPacketQueue();

    int putAvPacket(AVPacket *avPacket);
    int getAvPacket(AVPacket *avPacket);
    int getQueueSize();
    void clearQueue();
};


#endif //AVONE_MYQUEUE_H
