package com.av.show;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.av.myplayer.Demo;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Demo demo = new Demo();

        TextView tv = (TextView) findViewById(R.id.sample_text);
        //demo.createNativeThread();
      //  demo.createMutextNativeThread();
       // demo.nativeInvokeJava();
        demo.nativeInvokeJavaInChildThread();
        tv.setText(demo.stringFromJNI());
    }
}
