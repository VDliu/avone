#include "JavaListener.h"
#include "../MyLog.h"

const int JavaListener::MAIN_THREAD = 1;//静态成员在此初始化
const int JavaListener::CHILD_THREAD = 0;//静态成员在此初始化



JavaListener::JavaListener(JavaVM *vm, _JNIEnv *env, jobject obj) {
    this->jvm = vm;
    this->jenv = env;
    this->jobj = obj;
}

JavaListener::~JavaListener() {

}

void JavaListener::getMethodId(const char* methodName, const char* methodSignature) {
    jclass js = this->jenv->GetObjectClass(this->jobj);
    if (!js){
        LOGE("jcalss get error");
        isOk = false;
        return ;
    }

    this->jmid = this->jenv->GetMethodID(js,methodName,methodSignature);
    this->jenv->DeleteLocalRef(js);
    if (!this->jmid){
        isOk = false;
        LOGE("get jmethod id eroor");
        return ;
    }
    isOk = true;
}

