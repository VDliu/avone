
/**
  *2019/4/2.
  *
 */

#include "MyAudio.h"

MyAudio::MyAudio(int index,AVCodecParameters * codecPar) {
    this->streamIndex = index;
    this->codecParameters = codecPar;
}
