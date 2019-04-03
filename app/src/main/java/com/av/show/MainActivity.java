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

import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.player.MyPlayer;

import javax.security.auth.login.LoginException;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    Button begin_btn;
    Button start_btn;
    Button pcm_btn;
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
        pcm_btn = findViewById(R.id.play_pcm);

        start_btn = findViewById(R.id.start_btn);
        start_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                player.start();
            }
        });

        pcm_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
               // player.playPcm("/sdcard/mydream.pcm");
                player.playPcm("/sdcard/mymusic.pcm");
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
            }
        });

        //Demo demo = new Demo();
        //demo.createNativeThread();
        //demo.createMutextNativeThread();
        // demo.nativeInvokeJava();
        // demo.nativeInvokeJavaInChildThread();
    }
}
