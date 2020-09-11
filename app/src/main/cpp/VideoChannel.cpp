//
// Created by Administrator on 2020/9/1.
//

#include "VideoChannel.h"

int dstWidth = 0;
int dstHeight = 0;

/**
 * 丢AVPacket ，丢非I帧.
 * @param q
 */
void dropPacket(queue<AVPacket *> &q) {
    LOGE("todo dropPacket");
}

void dropFrame(queue<AVFrame *> &q) {
    LOGE("todo dropFrame");
}


VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                           AVRational time_base, AVFormatContext *formatContext) : BaseChannel(id,
                                                                                               javaCallHelper,
                                                                                               codecContext,
                                                                                               time_base) {
    this->javaCallHelper = javaCallHelper;
    this->codecContext = codecContext;
    this->formatContext = formatContext;
    pkt_queue.setReleaseCallback(releaseAvPacket);
    frame_queue.setReleaseCallback(releaseAVFrame);
    pkt_queue.setSyncHandle(dropPacket);
    frame_queue.setSyncHandle(dropFrame);
}

/**
 * 解码线程.
 * @param args
 * @return
 */
void *decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return 0;
}

void *synchronize(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return 0;
}

void VideoChannel::setRenderFrame(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}

void VideoChannel::setFps(int fps) {
    this->fps = fps;
}

void VideoChannel::play() {
    LOGI("VideoChannel <play>");
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    //解码线程packet->frame.
    pthread_create(&pid_video_play, nullptr, decode, this);
    //播放线程 frame->yuv.
    pthread_create(&pid_synchronize, nullptr, synchronize, this);
}

/**
 * 解码出packet队列数据 .
 */
void VideoChannel::decodePacket() {
    LOGI("VideoChannel:: decodePacket");
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) break;
        if (!ret) {
            continue;
        }
        if (!codecContext) {
            LOGE("VideoChannel:: avCodecContext is NULL!");
            break;
        }
        ret = avcodec_send_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN)) {
            LOGW("VideoChannel:: avcodec_send_packet EAGAIN 等待数据包！");
            //需要更多数据
            continue;
        } else if (ret < 0) {
            LOGE("VideoChannel::avcodec_send_packet fail ret < 0 %d", ret);
            //失败
            break;
        }

        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, avFrame);
        while (isPlaying && frame_queue.size() > 100) {
            av_usleep(1000 * 10);
            LOGW("VideoChannel:: frame queue is full！frame_queue size: %d", frame_queue.size());
            continue;
        }
        //压缩数据要 解压 yuv->rgb888
        //放入缓存队列.
        frame_queue.enQueue(avFrame);
    }
    //保险起见
    releaseAvPacket(packet);
}

void VideoChannel::synchronizeFrame() {
    LOGI("VideoChannel:: synchronizeFrame");
    SwsContext *swsContext = sws_getContext(codecContext->width, codecContext->height,
                                            codecContext->pix_fmt,
                                            codecContext->width, codecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BILINEAR, 0, 0, 0);
    //ARGB接收容器
    uint8_t *dst_data[4];
    //每一行的首地址
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize, codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    //绘制界面
    //转化YUV->RGB
    AVFrame *frame = 0;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        LOGI("VideoChannel:: get frame success : %d", frame_queue.size());
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, dst_data,
                  dst_linesize);
        frame->pts;
        //已经获取rgb数据，回调给native-lib使用
        renderFrame(dst_data[0], dst_linesize[0], codecContext->width, codecContext->height);
        LOGE("解码一帧视频  %d", frame_queue.size());

        clock = frame->pts * av_q2d(time_base);
        //解码一帧视频延时时间
        double frame_delay = 1.0 / fps;
        //解码一帧花费的时间，配置差的手机 解码耗时久，所以需要考虑解码时间.
        double extra_delay = frame->repeat_pict / (2 * fps);
        double delay = frame_delay + extra_delay;

        double audioClock = audioChannel->clock;
        double diff = clock - audioClock;
        LOGI("VideoChannel:: synchronizeFrame audio clock %d", audioClock);
        LOGI("VideoChannel:: synchronizeFrame video clock %d", clock);
        LOGI("VideoChannel:: synchronizeFrame fps %d", fps);
        LOGI("VideoChannel:: synchronizeFrame frame_delay %d", frame_delay);
        LOGI("VideoChannel:: synchronizeFrame extra_delay %d", extra_delay);
        //音视频同步一般都是视频同步音频，音频的卡顿影响感官上更大
        LOGE("-----------相差----------  %d ", diff);
        if (clock > audioClock) {//视频超前
            if (diff > 1) {
                LOGE("-----------睡眠long----------  %d", (delay * 2));
                //差的太久了，那只能慢慢赶 不然视频等待，用感受视频卡很久
                av_usleep((delay * 2) * 1000000);
            } else {
                LOGE("-----------睡眠normal----------  %d", (delay + diff));
                av_usleep((delay + diff) * 1000000);
            }
        } else {//音频超前, 丢帧处理
            LOGE("-----------音频超前，相差----------  %d", diff);
            if (abs(diff) > 1) {
                //不做休眠
            } else if (abs(diff) > 0.05) {
                //视频需要追赶.丢帧(非关键帧) 同步
                releaseAVFrame(frame);
                frame_queue.sync();
            } else {
                av_usleep((delay + diff) * 1000000);
            }
            releaseAVFrame(frame);
        }
    }
    //释放资源.
    av_free(&dst_data[0]);
    isPlaying = false;
    releaseAVFrame(frame);
    sws_freeContext(swsContext);
}

void VideoChannel::stop() {

}

void VideoChannel::onSurfaceChanged(int width, int height) {
    dstWidth = width;
    dstHeight = height;
}
