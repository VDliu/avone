#include "PrepareCallBack.h"
#include "../MyLog.h"

PrepareCallBack::PrepareCallBack(JavaVM *vm, _JNIEnv *env, jobject obj) : JavaListener(vm, env, obj) {
    getMethodId("onCallPrepared","()V");
}

void PrepareCallBack::onprepared(int thread) {
    if (thread == CHILD_THREAD){
        JNIEnv * tenv;
        //找到子线程中的env
       if (this->jvm->AttachCurrentThread(&tenv,0) != JNI_OK){
           LOGE("attach child thread failed");
           return;
       }
        tenv->CallVoidMethod(this->jobj,this->jmid);
        this->jvm->DetachCurrentThread();
    } else if (thread == MAIN_THREAD){
        this->jenv->CallVoidMethod(this->jobj,this->jmid);
    }
}
