//
// Created by yangw on 2018-5-14.
//

#include "MyVideo.h"
#include "../androidplatform/MyLog.h"


MyVideo::MyVideo(PlayStatus *playstatus, CallJava *wlCallJava) {

    this->playstatus = playstatus;
    this->wlCallJava = wlCallJava;
    queue = new AVPacketQueue(playstatus, 0);
    pthread_mutex_init(&codec_mutex, NULL);

}

void *playVideo(void *data) {

    MyVideo *video = (MyVideo *) (data);

    while (video != NULL && video->playstatus != NULL && !video->playstatus->exit) {

        if (video->playstatus->isSeeking) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->queue->getQueueSize() == 0) {
            if (!video->playstatus->isLoading) {
                video->playstatus->isLoading = true;
                video->wlCallJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            LOGE("loading video data");
            continue;
        } else {
            if (video->playstatus->isLoading) {
                LOGE("loading video data ok");
                video->playstatus->isLoading = false;
                video->wlCallJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        //暂停状态
        if (video->playstatus->isPause) {
            av_usleep(1000 * 100);
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        //解码--------------------------------------------------------------

        if (video->decode_type == CODE_YUV) {
            pthread_mutex_lock(&video->codec_mutex);
            if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codec_mutex);
                continue;
            }
            AVFrame *avFrame = av_frame_alloc();
            if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0) {
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codec_mutex);
                continue;
            }
            LOGE("子线程软解码一个AVframe视频成功");
            if (avFrame->format == AV_PIX_FMT_YUV420P) {
                //获取到音频和视频之间的pts差值时间
                double diff = video->getFrameDiffTime(avFrame);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                LOGE("当前视频是YUV420P格式");
                if (video != NULL) {
                    video->wlCallJava->onCallRenderYUV(
                            video->avCodecContext->width,
                            video->avCodecContext->height,
                            avFrame->data[0],
                            avFrame->data[1],
                            avFrame->data[2]);
                    LOGE("view draw ok");
                }

            } else {
                LOGE("当前视频不是YUV420P格式");
                //软解码 效率低
                AVFrame *pFrameYUV420P = av_frame_alloc();
                int num = av_image_get_buffer_size(
                        AV_PIX_FMT_YUV420P,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        1);
                uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                av_image_fill_arrays(
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize,
                        buffer,
                        AV_PIX_FMT_YUV420P,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        1);
                SwsContext *sws_ctx = sws_getContext(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        video->avCodecContext->pix_fmt,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        AV_PIX_FMT_YUV420P,
                        SWS_BICUBIC, NULL, NULL, NULL);

                if (!sws_ctx) {
                    av_frame_free(&pFrameYUV420P);
                    av_free(pFrameYUV420P);
                    av_free(buffer);
                    pthread_mutex_unlock(&video->codec_mutex);
                    continue;
                }
                sws_scale(
                        sws_ctx,
                        reinterpret_cast<const uint8_t *const *>(avFrame->data),
                        avFrame->linesize,
                        0,
                        avFrame->height,
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize);

                //音视频同步
                double diff = video->getFrameDiffTime(avFrame);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                //渲染
                if (video != NULL) {
                    video->wlCallJava->onCallRenderYUV(
                            video->avCodecContext->width,
                            video->avCodecContext->height,
                            pFrameYUV420P->data[0],
                            pFrameYUV420P->data[1],
                            pFrameYUV420P->data[2]);
                }
                av_frame_free(&pFrameYUV420P);

                av_free(pFrameYUV420P);

                av_free(buffer);

                sws_freeContext(sws_ctx);
            }

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
        } else {
            LOGE("硬解码");

        }



        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        pthread_mutex_unlock(&video->codec_mutex);
    }

    LOGE("break recycle");
    if (video != NULL) {
        LOGE("break 00000000");
        pthread_exit(&(video->thread_play));
    }

}

void MyVideo::play() {

    pthread_create(&thread_play, NULL, playVideo, this);

}

void MyVideo::release() {
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    pthread_mutex_lock(&codec_mutex);
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    pthread_mutex_unlock(&codec_mutex);

    if (playstatus != NULL) {
        playstatus = NULL;
    }
    if (wlCallJava != NULL) {
        wlCallJava = NULL;
    }

}

MyVideo::~MyVideo() {
    pthread_mutex_destroy(&codec_mutex);
}

double MyVideo::getFrameDiffTime(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);

    if (pts > 0) {
        clock = pts;
    }
    //如果视频pts为0 使用上一次的clock

    double diff = audio->clock - clock;
    return diff;
}

double MyVideo::getDelayTime(double diff) {
    if (diff > 0.003) //音频比视频快 减少视频休眠时间
    {
        LOGE("delay time =%f", delayTime);
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003)//音频比视频慢 加长视频休眠时间
    {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    }

    if (diff >= 0.5) {
        delayTime = 0; //音频比视频快 0.5秒  视频不休眠
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }

    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
