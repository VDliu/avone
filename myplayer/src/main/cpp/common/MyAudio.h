
/**
  *2019/4/2.
  *
 */

#ifndef AVONE_MYAUDIO_H
#define AVONE_MYAUDIO_H

#include "AVPacketQueue.h"
#include "PlayStatus.h"
extern "C"
{
    #include <libavformat/avformat.h>
}

class MyAudio {

public:
    int streamIndex = -1;
    AVCodecParameters * codecParameters = NULL;
    AVCodecContext *codecContext = NULL;
    AVPacketQueue *queue = NULL;
    PlayStatus *playstatus = NULL;

public:
    MyAudio(int index,AVCodecParameters * codecPar,PlayStatus *playStatus);
    ~MyAudio();

};


#endif //AVONE_MYAUDIO_H
