//
// Created by hulk on 2019/4/2.
//

#ifndef AVONE_PREPARECALLBACK_H
#define AVONE_PREPARECALLBACK_H


#include "../base/JavaListener.h"
#include "../../common/MyAudio.h"

class PrepareCallBack : public JavaListener{
public:
    MyAudio *myAudio;

public:
    PrepareCallBack(JavaVM *vm, _JNIEnv *env,  jobject obj);
    void onprepared(int thread);
};


#endif //AVONE_PREPARECALLBACK_H
