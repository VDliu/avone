package com.av.show.render;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.util.Log;

import com.av.show.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * 2019/7/15.
 */
public class MyRender implements GLSurfaceView.Renderer {
    private static final String TAG = "MyRender";
    private final Context context;
    private final FloatBuffer textureBuffer;
    private boolean isprint = false;

    /**
     * 顶点坐标范围-1 ~1,归一化坐标，独立于屏幕坐标，此时注意正交投影问题
     *     1
     *
     *-1 --------1
     *
     *    -1
     *
     * (-1,1)           (1,1)
     * (-1,-1)          (1,-1)
     *
     */
    private final float[] vertexData ={
            -1f, -1f, // bottom left
            1f, -1f, // bottom right
            -1f, 1f, // top left
            1f, 1f,  // top right

    };

    /**
     * 纹理坐标范围
     *
     *
     * (0,0)              (1,0)
     * (0,1)              (1,1)
     *
     *
     */
    //纹理坐标  对应顶点坐标  与之映射
    static float textureData[] = {   // in counterclockwise order:
            0f, 1f, // bottom left
            1f, 1f, // bottom right
            0f, 0f, // top left
            1f, 0f,  // top right
    };

    private FloatBuffer vertexBuffer;
    private int program;
    private int avPosition;
    private int afPosition;//纹理坐标
    private int sTexture;
    private int textureId;

    public MyRender(Context context)
    {
        this.context = context;
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        vertexBuffer.position(0);


        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        textureBuffer.position(0);
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        Log.e(TAG, "onSurfaceCreated: " );
        String vertexSource = ShaderUtils.readRawTxt(context, R.raw.vertex_shader_image);
        String fragmentSource = ShaderUtils.readRawTxt(context, R.raw.fragment_shader_imge);
        program = ShaderUtils.createProgram(vertexSource, fragmentSource);
        if(program > 0)
        {
            avPosition = GLES20.glGetAttribLocation(program, "av_Position");
            //获取纹理坐标location
            afPosition = GLES20.glGetAttribLocation(program, "af_Position");
            //获取fragment中纹理的location
            sTexture = GLES20.glGetUniformLocation(program,"sTexture");
            //afColor = GLES20.glGetUniformLocation(program, "af_Color");
            int [] textureIds = new int[1];
            GLES20.glGenTextures(1,textureIds,0);
            if (textureIds[0] == 0){
                Log.e(TAG, "onSurfaceCreated: gen textures failed" );
                return;
            }

             textureId = textureIds[0];

            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,textureId);

            //环绕（超出纹理坐标范围）  （s==x t==y GL_REPEAT 重复）
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            //过滤（纹理像素映射到坐标点）  （缩小、放大：GL_LINEAR线性）
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            //加载图片
            Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.so);

            if (bitmap == null) {
                Log.e(TAG, "onSurfaceCreated: bitmap is null" );
                return;
            }
            //设置纹理为2d图片
            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
        }

    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int w, int h) {
        Log.e(TAG, "onSurfaceChanged: w = " + w + ",h =" +h );
        GLES20.glViewport(0,0,w,h);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if(!isprint){
            Log.e(TAG, "onDrawFrame: " );
            isprint = true;
        }

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        GLES20.glUseProgram(program);
        //紅色
       // GLES20.glUniform4f(afColor, 1f, 0f, 0f, 1f);

        GLES20.glEnableVertexAttribArray(avPosition);
        //2 每個顶点由多少个坐标组成  false是否归一化  tride 每个顶点占多少字节
        //设置顶点位置
        GLES20.glVertexAttribPointer(avPosition, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);



        //使能纹理
        GLES20.glEnableVertexAttribArray(afPosition);
        //设置纹理位置值
        GLES20.glVertexAttribPointer(afPosition, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);


        //0 从第一个顶点开始  3总共绘制三个顶点 绘制
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);


        GLES20.glDisableVertexAttribArray(avPosition);
        GLES20.glDisableVertexAttribArray(afPosition);
    }
}
