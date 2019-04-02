package com.av.myplayer;

import android.util.Log;

public class Demo {
    private static final String TAG = "Demo";

    static {
//        System.loadLibrary("native-lib");
//        System.loadLibrary("avcodec");
//        System.loadLibrary("avfilter");
//        System.loadLibrary("avformat");
//        System.loadLibrary("avutil");
//        System.loadLibrary("swresample");
//        System.loadLibrary("postproc");
//        System.loadLibrary("swscale");
    }

    public void onError(int code,String message){
        Log.e(TAG, "onError: " + "code = " + code + ",message = " +message );
    }

    public native String stringFromJNI();
    public native void createNativeThread();
    public native void createMutextNativeThread();
    public native void nativeInvokeJava();
    public native void nativeInvokeJavaInChildThread();

}
