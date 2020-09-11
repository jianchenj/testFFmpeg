//
// Created by Administrator on 2020/9/1.
//

#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
}

#include "JFFmpeg.h"
#include "macro.h"

/**
 * 区别于 RenderFrame 函数指针
 * 这里是个指针函数，返回一个指针
 * 返回viod，然后强转成想要的(有风险)
 *  https://blog.csdn.net/luoyayun361/article/details/80428882
 * @param args
 * @return
 */
void *prepareFFmpeg_(void *args) {
    //this强制转成FFmpeg对象
    JFFmpeg *jfFmpeg = static_cast<JFFmpeg *>(args);
    jfFmpeg->prepareFFmpeg();
    return 0;
}

void *startThread(void *args) {
    JFFmpeg *jfFmpeg = static_cast<JFFmpeg *>(args);
    jfFmpeg->play();
    return 0;
}

JFFmpeg::JFFmpeg(JavaCallHelper *_javaCallHelper, const char *dataSource) {
    url = new char[strlen(dataSource) + 1];
    this->javaCallHelper = _javaCallHelper;
    strcpy(url, dataSource);
}

JFFmpeg::~JFFmpeg() {

}

/**
 * 准备操作，准备线程中线程执行
 */
void JFFmpeg::prepare() {
    pthread_create(&pid_prepare, nullptr, prepareFFmpeg_, this);
}

/**
 * 准备 找到视频/音频流
 * 找到流对应 包含codec参数的 codecContext 解码上下文
 */
void JFFmpeg::prepareFFmpeg() {
    LOGI("<prepareFFmpeg> start");
    //avformat 既可以解码本地文件，也可以解码直播文件,一样的
    av_register_all();
    avformat_network_init();
    //解封装上下文，将视频解压为 视频流+音频流
    formatContext = avformat_alloc_context();

    //参数配置,其实就是一个key value
    AVDictionary *opts = nullptr;
    //设置超时时间3s（3000ms->3000000mms)
    av_dict_set(&opts, "timeout", "3000000", 0);
    char buf[1024];
    //开始打开视频文件
    int ret = avformat_open_input(&formatContext, url, nullptr, &opts);
    av_strerror(ret, buf, 1024);
    if (ret != 0) {
        LOGE("Couldn’t open file %s: %d(%s)", url, ret, buf);
        //回调java层
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    //解析流信息，保存到formatContext
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        LOGE("* * * * * * find stream info failure! * * * * * * * * *n %d", ret);
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    //遍历所有流
    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *stream = formatContext->streams[i];
        //找到流的解码信息
        AVCodecParameters *codecParameters = stream->codecpar;
        //根据解码参数，找解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }

        //解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }

        //将解码参数添加到解码器上下文
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_PARAMETERS_FAIL);
            }
            return;
        }

        //打开解码器
        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FIAL);
            }
            return;
        }

        //区分音视频
        if (AVMEDIA_TYPE_AUDIO == codecParameters->codec_type) {
            LOGI("find AUDIO stream number = %d", i);
            audioChannel = new AudioChannel(i, javaCallHelper, codecContext, stream->time_base,
                                            formatContext);
        } else if (AVMEDIA_TYPE_VIDEO == codecParameters->codec_type) {
            LOGI("find VIDEO stream number = %d", i);
            //视频的帧率
            AVRational av_frame_rate = stream->avg_frame_rate;

            int fps = av_q2d(av_frame_rate);

            //视频
            videoChannel = new VideoChannel(i, javaCallHelper, codecContext, stream->time_base,
                                            formatContext);
            videoChannel->setRenderFrame(renderFrame);
            videoChannel->setFps(fps);
        }
    }

    //视频/音频流都没有找到
    if (!audioChannel && !videoChannel) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_NO_MEDIA);
        }
        return;
    }

    //获取音频对象，用于音视频同步
    videoChannel->audioChannel = audioChannel;
    if (javaCallHelper) {
        javaCallHelper->onPrepare(THREAD_CHILD);
    }
    LOGI("<prepareFFmpeg> end");
}

/**
 * 打开播放标志，开始解码
 */
void JFFmpeg::start() {
    if (isPlaying) {
        LOGW("already Playing start canceled");
        return;
    }
    //起播成功
    isPlaying = true;
    //开启解码线程
    if (audioChannel) {
        audioChannel->play();
    }
    if (videoChannel) {
        // 读取packet-》frame->synchronized->window_buffer.
        videoChannel->play();
    }
    pthread_create(&pid_play, nullptr, startThread, this);
}

void JFFmpeg::play() {
    int ret = 0;
    while (isPlaying) {
        //如果队列数据大于100则延缓解码速度
        if (audioChannel && audioChannel->pkt_queue.size() > 100) {
            LOGI("audio pkt_queue enqueue delay ");
            //思想，生产者的速度远远大于消费者.  10ms.
            av_usleep(1000 * 10);
            continue;
        }
        if (videoChannel && videoChannel->pkt_queue.size() > 100) {
            LOGI("video pkt_queue enqueue delay ");
            av_usleep(1000 * 10);
            continue;
        }
        //读取包
        AVPacket *packet = av_packet_alloc();
        //从媒体中读取音视频的packet包.
        ret = av_read_frame(formatContext, packet);
//        LOGI(" ****************************************");
//        LOGI(" ret = %d", ret);
//        LOGI(" audioChannel = %d", audioChannel);
//        LOGI(" videoChannel = %d", videoChannel);
//        LOGI(" audioChannel AAAA !!!! channelId = %d", audioChannel->channelId);
//        LOGI(" videoChannel VVVVV !!!! channelId = %d", videoChannel->channelId);
//        LOGI(" packet->stream_index = %d", packet->stream_index);
//        LOGI(" ****************************************");
        if (ret == 0) {
            //将数据包packet加入队列
            if (audioChannel && packet->stream_index == audioChannel->channelId) {
                audioChannel->pkt_queue.enQueue(packet);
                LOGI("Auido pkt_queue enqueue size %d ", audioChannel->pkt_queue.size());
            } else if (videoChannel && packet->stream_index == videoChannel->channelId) {
                videoChannel->pkt_queue.enQueue(packet);
                LOGI("Video pkt_queue enqueue size %d ", videoChannel->pkt_queue.size());
            }
        } else if (ret == AVERROR_EOF) {
            //读取完毕，但是不一定播放完毕
            if (audioChannel->pkt_queue.empty() && videoChannel->pkt_queue.empty() &&
                audioChannel->frame_queue.empty() && videoChannel->frame_queue.empty()) {
                LOGW("播放完毕");
                break;
            }
        } else {
            LOGW("av_read_frame fail ret = %d", ret);
            break;
        }
    }
    isPlaying = false;
    if (audioChannel) {
        audioChannel->stop();
    }
    if (videoChannel) {
        videoChannel->stop();
    }
}

void JFFmpeg::pause() {

}

void JFFmpeg::close() {

}

void JFFmpeg::seek(long ms) {

}

void JFFmpeg::setRenderCallBack(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}

void JFFmpeg::onSurfaceChanged(int width, int height) {
    if (videoChannel) {
        videoChannel->onSurfaceChanged(width, height);
    }
}

