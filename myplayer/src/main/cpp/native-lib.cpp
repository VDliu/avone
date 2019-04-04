#include <jni.h>
#include <string>
#include "pthread.h"
#include "androidplatform/MyLog.h"
#include "androidplatform/callback/TestErrorCallBack.h"

pthread_t pthread;

extern "C"
{
    #include <libavformat/avformat.h>
}

#include "common/MyFFmpeg.h"
#include "common/PlayStatus.h"
MyFFmpeg *myFFmpeg = NULL;
PrepareCallBack *prepareCallBack;
PlayStatus *playStatus = NULL;
JavaVM *local_jvm;

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1prepare(JNIEnv *env, jobject instance,
                                                     jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (myFFmpeg == NULL){
        if (prepareCallBack == NULL){
            prepareCallBack = new PrepareCallBack(local_jvm,env,env->NewGlobalRef(instance));
        }

        if (playStatus == NULL){
            playStatus = new PlayStatus();
        }
        myFFmpeg = new MyFFmpeg(playStatus,prepareCallBack,source);
    }
    myFFmpeg->prepare();

   //env->ReleaseStringUTFChars(source_, source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1start(JNIEnv *env, jobject instance) {
    if(myFFmpeg != NULL) {
        myFFmpeg->start();
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void* reserved){
    JNIEnv *env;
    local_jvm = jvm;
    if(jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK){
        return -1;
    }
    return JNI_VERSION_1_6;
}



