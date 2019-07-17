
#include <jni.h>
#include <string>
#include "pthread.h"

pthread_t pthread;

extern "C"
{
#include <libavformat/avformat.h>
}

#include "common/MyFFmpeg.h"
#include "common/PlayStatus.h"

MyFFmpeg *myFFmpeg = NULL;
CallJava *callJava;
PlayStatus *playStatus = NULL;
JavaVM *local_jvm;
bool n_exit = true;
pthread_t start_thread;

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1prepare(JNIEnv *env, jobject instance,
                                                     jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (myFFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(local_jvm, env, &instance);
        }

        if (playStatus == NULL) {
            playStatus = new PlayStatus();
        }
        myFFmpeg = new MyFFmpeg(playStatus, source, callJava);
    }
    myFFmpeg->prepare();

    //env->ReleaseStringUTFChars(source_, source);
}

void *startFFmpeg(void *data) {
    MyFFmpeg *fFmpeg = (MyFFmpeg *) data;
    fFmpeg->start();
    pthread_exit(&start_thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1start(JNIEnv *env, jobject instance) {
    if (myFFmpeg != NULL) {
        pthread_create(&start_thread, NULL, startFFmpeg, myFFmpeg);
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    local_jvm = jvm;
    if (jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1pause(JNIEnv *env, jobject instance) {
    if (myFFmpeg != NULL) {
        myFFmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1resume(JNIEnv *env, jobject instance) {
    if (myFFmpeg != NULL) {
        myFFmpeg->resume();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1stop(JNIEnv *env, jobject instance) {
    if (!n_exit) {
        return;
    }
    //调用下一曲
//    jclass clss = env->GetObjectClass(instance);
//    jmethodID next_method = env->GetMethodID(clss, "onCallNext", "()V");
    n_exit = false;

    if (myFFmpeg != NULL) {
        myFFmpeg->release();
        delete myFFmpeg;
        myFFmpeg = NULL;

        if (callJava != NULL) {
            delete callJava;
            callJava = NULL;
        }

        if (playStatus != NULL) {
            delete playStatus;
            playStatus = NULL;
        }
    }
    n_exit = true;

   // env->CallVoidMethod(instance, next_method);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1seek(JNIEnv *env, jobject instance, jint sec) {

    if (myFFmpeg != NULL) {
        myFFmpeg->seek(sec);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_av_myplayer_player_MyPlayer_get_1Duration(JNIEnv *env, jobject instance) {

    // TODO
    if (myFFmpeg != NULL) {
        return myFFmpeg->myAudio->duration;
    }

    return 0;

}extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_set_1volume(JNIEnv *env, jobject instance, jint percent) {

    // TODO
    if (myFFmpeg != NULL) {
        myFFmpeg->setVolume(percent);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_set_1mute(JNIEnv *env, jobject instance, jint mute) {
    if (myFFmpeg != NULL) {
        myFFmpeg->setMute(mute);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_set_1speed(JNIEnv *env, jobject instance, jfloat speed) {

    // TODO
    if (myFFmpeg != NULL) {
        myFFmpeg->setSpeed(speed);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_set_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {

    // TODO
    if (myFFmpeg != NULL) {
        myFFmpeg->setPitch(pitch);
    }

}