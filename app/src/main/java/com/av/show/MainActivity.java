package com.av.show;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.av.myplayer.bean.TimeInfoBean;
import com.av.myplayer.gl.view.MySurfaceView;
import com.av.myplayer.listener.OnComplete;
import com.av.myplayer.listener.OnErrorListener;
import com.av.myplayer.listener.OnLoadListener;
import com.av.myplayer.listener.OnPauseResumeListener;
import com.av.myplayer.listener.OnTimeInfoListener;
import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.player.MyPlayer;
import com.av.myplayer.utils.TimeUtils;


public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";

    Button begin_btn;
    Button pause_btn;
    Button resume_btn;
    Button stop_btn;
    Button seek_btn;
    Button left_mute;
    Button right_mute;
    Button center_mute;

    Button speed_btn;
    Button pitch_btn;
    Button normal_btn;
    Button speed_pitch_btn;
    SeekBar seekBar;
    boolean is_progress_seek = false;
    private int currentVolume = 50;
    SeekBar volume_seek;
    TextView tv_volume;
    TextView tv_mute;
    TextView tv_voice;

    TextView tv_time;
    private MyPlayer player;
    private MySurfaceView surfaceView;
    private final static String SDCARD = "/storage/emulated/0";
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE"};
    private int mute = 2;
    private int seek_time;


    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            int permission1 = ActivityCompat.checkSelfPermission(activity,
                    "READ_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED && permission1 != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, 1000);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        verifyStoragePermissions(this);
        Log.e(TAG, "onCreate: ==" + Environment.getExternalStorageDirectory());

        player = new MyPlayer();
        surfaceView = findViewById(R.id.surface);
        player.setSurfaceView(surfaceView);

        seekBar = findViewById(R.id.seek_bar);
        begin_btn = findViewById(R.id.begin_btn);
        pause_btn = findViewById(R.id.pause_btn);
        resume_btn = findViewById(R.id.play_resume);
        tv_time = findViewById(R.id.tv_time);
        stop_btn = findViewById(R.id.play_stop);
        volume_seek = findViewById(R.id.seek_bar_volume);
        seek_btn = findViewById(R.id.seek_btn);
        tv_volume = findViewById(R.id.tv_volume);
        left_mute = findViewById(R.id.left_mute);
        right_mute = findViewById(R.id.right_mute);
        center_mute = findViewById(R.id.center_mute);
        tv_mute = findViewById(R.id.current_mute);
        left_mute.setOnClickListener(this);
        right_mute.setOnClickListener(this);
        center_mute.setOnClickListener(this);


        speed_btn = findViewById(R.id.speed_btn);
        pitch_btn = findViewById(R.id.pitch_btn);
        normal_btn = findViewById(R.id.normal_btn);
        speed_pitch_btn = findViewById(R.id.speed_pitch);
        speed_btn.setOnClickListener(this);
        pitch_btn.setOnClickListener(this);
        normal_btn.setOnClickListener(this);
        speed_pitch_btn.setOnClickListener(this);

        tv_voice = findViewById(R.id.voice_tv);


        tv_volume.setText("当前音量" + currentVolume + "%");
        tv_mute.setText("当前声道" + "立体声");
        volume_seek.setProgress(currentVolume);

        volume_seek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                player.setVolume(progress);
                tv_volume.setText("当前音量" + progress + "%");
                currentVolume = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                if (is_progress_seek) {
                    Log.e(TAG, "onProgressChanged: durateion = " + player.getDuration() + ",i=" + i);
                    seek_time = player.getDuration() * i / 100;
                    Log.e(TAG, "onProgressChanged: seek time =" + seek_time);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                is_progress_seek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                player.seek(seek_time);
                is_progress_seek = false;
            }
        });
        seek_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                player.seek(5);
            }
        });

        pause_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                player.pause();

            }
        });

        resume_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                player.resume();
            }
        });


        begin_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                player.setSource("/mnt/shared/Other/miao.mp4");
                String str = Environment.getExternalStorageDirectory().toString() +"/test.mp4" ;
                Log.e(TAG, "onClick: str =" + str);
                //player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                //player.setSource("/storage/emulated/0/a.m4a");
                //player.setSource("/storage/emulated/0/test.mp3");
                // player.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
                player.prepare();
            }
        });

        player.setPrepareListener(new PrepareListener() {
            @Override
            public void onPrepared() {
                Log.e(TAG, "native media is prepared");
                player.setVolume(currentVolume);
                player.setMute(mute);
                //开始播放
                player.start();
            }
        });

        stop_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                player.stop();
            }
        });

        player.setLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean loading) {
                Log.e(TAG, "onLoad: loading = " + loading);

            }
        });

        player.setPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause) {
                    Log.e(TAG, "onPause: 暂停中");
                } else {
                    Log.e(TAG, "onPause: 继续播放");
                }
            }
        });

        player.setTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTimeInfo(final TimeInfoBean timeInfoBean) {
                // Log.e(TAG, "onTimeInfo: " +timeInfoBean );
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tv_time.setText(TimeUtils.secdsToDateFormat(timeInfoBean.getTotalTime(), timeInfoBean.getTotalTime())
                                + "/" + TimeUtils.secdsToDateFormat(timeInfoBean.getCurrentTime(), timeInfoBean.getTotalTime()));
                        if (is_progress_seek) return;
                        double time = (double) timeInfoBean.getCurrentTime() / (double) player.getDuration() * 100;
                        //seekBar.setProgress((int) time);
                    }
                });

            }
        });

        player.setErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.e(TAG, "onError: code =" + code + ",error message =" + msg);

            }
        });

        player.setCompelet(new OnComplete() {
            @Override
            public void complete() {
                Log.e(TAG, "compelet: --");
            }
        });
    }

    public void next(View v) {
        Log.e(TAG, "next: ");
        player.playNext("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (player != null) {
            player.stop();
        }
    }

    @Override
    public void onClick(View view) {
        String str = "";
        switch (view.getId()) {
            case R.id.left_mute:
                mute = 1;
                str = "左声道";
                tv_mute.setText("当前声道：" + str);
                player.setMute(mute);
                break;
            case R.id.right_mute:
                mute = 0;
                str = "右声道";
                tv_mute.setText("当前声道：" + str);
                player.setMute(mute);
                break;
            case R.id.center_mute:
                mute = 2;
                str = "立体声";
                tv_mute.setText("当前声道：" + str);
                player.setMute(mute);
                break;
            case R.id.speed_btn:
                player.setSpeed(2.0f);
                player.setPitch(1.0f);
                tv_voice.setText("变速");
                break;
            case R.id.pitch_btn:
                player.setPitch(2.0f);
                player.setSpeed(1.0f);
                tv_voice.setText("变调");
                break;
            case R.id.speed_pitch:
                player.setSpeed(2.0f);
                player.setPitch(2.0f);
                tv_voice.setText("变调-变速");
                break;
            case R.id.normal_btn:
                player.setSpeed(1.0f);
                player.setPitch(1.0f);
                tv_voice.setText("正常音调");
                break;

            default:
                break;
        }

    }
}
