#include "TestErrorCallBack.h"
#include "../MyLog.h"

TestErrorCallBack::TestErrorCallBack(JavaVM *vm, _JNIEnv *env,  jobject obj) : JavaListener(
        vm, env, obj) {
    getMethodId("onError","(ILjava/lang/String;)V");
}

void TestErrorCallBack::onError(int type, int code, const char *msg) {
    if (!isOk){
        LOGE("callback not prepare ok");
        return;
    }

    if (type == CHILD_THREAD){
        JNIEnv * tenv;
        //找到子线程中的env
        if (this->jvm->AttachCurrentThread(&tenv,0) != JNI_OK){
            LOGE("attch child thread failed");
            return;
        }
        jstring str = tenv->NewStringUTF(msg);
        tenv->CallVoidMethod(this->jobj,this->jmid,code,str);
        //由于str给了 java层使用，不用去释放内存
        //tenv->DeleteGlobalRef(str);
        this->jvm->DetachCurrentThread();
    } else if (type == MAIN_THREAD){
        jstring str = this->jenv->NewStringUTF(msg);
        this->jenv->CallVoidMethod(this->jobj,this->jmid,code,str);
        //由于str给了 java层使用，不用去释放内存
       // this->jenv->DeleteGlobalRef(str);
        return;
    }
}

TestErrorCallBack::~TestErrorCallBack() {

}
