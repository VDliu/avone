package pictrue.com.reiniot.ffmpeg;

/**
 * 2019/7/10.
 */
public class FFmpeg {
    static {
        System.loadLibrary("ffmpeg-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
    }
    public native boolean snapVideo(double startTime,String source,String result);
}
