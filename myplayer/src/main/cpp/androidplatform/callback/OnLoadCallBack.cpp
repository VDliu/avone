
/**
  *2019/4/8.
  *
 */

#include "OnLoadCallBack.h"
#include "../MyLog.h"

OnLoadCallBack::OnLoadCallBack(JavaVM *vm, _JNIEnv *env, jobject obj) : JavaListener(vm, env, obj) {
    getMethodId("onCallLoad","(Z)V");
}

void OnLoadCallBack::onLoading(int thread, bool isLoading) {
    if (thread == CHILD_THREAD){
        JNIEnv * tenv;
        //找到子线程中的env
        if (this->jvm->AttachCurrentThread(&tenv,0) != JNI_OK){
            LOGE("attach child thread failed");
            return;
        }
        tenv->CallVoidMethod(this->jobj,this->jmid,isLoading);
        this->jvm->DetachCurrentThread();
    } else if (thread == MAIN_THREAD){
        this->jenv->CallVoidMethod(this->jobj,this->jmid,isLoading);
    }

}
