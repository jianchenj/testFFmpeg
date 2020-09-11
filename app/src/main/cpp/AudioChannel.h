//
// Created by Administrator on 2020/9/1.
//

#ifndef MY_APPLICATION_AUDIOCHANNEL_H
#define MY_APPLICATION_AUDIOCHANNEL_H


extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/time.h>
}
#include "BaseChannel.h"
#include "JavaCallHelper.h"
#include <SLES/OpenSLES_Android.h>
#include "macro.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                 AVRational time_base, AVFormatContext *formatContext);

    virtual void play();

    virtual void stop();

    void initOpenSL();

    void decoder();

    int getPcm();

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;
    int out_channels;//通道数
    int out_sample_size;//采样率
    int out_sample_rate;//采样频率.
    SwrContext* swrContext = nullptr;//重采样上下文
public:
    uint8_t * buffer;
    double  clock;//音视频同步使用
};


#endif //MY_APPLICATION_AUDIOCHANNEL_H
