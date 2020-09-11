//
// Created by Administrator on 2020/9/1.
//
#include "AudioChannel.h"


void *audioPlay(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *> (args);
    audioChannel->initOpenSL();
    return 0;
}

void *audioDecode(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *> (args);
    audioChannel->decoder();
    return 0;
}

void bpPlayerCallback(SLAndroidSimpleBufferQueueItf bp, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int data_len = audioChannel->getPcm();
    LOGI("<bpPlayerCallback> data_len %d", data_len);
    if (data_len > 0) {
        (*bp)->Enqueue(bp, audioChannel->buffer, data_len);
    }
}

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,
                           AVRational time_base, AVFormatContext *formatContext) : BaseChannel(id,
                                                                                               javaCallHelper,
                                                                                               codecContext,
                                                                                               time_base) {
    LOGI("AudioChannel init");
    this->javaCallHelper = javaCallHelper;
    this->codecContext = codecContext;
    this->formatContext = formatContext;
    //初始化音频参数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);//每个采样所占的比特数，2个字节
    out_sample_rate = 44100;
    //CD音频标准
    //44100HZ 双声道 2字节
    buffer = (uint8_t *) malloc(out_channels * out_sample_rate * out_sample_size);//计算每一个音频帧的大小
    pkt_queue.setReleaseCallback(releaseAvPacket);
    frame_queue.setReleaseCallback(releaseAVFrame);
}

void AudioChannel::play() {
    LOGI("AudioChannel <play>");
    //重采样，设置采样参数，创建采样上下文
    swrContext = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                    out_sample_rate,
                                    codecContext->channel_layout, codecContext->sample_fmt,
                                    codecContext->sample_rate, 0, 0);
    swr_init(swrContext);
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    //初始化opensl es线程
    pthread_create(&pid_audio_play, nullptr, audioPlay, this);
    //将AvFrame转化成yuv数据
    pthread_create(&pid_audio_decode, nullptr, audioDecode, this);
}

//初始化opensl es线程
void AudioChannel::initOpenSL() {
    LOGW("initOpenSL");
    //1 引擎接口
    SLEngineItf engineInterface = nullptr;
    //2 引擎
    SLObjectItf engineObject = nullptr;
    //3 混音器
    SLObjectItf outputMixObject = nullptr;
    //4 播放器
    SLObjectItf bpPlayerObject = nullptr;
    //5 回调接口
    SLPlayItf bpPlayerInterface = nullptr;
    // 缓冲队列
    SLAndroidSimpleBufferQueueItf bpPlayerBufferQueue;

    //------------------- 初始化引擎 -------------------
    SLresult result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" slCreateEngine fail !");
        return;
    }
    //实例化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" Realize engineObject fail !");
        return;
    }
    //获取音频接口
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" GetInterface engineInterface fail !");
        return;
    }

    //----------------- 初始化混音器 -----------------
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    //创建混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 1, ids, req);
    //初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //设置播放参数
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,//播放pcm数据格式
                            2,//双声道
                            SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
                            SL_BYTEORDER_LITTLEENDIAN//小端模式，大端不能播放
    };
    SLDataSource audioSrc = {&android_queue, &pcm};

    //config audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&outputMix, nullptr};
    //只是简单播放，不需要调音
    const SLInterfaceID id2[1] = {SL_IID_BUFFERQUEUE/*, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            SL_IID_MUTESOLO,*/};
    const SLboolean req2[1] = {SL_BOOLEAN_TRUE/*, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,*/ };
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bpPlayerObject,//播放器
                                          &audioSrc,//播放器参数，播放器缓冲队列， 播放格式
                                          &audioSink,//播放缓冲区
                                          1,//播放回调接口数
                                          id2,//设置播放队列id
                                          req2//是否采用内置的播放队列
    );
    //实例化播放器
    result = (*bpPlayerObject)->Realize(bpPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" Realize bpPlayerObject fail !");
        return;
    }
    LOGI("realize the player success!");
    //获取播放器接口player interface
    result = (*bpPlayerObject)->GetInterface(bpPlayerObject, SL_IID_PLAY, &bpPlayerInterface);
    //获取pcm缓冲队列
    result = (*bpPlayerObject)->GetInterface(bpPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bpPlayerBufferQueue);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" GetInterface SL_IID_BUFFERQUEUE fail !");
        return;
    }
    LOGI("get the buffer queue interface success!");

    //register callback on the buffer queue
    result = (*bpPlayerBufferQueue)->RegisterCallback(bpPlayerBufferQueue, bpPlayerCallback, this);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" RegisterCallback fail !");
        return;
    }
    result = (*bpPlayerInterface)->SetPlayState(bpPlayerInterface, SL_PLAYSTATE_PLAYING);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(" SetPlayState playing fail !");
        return;
    }

    LOGI("SetPlayState SL_PLAYSTATE_PLAYING success!");
    //手动启动第一次回调函数
    LOGI("手动调用播放器 packet:%d", this->pkt_queue.size());
    bpPlayerCallback(bpPlayerBufferQueue, this);
}

