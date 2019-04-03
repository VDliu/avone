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


//mutex thread demo,native调用java方法，c++生产者消费者模型 ---------------------------------------------------------------------------------------------------------------------------------------------------------//


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



JavaVM *local_jvm;
pthread_t child_thread;

void* childThreadCallBack(void *call){
    TestErrorCallBack * callBack = (TestErrorCallBack *)call;
    callBack->onError(JavaListener::CHILD_THREAD,100,"call java in child thread");
    delete callBack;
    pthread_exit(&child_thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_Demo_nativeInvokeJava(JNIEnv *env, jobject instance) {

    //env->NewGlobalRef(instance)  将obj转化为全局的
    TestErrorCallBack *callBack = new TestErrorCallBack(local_jvm,env,env->NewGlobalRef(instance));
    callBack->onError(JavaListener::MAIN_THREAD,200,"call java in main thread");
    delete callBack;
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
    TestErrorCallBack *callBack = new TestErrorCallBack(local_jvm,env,env->NewGlobalRef(instance));
    pthread_create(&child_thread, nullptr,childThreadCallBack,callBack);
}

//------------音频解码------------------------------------------------------------------------------------------------//
#include "common/MyFFmpeg.h"
#include "common/PlayStatus.h"
MyFFmpeg *myFFmpeg = NULL;
PrepareCallBack *prepareCallBack;
PlayStatus *playStatus = NULL;

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

   // env->ReleaseStringUTFChars(source_, source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_player_1start(JNIEnv *env, jobject instance) {
    if(myFFmpeg != NULL){
        myFFmpeg->start();
    }

}

//---------------------------------Opensl es 播放pcm-----------------------------------------------------------------------//
extern "C"
{
    #include <SLES/OpenSLES.h>
    #include <SLES/OpenSLES_Android.h>
}

//引擎接口
SLObjectItf engineObject = NULL;
SLEngineItf engineEngine = NULL;

//混音器
SLObjectItf outputMixObject = NULL;
SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//pcm
SLObjectItf pcmPlayerObject = NULL;
SLPlayItf pcmPlayerPlay = NULL;
SLVolumeItf pcmPlayerVolume = NULL;

//缓冲器队列接口
SLAndroidSimpleBufferQueueItf pcmBufferQueue;

FILE *pcmFile;
void *buffer;

uint8_t *out_buffer;

int getPcmData(void **pcm)
{
    int size = 0;
    while(!feof(pcmFile))
    {
        size = fread(out_buffer, 1, 44100 * 2 * 2, pcmFile);
        if(out_buffer == NULL)
        {
            LOGI("%s", "read end");
            break;
        } else{
            LOGI("%s", "reading");
        }
        *pcm = out_buffer;
        break;
    }
    LOGI("size of pcm = %d",size);
    return size;
}

//进入callback获取数据
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
   int size =  getPcmData(&buffer);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (NULL != buffer) {
        SLresult result;
        // enqueue another buffer
        result = (*pcmBufferQueue)->Enqueue(pcmBufferQueue, buffer, size);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_av_myplayer_player_MyPlayer_playPcm(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    // TODO
    //读取pcm文件
    pcmFile = fopen(url, "r");
    if(pcmFile == NULL)
    {
        LOGE("%s", "fopen file error");
        return;
    }
    //每次读一秒钟的数据
    out_buffer = (uint8_t *) malloc(44100 * 2 * 2);

    SLresult result = -1;
    LOGE("----init result = %d",result);

    //第一步------------------------------------------
    // 创建引擎对象
    // create->realize->GetInterface
    slCreateEngine(&engineObject,0,0,0,0,0);
    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);


    //第二步-------------------------------------------
    // 创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    // 第三步--------------------------------------------
    // 创建播放器

    //buffer队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            SL_SAMPLINGRATE_44_1,//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource slDataSource = {&android_queue, &pcm};
    //播放器添加混音器
    SLDataSink audioSnk = {&outputMix, NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 3, ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    //得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    //第四步---------------------------------------
    // 创建缓冲区和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, NULL);
    //获取音量接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);

    //第五步----------------------------------------
    // 设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    //第六步----------------------------------------
    // 主动调用回调函数开始工作
    pcmBufferCallBack(pcmBufferQueue, NULL);

    env->ReleaseStringUTFChars(url_, url);
}


