#include "jni.h"
#ifndef AVONE_JAVALISTENER_H
#define AVONE_JAVALISTENER_H

class JavaListener {
public:
    static const int MAIN_THREAD ;
    static const int CHILD_THREAD;

protected:
    JavaVM *jvm = nullptr;
    _JNIEnv *jenv = nullptr;
    jobject jobj ;
    jmethodID jmid;
    bool isOk = false;

public:
    JavaListener(JavaVM *vm, _JNIEnv *env, jobject obj);
    ~JavaListener();

public:
    //自类自行实现获取jmethod方法
     virtual void getMethodId(const char* methodName,const char* methodSignature);
};


#endif //AVONE_JAVALISTENER_H
