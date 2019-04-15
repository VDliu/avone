package com.av.myplayer.player;

import android.text.TextUtils;
import android.util.Log;

import com.av.myplayer.bean.TimeInfoBean;
import com.av.myplayer.listener.OnCompelet;
import com.av.myplayer.listener.OnErrorListener;
import com.av.myplayer.listener.OnLoadListener;
import com.av.myplayer.listener.OnPauseResumeListener;
import com.av.myplayer.listener.OnTimeInfoListener;
import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.myLog.MyLog;


public class MyPlayer {
    private static final String TAG = "MyPlayer";

    private static String source;
    private static boolean play_next = false;

    private PrepareListener prepareListener;
    private OnLoadListener loadListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnTimeInfoListener timeInfoListener;
    private OnErrorListener errorListener;
    private OnCompelet compelet;

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

    private TimeInfoBean timeBean;

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

    public void setTimeInfoListener(OnTimeInfoListener timeInfoListener) {
        this.timeInfoListener = timeInfoListener;
    }

    public void setErrorListener(OnErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setCompelet(OnCompelet compelet) {
        this.compelet = compelet;
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

    public void onCallCompelet() {
        stop();

        if (compelet != null) {
            compelet.compelet();
        }
    }

    public void setPauseResumeListener(OnPauseResumeListener pauseResumeListener) {
        this.pauseResumeListener = pauseResumeListener;
    }

    public void onCallError(int code, String message) {
        player_stop();
        if (errorListener != null) {
            errorListener.onError(code, message);
        }
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

    public void pause() {
        player_pause();
        if (pauseResumeListener != null) {
            pauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        player_resume();
        if (pauseResumeListener != null) {
            pauseResumeListener.onPause(false);
        }
    }

    public void stop() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                player_stop();
            }
        }).start();
    }

    public void seek(int sec) {
        player_seek(sec);
    }

    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (timeInfoListener != null) {
            if (timeBean == null) {
                timeBean = new TimeInfoBean();
            }
            timeBean.setCurrentTime(currentTime);
            timeBean.setTotalTime(totalTime);
            timeInfoListener.onTimeInfo(timeBean);
        }
    }


    public void onCallNext() {
        Log.e(TAG, "onCallNext: " );
        if (play_next) {
            play_next = false;
            prepare();
        }
    }

    //播放完毕以后，判断是否有新的url需要播放
    public void playNext(String url) {
        source = url;
        play_next = true;
        player_stop();
    }


    private native void player_prepare(String source);

    private native void player_start();

    private native void player_pause();

    private native void player_resume();

    private native void player_stop();

    private native void player_seek(int sec);
}
