package com.av.myplayer.player;

import android.text.TextUtils;

import com.av.myplayer.listener.OnLoadListener;
import com.av.myplayer.listener.OnPauseResumeListener;
import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.myLog.MyLog;


public class MyPlayer {
    private static final String TAG = "MyPlayer";

    private String source;
    private PrepareListener prepareListener;
    private OnLoadListener loadListener;
    private OnPauseResumeListener pauseResumeListener;

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

    public MyPlayer() {

    }

    public MyPlayer(String source) {
        this.source = source;

    }

    public void setSource(String source) {
        this.source = source;
    }

    public void setPrepareListener(PrepareListener prepareListener) {
        this.prepareListener = prepareListener;
    }

    public void setLoadListener(OnLoadListener loadListener) {
        this.loadListener = loadListener;
    }

    public void onCallPrepared() {
        if (prepareListener != null) {
            prepareListener.onPrepared();
        }
    }

    public void onCallLoad(boolean loading) {
        if (loadListener != null) {
            loadListener.onLoad(loading);
        }
    }

    public void setPauseResumeListener(OnPauseResumeListener pauseResumeListener) {
        this.pauseResumeListener = pauseResumeListener;
    }

    public void prepare() {
        if (TextUtils.isEmpty(source)) {
            MyLog.d(TAG, "source must not be empty");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                player_prepare(source);
            }
        }).start();

    }

    public void start() {
        if (TextUtils.isEmpty(source)) {
            MyLog.d(TAG, "source must not be empty");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                player_start();
            }
        }).start();

    }

    public void pause()
    {
        player_pause();
        if(pauseResumeListener != null)
        {
            pauseResumeListener.onPause(true);
        }
    }

    public void resume()
    {
        player_resume();
        if(pauseResumeListener != null)
        {
            pauseResumeListener.onPause(false);
        }
    }

    private native void player_prepare(String source);

    private native void player_start();
    private native void player_pause();
    private native void player_resume();
}
