
/**
  *2019/4/2.
  *
 */

#ifndef AVONE_MYAUDIO_H
#define AVONE_MYAUDIO_H

extern "C"
{
    #include <libavformat/avformat.h>
}

class MyAudio {

public:
    int streamIndex = -1;
    AVCodecParameters * codecParameters = nullptr;
    AVCodecContext *codecContext = nullptr;

public:
    MyAudio(int index,AVCodecParameters * codecPar);
    ~MyAudio();

};


#endif //AVONE_MYAUDIO_H
