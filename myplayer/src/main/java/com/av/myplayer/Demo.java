package com.av.myplayer;

/**
 * Created by yangw on 2018-1-31.
 */

public class Demo {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("postproc");
        System.loadLibrary("swscale");
    }

    public native String stringFromJNI();

}