/**
 * 从packet的队列中取出AVPacket，解码成AVFrame 入frame的队列
 */
void AudioChannel::decoder() {
    LOGI("AudioChannel::decoder()  ! isPlaying: %d", isPlaying);
    AVPacket *packet = nullptr;
    while (isPlaying) {
        //音频的packet
        LOGI("AudioChannel::decoder isPlaying = %d", isPlaying);
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(codecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            LOGE("AudioChannel::avcodec_send_packet EAGAIN 等待数据包！");
            //需要更多数据
            continue;
        } else if (ret < 0) {
            LOGE("AudioChannel::avcodec_send_packet fail ret < 0 %d", ret);
            //失败
            break;
        }

        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, avFrame);
        LOGI("AudioChannel::avcodec_receive_frame success ! avFrame:%s", avFrame);

        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            LOGE("AudioChannel::avcodec_receive_frame FAilure ret < 0 %d", ret);
            //失败
            break;
        }
        frame_queue.enQueue(avFrame);
        LOGI("AudioChannel::frame_queue enQueue success ! :%d", frame_queue.size());
        while (frame_queue.size() > 100 && isPlaying) {
            LOGE("AudioChannel:: frame_queue %d is full, sleep 16 ms", frame_queue.size());
            av_usleep(16*1000);
            continue;
        }
    }
    releaseAvPacket(packet);
}

/**
 * 获取音频解码的pcm
 * 取出在decode方法中入队的AVFrame
 * @return
 */
int AudioChannel::getPcm() {
    LOGI("AudioChannel::getPcm()  %d", frame_queue.size());
    AVFrame *frame = 0;
    int data_size = 0;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            LOGW("AudioChannel not playing frame");
            break;
        }
        if (!ret) {
            LOGE("AudioChannel frame_queue.deQueue ret continue");
            continue;
        }
        //frame -> 转化为pcm数据
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext, frame->sample_rate) + frame->nb_samples,
                out_sample_rate, frame->sample_rate, AV_ROUND_UP);
        //转换 返回值为转换后的sample个数
        //buffer 是opensl 回调 bpPlayerCallback 里填充的
        int nb = swr_convert(swrContext, &buffer, dst_nb_samples, (const uint8_t **) frame->data, frame->nb_samples);
        //计算转换后buffer的大小 44100*2（采样位数2个字节）*2（双通道）.  。
        data_size = nb * out_channels * out_sample_size;

        //计算当前音频的播放时钟clock， pts相对数量 time_base:时间单位(1,25) 表示1/25分之一秒
        clock = frame->pts * av_q2d(time_base);
        break;
    }
    releaseAVFrame(frame);
    return data_size;
}

void AudioChannel::stop() {

}

