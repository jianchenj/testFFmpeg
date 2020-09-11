//
// Created by Administrator on 2020/9/1.
//

#ifndef MY_APPLICATION_VIDEOCHANNEL_H
#define MY_APPLICATION_VIDEOCHANNEL_H

#include <cstdint>

extern "C" {
#include <libavformat/avformat.h>
}

#include "BaseChannel.h"
#include "AudioChannel.h"

/**
 * 视频channel
 */
//定义一个函数指针类型(就是指向一个函数)，
//这个类型的名字叫做RenderFrame，函数具体是干什么，看他指向的函数,
//这个函数的参数是 uint8_t , int, int, int，
//这个函数返回值是 void
//dst_data[0], dst_linesize[0], codecContext->width, codecContext->height
typedef void (*RenderFrame)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper1, AVCodecContext *codecContext1,
                 AVRational time_base, AVFormatContext *formatContext);

    void setRenderFrame(RenderFrame renderFrame);

    void setFps(int fps);

    virtual void play();

    virtual void stop();

    void decodePacket();

    void synchronizeFrame();
    //上层surface产生变化
    virtual void onSurfaceChanged(int width, int height);
private:
    RenderFrame renderFrame;
    int fps;
    pthread_t pid_video_play;
    pthread_t pid_synchronize;
    int dstWidth;
    int dstHeight;

public:
    //用于音视频同步，一般都是视频根据音频播放进度调整
    AudioChannel *audioChannel;
    double clock;
};


#endif //MY_APPLICATION_VIDEOCHANNEL_H
