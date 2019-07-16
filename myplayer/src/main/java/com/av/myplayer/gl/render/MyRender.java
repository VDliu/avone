package com.av.myplayer.gl.render;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import com.av.myplayer.R;

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
     * 音视频同步
     *
     * 音频播放时长 固定：由采样率 通道数 位深度 决定/ all data /
     *
     * 视频播放时长 ->与两帧之间的渲染间隔相关
     *
     * 由于人对声音敏感 所以固定声音播放 视频播放去同步音频播放
     *
     *
     * 视频快了就休眠久一点 视频慢了就休眠短一点
     */


    /**
     * 顶点坐标范围-1 ~1,归一化坐标，独立于屏幕坐标，此时注意正交投影问题
     * 1
     * <p>
     * -1 --------1
     * <p>
     * -1
     * <p>
     * (-1,1)           (1,1)
     * (-1,-1)          (1,-1)
     */
    private final float[] vertexData = {
            -1f, -1f, // bottom left
            1f, -1f, // bottom right
            -1f, 1f, // top left
            1f, 1f,  // top right

    };

    /**
     * 纹理坐标范围
     * <p>
     * <p>
     * (0,0)              (1,0)
     * (0,1)              (1,1)
     */
    //纹理坐标  对应顶点坐标  与之映射
    static float textureData[] = {   // in counterclockwise order:
            0f, 1f, // bottom left
            1f, 1f, // bottom right
            0f, 0f, // top left
            1f, 0f,  // top right
    };


    private FloatBuffer vertexBuffer;
    private int programYUV;
    private int avPosition;
    private int afPosition;//纹理坐标

    //yuv 纹理id
    private int s_y;
    private int s_u;
    private int s_v;

    private int[] textureYuv = new int[3];
    private int yuvW;
    private int yuvH;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    private void initRenderYuv() {
        String vertexSource = ShaderUtils.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = ShaderUtils.readRawTxt(context, R.raw.fragment_shader);
        programYUV = ShaderUtils.createProgram(vertexSource, fragmentSource);

        if (programYUV > 0) {
            avPosition = GLES20.glGetAttribLocation(programYUV, "av_Position");
            //获取纹理坐标location
            afPosition = GLES20.glGetAttribLocation(programYUV, "af_Position");

            s_y = GLES20.glGetUniformLocation(programYUV, "sampler_y");
            s_u = GLES20.glGetUniformLocation(programYUV, "sampler_u");
            s_v = GLES20.glGetUniformLocation(programYUV, "sampler_v");

            GLES20.glGenTextures(3, textureYuv, 0);

            for (int i = 0; i < textureYuv.length; i++) {

                //绑定纹理
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureYuv[i]);
                //环绕（超出纹理坐标范围）  （s==x t==y GL_REPEAT 重复）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
                //过滤（纹理像素映射到坐标点）  （缩小、放大：GL_LINEAR线性）
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            }
        }

    }

    public MyRender(Context context) {
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
        Log.e(TAG, "onSurfaceCreated: ");
        initRenderYuv();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int w, int h) {
        Log.e(TAG, "onSurfaceChanged: w = " + w + ",h =" + h);
        GLES20.glViewport(0, 0, w, h);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        renderYuv();
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    public void setYuvData(int w, int h, byte[] buffer_y, byte[] buffer_u, byte[] buffer_v) {
        this.yuvW = w;
        this.yuvH = h;
        this.y = ByteBuffer.wrap(buffer_y);
        this.u = ByteBuffer.wrap(buffer_u);
        this.v = ByteBuffer.wrap(buffer_v);
    }

    private void renderYuv() {
        if (yuvH > 0 && yuvW > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(programYUV);
            GLES20.glEnableVertexAttribArray(avPosition);
            //2 每個顶点由多少个坐标组成  false是否归一化  tride 每个顶点占多少字节
            //设置顶点位置
            GLES20.glVertexAttribPointer(avPosition, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);


            //使能纹理
            GLES20.glEnableVertexAttribArray(afPosition);
            //设置纹理位置值
            GLES20.glVertexAttribPointer(afPosition, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);


            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureYuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, yuvW, yuvH, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureYuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, yuvW / 2, yuvH / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureYuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, yuvW / 2, yuvH / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

            //传递纹理到fragment
            //通过赋值，可以指定sampler与纹理单元的关系，
            //想让sampler对哪个纹理单元GL_TEXTUREi中的纹理进行采样/处理，就给它赋值i，如果纹理是GL_TEXTURE0，就给sampler2D赋值为0，以此类推。
            GLES20.glUniform1i(s_y, 0);
            GLES20.glUniform1i(s_u, 1);
            GLES20.glUniform1i(s_v, 2);

            y.clear();
            u.clear();
            v.clear();
        }
    }
}
