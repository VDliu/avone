package com.av.myplayer.player;

import android.app.IntentService;
import android.text.TextUtils;
import android.util.Log;

import com.av.myplayer.bean.TimeInfoBean;
import com.av.myplayer.gl.view.MySurfaceView;
import com.av.myplayer.listener.OnComplete;
import com.av.myplayer.listener.OnErrorListener;
import com.av.myplayer.listener.OnLoadListener;
import com.av.myplayer.listener.OnPauseResumeListener;
import com.av.myplayer.listener.OnTimeInfoListener;
import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.myLog.MyLog;


public class MyPlayer {
    /**
     * 1.setsource
     * 2.prepare
     */
    private static final String TAG = "MyPlayer";

    private static String source;
    private static boolean play_next = false;
    private int total_time;

    private PrepareListener prepareListener;
    private OnLoadListener loadListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnTimeInfoListener timeInfoListener;
    private OnErrorListener errorListener;
    private OnComplete complete;
    private MySurfaceView surfaceView;

    public void setSurfaceView(MySurfaceView surfaceView) {
        this.surfaceView = surfaceView;
    }

    public void setYuvData(int w, int h, byte[] buffer_y, byte[] buffer_u, byte[] buffer_v) {
        if (surfaceView != null) {
            surfaceView.setYuvData(w, h, buffer_y, buffer_u, buffer_v);
        }
    }

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        //  System.loadLibrary("postproc");
        System.loadLibrary("swscale-4");
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

    public void setCompelet(OnComplete complete) {
        this.complete = complete;
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

    public void onCallComplete() {
        stop();

        if (complete != null) {
            complete.complete();
        }

    }

    public void setPauseResumeListener(OnPauseResumeListener pauseResumeListener) {
        this.pauseResumeListener = pauseResumeListener;
    }

    //出错以后 停止
    public void onCallError(int code, String message) {

        if (errorListener != null) {
            errorListener.onError(code, message);
        }
        Log.e(TAG, "onCallError: error" );
        player_stop();
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
        timeBean = null;
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
        Log.e(TAG, "onCallNext: ");
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

    public int getDuration() {
        if (total_time > 0) {
            return total_time;
        }
        return get_Duration();
    }

    public void setVolume(int percent) {
        if (percent >= 0 && percent <= 100) {
            set_volume(percent);
        }
    }

    public void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        Log.e(TAG, "onCallRenderYUV: ------------" );

        setYuvData(width,height,y,u,v);
    }


    public void setMute(int mute) {
        set_mute(mute);
    }

    public void setSpeed(float speed) {
        set_speed(speed);
    }

    public void setPitch(float pitch) {
        set_pitch(pitch);
    }


    private native void player_prepare(String source);

    private native void player_start();

    private native void player_pause();

    private native void player_resume();

    private native void player_stop();

    private native void player_seek(int sec);

    private native int get_Duration();

    private native void set_volume(int percent);

    private native void set_mute(int mute);

    private native void set_speed(float speed);

    private native void set_pitch(float mute);
}
