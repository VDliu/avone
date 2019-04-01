//
// Created by hulk on 2019/3/28.
//

#include "JavaListener.h"
const int JavaListener::MAIN_THREAD = 1;//静态成员在此初始化
const int JavaListener::CHILD_THREAD = 0;//静态成员在此初始化

void JavaListener::onError(int type, int code, const char *msg) {
    if (type == CHILD_THREAD){
        JNIEnv * tenv;
        //找到子线程中的env
        this->jvm->AttachCurrentThread(&tenv,0);
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

JavaListener::JavaListener(JavaVM *vm, _JNIEnv *env, jobject obj) {
    this->jvm = vm;
    this->jenv = env;
    this->jobj = obj;

    jclass js = env->GetObjectClass(obj);
    if (!js){
        return ;
    }

    this->jmid = env->GetMethodID(js,"onError","(ILjava/lang/String;)V");
    env->DeleteLocalRef(js);
    if (!this->jmid){
        return ;
    }

}

JavaListener::~JavaListener() {

}
