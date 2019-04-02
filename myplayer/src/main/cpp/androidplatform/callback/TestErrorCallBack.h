//
// Created by hulk on 2019/4/2.
//

#ifndef AVONE_TESTERRORCALLBACK_H
#define AVONE_TESTERRORCALLBACK_H


#include "../base/JavaListener.h"


class TestErrorCallBack : public JavaListener{

public:
    TestErrorCallBack(JavaVM *vm, _JNIEnv *env,  jobject obj);
    ~TestErrorCallBack();

public:
    /**
 * 1:主线程
 * 0：子线程
 * @param type
 * @param code   java 函数参数
 * @param msg    java 函数参数
 */
    void onError(int type, int code, const char *msg);
};


#endif //AVONE_TESTERRORCALLBACK_H
