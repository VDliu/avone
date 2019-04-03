package com.av.show;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.av.myplayer.listener.PrepareListener;
import com.av.myplayer.player.MyPlayer;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    Button begin_btn;
    Button start_btn;
    Button pcm_btn;
    private MyPlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

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
                player.playPcm("/sdcard/mydream.pcm");
            }
        });

        begin_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
               // player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                player.setSource("/sdcard/test.mp3");
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
