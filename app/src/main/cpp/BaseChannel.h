//
// Created by Administrator on 2020/9/1.
//

#ifndef MY_APPLICATION_BASECHANNEL_H
#define MY_APPLICATION_BASECHANNEL_H


extern "C"
{
#include <libavutil/rational.h>
//封装格式，总上下文
#include "libavformat/avformat.h"
//解码器.
#include "libavcodec/avcodec.h"
//#缩放
#include "libswscale/swscale.h"
// 重采样
#include "libswresample/swresample.h"
//时间工具
#include "libavutil/time.h"
//编码转换工具yuv->rgb888
#include "libavutil/imgutils.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JavaCallHelper.h"
#include "SafeQueue.h"
#include "macro.h"

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                AVRational time_base) : channelId(id),
                                        javaCallHelper(javaCallHelper), codecContext(codecContext),
                                        time_base(time_base) {

    }

    ~BaseChannel() {
        //销毁解码器上下文
        if (codecContext) {
            avcodec_close(codecContext);
            avcodec_free_context(&codecContext);
            codecContext = 0;
        }
        //销毁队列
        //todo
        //pkt_queue.clear();
        //frame_queue.clear();
        LOGI("释放channel:%d %d", pkt_queue.size(), frame_queue.size());
    }

    virtual void play() = 0;

    virtual void stop() = 0;

    static void releaseAvPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame *&frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = 0;
        }
    }

public:
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *codecContext;
    AVFormatContext *formatContext;
    JavaCallHelper *javaCallHelper;
    AVRational time_base;
};


#endif //MY_APPLICATION_BASECHANNEL_H
