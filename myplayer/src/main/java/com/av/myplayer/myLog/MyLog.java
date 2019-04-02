package com.av.myplayer.myLog;

import android.util.Log;

public class MyLog {
    private static boolean DEBUG = true;


   public static void d(String TAG,String msg){
       if (DEBUG){
           Log.d(TAG, "d: " + msg);
       }
   }
}
