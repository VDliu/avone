
/**
  *2019/4/2.
  *
 */

#include "AVPacketQueue.h"
#include "../androidplatform/MyLog.h"

AVPacketQueue::AVPacketQueue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

AVPacketQueue::~AVPacketQueue() {

    //释放队列中保存的数据
    pthread_mutex_lock(&mutex);
    while (avPackQueue.size() > 0){
        AVPacket *packet = avPackQueue.front();
        avPackQueue.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutex);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

int AVPacketQueue::putAvPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);
    if (avPacket != NULL) {
        avPackQueue.push(avPacket);
       // LOGD("insert a packet in queue ,total = %d",avPackQueue.size());
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int AVPacketQueue::getAvPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);

    while (playStatus != NULL && !playStatus->exit) {
        if (avPackQueue.size() > 0) {
            AVPacket *packet = avPackQueue.front();

            if (av_packet_ref(avPacket, packet) == 0) {
                avPackQueue.pop();
            }

            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            LOGD("get avpacket ok ,the rest data of queue is = %d", avPackQueue.size());
            break;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

int AVPacketQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutex);
    size = avPackQueue.size();
    pthread_mutex_unlock(&mutex);
    return size;
}


