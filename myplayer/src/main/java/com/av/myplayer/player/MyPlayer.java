package com.av.myplayer.player;

import android.text.TextUtils;

import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.myLog.MyLog;


public class MyPlayer {
    private static final String TAG = "MyPlayer";

    private String source;
    private PrepareListener prepareListener;

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

    public MyPlayer(){

    }

    public MyPlayer(String source){
        this.source = source;

    }

    public void setSource(String source) {
        this.source = source;
    }

    public void setPrepareListener(PrepareListener prepareListener) {
        this.prepareListener = prepareListener;
    }

    public void onCallPrepared(){
        if (prepareListener != null){
            prepareListener.onPrepared();
        }
    }

    public void prepare(){
        if (TextUtils.isEmpty(source)){
            MyLog.d(TAG,"source must not be empty");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
               player_prepare(source);
            }
        }).start();

    }

    public void start(){
        if (TextUtils.isEmpty(source)){
            MyLog.d(TAG,"source must not be empty");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                player_start();
            }
        }).start();

    }

    private native void player_prepare(String source);

    private native void player_start();

    public native void playPcm(String url);
}
