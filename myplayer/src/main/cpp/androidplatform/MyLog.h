//
// Created by hulk on 2019/4/2.
//

#ifndef AVONE_MYLOG_H
#define AVONE_MYLOG_H

#include <android/log.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"MY_PLAYER",FORMAT,##__VA_ARGS__);

#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"MY_PLAYER",FORMAT,##__VA_ARGS__);

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"MY_PLAYER",FORMAT,##__VA_ARGS__);



#endif //AVONE_MYLOG_H
