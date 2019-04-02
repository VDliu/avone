
/**
  *2019/4/2.
  *
 */

#include "MyAudio.h"

MyAudio::MyAudio(int index,AVCodecParameters * codecPar,PlayStatus *playStatus) {
    this->streamIndex = index;
    this->codecParameters = codecPar;
    this->queue = new AVPacketQueue(playStatus);
    this->playstatus = playStatus;
}

MyAudio::~MyAudio() {
    if (queue != NULL){
        delete queue;
    }

}
