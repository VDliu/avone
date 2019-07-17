
/**
  *2019/4/8.
  *
 */

#ifndef AVONE_CALLJAVA_H
#define AVONE_CALLJAVA_H


#include "jni.h"
#include <linux/stddef.h>

#define MAIN_THREAD 0
#define CHILD_THREAD 1


class CallJava {

public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_parpared;
    jmethodID jmid_load;
    jmethodID jmid_timeinfo;
    jmethodID jmid_error;
    jmethodID jmid_compelet;
    jmethodID jmid_renderyuv;
    jmethodID jmid_supportvideo;

public:
    CallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~CallJava();

    void onCallParpared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int curr, int total);

    void onCallError(int type, int code,char *msg);

    void onCallCompelet(int type);

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

    bool onCallIsSupportVideo(const char *ffcodecname);

};


#endif //AVONE_CALLJAVA_H
