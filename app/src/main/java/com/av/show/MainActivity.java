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
import android.widget.Toast;

import com.av.myplayer.listener.OnLoadListener;
import com.av.myplayer.listener.OnPauseResumeListener;
import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.player.MyPlayer;

import javax.security.auth.login.LoginException;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    Button begin_btn;
    Button pause_btn;
    Button resume_btn;
    private MyPlayer player;
    private final static String SDCARD = "/storage/emulated/0";
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };


    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            int permission1 = ActivityCompat.checkSelfPermission(activity,
                    "READ_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED && permission1 != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,1000);
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
        Log.e(TAG, "onCreate: ==" + Environment.getExternalStorageDirectory() );

        player = new MyPlayer();
        begin_btn = findViewById(R.id.begin_btn);
        pause_btn = findViewById(R.id.pause_btn);
        resume_btn = findViewById(R.id.play_resume);

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
                //player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
               // player.setSource("/storage/emulated/0/a.m4a");
                player.setSource("/storage/emulated/0/test.mp3");
                player.prepare();
            }
        });

        player.setPrepareListener(new PrepareListener() {
            @Override
            public void onPrepared() {
                Log.e(TAG, "native media is prepared" );
                //开始播放
                player.start();
            }
        });

        player.setLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean loading) {
                Log.e(TAG, "onLoad: loading = " +loading );

            }
        });

        player.setPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause){
                    Log.e(TAG, "onPause: 暂停中"  );
                }else{
                    Log.e(TAG, "onPause: 继续播放" );
                }
            }
        });
    }
}
