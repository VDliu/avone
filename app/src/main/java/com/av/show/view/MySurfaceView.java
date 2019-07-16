package com.av.show.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.av.show.render.MyRender;


/**
 * 2019/7/15.
 */
public class MySurfaceView extends GLSurfaceView {
    public MySurfaceView(Context context) {
        super(context);
    }

    public MySurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        setRenderer(new MyRender(context));
    }


}
