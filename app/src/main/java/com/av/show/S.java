package com.av.show;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.view.View;


/**
 * 2019/7/10.
 */
public class S extends Activity {
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
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        verifyStoragePermissions(this);
        findViewById(R.id.begin_btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
               // new FFmpeg().snapVideo(30,"/storage/emulated/0/qiu.mp4","/storage/emulated/0/test_restult.mp4");
            }
        });
    }
}
