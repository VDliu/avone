#include <jni.h>
#include <string>
#include <android/log.h>
#include "pthread.h"

pthread_t pthread;

extern "C"
{
    #include <libavformat/avformat.h>
}





#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"MY_PLAYER",FORMAT,##__VA_ARGS__);

void* normalCallBack(void *){
    LOGI("normall CallBack...");

    //退出线程
    pthread_exit(&pthread);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_av_myplayer_Demo_stringFromJNI(JNIEnv *env, jobject instance) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(nullptr);
    while (c_temp != nullptr)
    {
        switch (c_temp->type)
        {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_Demo_createNativeThread(JNIEnv *env, jobject instance) {

    //创建线程
    pthread_create(&pthread, nullptr,normalCallBack, nullptr);
}


//mutex thread demo ---------------------------------------------------------------------------------------------------------------------------------------------------------//


#include "queue"
#include "unistd.h"

pthread_t pthread_productor;
pthread_t pthread_customer;
pthread_mutex_t mutex;
pthread_cond_t cond;

std::queue<int>  queue;

void* callBackForProductor(void *){
  while (true){
      pthread_mutex_lock(&mutex);
      queue.push(1);
      LOGI("productor a thing, size of queue = %lu",queue.size());
      //唤醒等待锁的线程，此时线程锁还是该线持有，当等待锁的线程被唤醒后将会去等待线程锁
      //随后本线程释放锁，根据本程序代码逻辑，本线程需要休眠5s,被唤醒的线程竞争到线程锁
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
      sleep(5);
  }
    pthread_exit(&pthread_productor);
}

void* callBackForCustomer(void *){
  while (true){
      pthread_mutex_lock(&mutex);
      if (queue.size() > 0){
          queue.pop();
          LOGI("custom a thing, the rest size of queue = %lu",queue.size());
      }else{
          LOGI("the queue is empty, waiting...");
          //1.等待的时候，会把线程锁让出
          //2.在条件满足从而离开pthread_cond_wait()之前，mutex将被重新加锁
          pthread_cond_wait(&cond,&mutex);
      }
      u_int32_t time = 500 * 1000;
      usleep(time);
      pthread_mutex_unlock(&mutex);
  }

    pthread_exit(&pthread_customer);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_Demo_createMutextNativeThread(JNIEnv *env, jobject instance) {
    for(int i = 0 ;i < 10 ;i ++){
        queue.push(i);
    }
    //初始化线程锁和条件锁 以及生产者消费者线程
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_create(&pthread_productor, nullptr,callBackForProductor, nullptr);
    pthread_create(&pthread_customer, nullptr,callBackForCustomer, nullptr);
}

#include "androidplatform/JavaListener.h"
JavaVM *local_jvm;
JavaListener *javaListener;
pthread_t child_thread;

void* childThreadCallBack(void *call){
    JavaListener * javaListener1 = (JavaListener *)call;
    javaListener1->onError(0,200,"call java in child thread");
    pthread_exit(&child_thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_Demo_nativeInvokeJava(JNIEnv *env, jobject instance) {

      //env->NewGlobalRef(instance)  将obj转化为全局的
    javaListener = new JavaListener(local_jvm,env,env->NewGlobalRef(instance));
    javaListener->onError(1,100,"hello main");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void* reserved){
    JNIEnv *env;
    local_jvm = jvm;
    if(jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK){
        return -1;
    }
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_Demo_nativeInvokeJavaInChildThread(JNIEnv *env, jobject instance) {

    //env->NewGlobalRef(instance)  将obj转化为全局的
    javaListener = new JavaListener(local_jvm,env,env->NewGlobalRef(instance));
    pthread_create(&child_thread, nullptr,childThreadCallBack,javaListener);


}