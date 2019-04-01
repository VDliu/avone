#include "jni.h"
#ifndef AVONE_JAVALISTENER_H
#define AVONE_JAVALISTENER_H

class JavaListener {
public:
    static const int MAIN_THREAD ;
    static const int CHILD_THREAD;

private:
    JavaVM *jvm = nullptr;
    _JNIEnv *jenv = nullptr;
    jobject jobj;
    jmethodID jmid;

public:
    JavaListener(JavaVM *vm, _JNIEnv *env, jobject obj);
    ~JavaListener();

    /**
     * 1:主线程
     * 0：子线程
     * @param type
     * @param code   java 函数参数
     * @param msg    java 函数参数
     */
    void onError(int type, int code, const char *msg);
};


#endif //AVONE_JAVALISTENER_H
