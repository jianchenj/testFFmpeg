//
// Created by Administrator on 2020/9/1.
//

#ifndef MY_APPLICATION_JFFMPEG_H
#define MY_APPLICATION_JFFMPEG_H

#include <pthread.h>
#include "JavaCallHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

/**
 * FFmpeg控制类
 */
class JFFmpeg {
public:
    /**
     * 构造函数
     * @param _javaCallHelper 回调控制类
     * @param dataSource 播放源
     */
    JFFmpeg(JavaCallHelper* _javaCallHelper, const char* dataSource);
    ~JFFmpeg();

    void setRenderCallBack(RenderFrame renderFrame);
    void prepare();
    void prepareFFmpeg();
    void start();
    void play();
    void pause();
    void close();
    void seek(long ms);
    void onSurfaceChanged(int width , int height);

private:
    bool isPlaying;
    pthread_t pid_prepare;//准备线程的id
    pthread_t pid_play;//解码线程, 一直存在，直到播放完成
    char* url;//播放源
    JavaCallHelper *javaCallHelper;
    RenderFrame renderFrame;
    AVFormatContext *formatContext;
    AudioChannel *audioChannel;
    VideoChannel *videoChannel;
};


#endif //MY_APPLICATION_JFFMPEG_H
