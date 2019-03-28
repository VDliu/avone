package com.av.show;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.av.myplayer.Demo;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Demo demo = new Demo();

        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(demo.stringFromJNI());
    }
}
