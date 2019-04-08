
/**
  *2019/4/8.
  *
 */

#ifndef AVONE_ONLOADCALLBACK_H
#define AVONE_ONLOADCALLBACK_H


#include "../base/JavaListener.h"

class OnLoadCallBack : public JavaListener{
public:
    OnLoadCallBack(JavaVM *vm, _JNIEnv *env,  jobject obj);
    void onLoading(int type, bool isLoading);

};


#endif //AVONE_ONLOADCALLBACK_H
