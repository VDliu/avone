package com.av.myplayer.gl.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.av.myplayer.gl.render.MyRender;


/**
 * 2019/7/15.
 */
public class MySurfaceView extends GLSurfaceView {
    private MyRender render;

    public MySurfaceView(Context context) {
        super(context);
    }

    public MySurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        render = new MyRender(context);
        setRenderer(render);
        //render 调用request方法后 才调用onDraw方法
        setRenderMode(RENDERMODE_WHEN_DIRTY);
    }


    public void setYuvData(int w, int h, byte[] buffer_y, byte[] buffer_u, byte[] buffer_v) {
        if (render != null) {
            render.setYuvData(w, h, buffer_y, buffer_u, buffer_v);
            requestRender();
        }
    }

}
