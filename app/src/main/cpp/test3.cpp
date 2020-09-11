//
// Created by Administrator on 2020/8/24.
//

#include "test3.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * H264 编码
 *
 */



// 对每一帧进行编码
static void encode(AVCodecContext *c, AVFrame *frame, AVPacket *pkt,
                   FILE *f) {
    int ret = 0;
    /**
        * encode the image
        */
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(c, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            exit(0);
        } else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        fwrite(pkt->data, 1, pkt->size, f);
        av_packet_unref(pkt);
    }
}

int main3(int argc, char *argv[]) {
    const char *filename, *codec_name;
    const AVCodec *codec;
    AVCodecContext *c = NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;

    uint8_t endcode[] = {0, 0, 1, 0xb7};

    if (argc <= 2) {
        fprintf(stderr, "Usage： %s <output file> <code name> \n", argv[0]);
        exit(0);
    }
    filename = argv[1];
    codec_name = argv[2];

    avcodec_register_all();

    /**
     * find the mpeg1video encoder
     */
    codec = avcodec_find_encoder_by_name(codec_name);
    //codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "codec find fail !!");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "avcodec_alloc_context3 fail !!");
        exit(1);
    }

    /**
     * put sample parameters
     */
    c->bit_rate = 400000;
    /**
     * resolution must be a multiple of two
     */
    c->width = 352;
    c->height = 288;
    /**
     * frames per second
     */
    c->time_base = (AVRational) {1, 25};
    c->framerate = (AVRational) {25, 1};

    /**
     * emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be a I frame irrespective to gop_size
     */
    c->gop_size = 10;//一组图片里有一张I帧
    c->max_b_frames = 1;//b前后参考帧 , p向前参考帧
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264) {
        //采用预先设定好的h264参数，slow代表质量高，编码速度会慢
        av_opt_set(c->priv_data, "preset", "slow", 0);
    }

    /**
     * open it
     */
    if (avcodec_open2(c, codec, nullptr) < 0) {
        fprintf(stderr, "avcodec_open2 fail !!");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "fopen %s\n fail !!", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "av_frame_alloc fail !!");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "av_frame_get_buffer fail !!");
        exit(1);
    }

    /**
     * encode 1 second of video
     * // 这里是人工添加数据模拟生成1秒钟(25帧)的视频(真实应用中是从摄像头获取的原始数据，摄像头拿到数据后会传给编码器，然后编码器进行编码形成一帧帧数据。
     */
    for (i = 0; i < 25; i++) {
        av_init_packet(&pkt);
        pkt.data = nullptr;// packet data will be allocated by the encoder
        pkt.size = 0;

        fflush(stdout);// 强制输出写入文件
        /**
         * make sure the frame data is writeable
         */
        ret = av_frame_make_writable(frame);
        if (ret < 0) {
            exit(1);
        }

        /**
         * prepare a dummy image
         *
         */
        /* Y */
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for (y = 0; y < c->height / 2; y++) {
            for (x = 0; x < c->width / 2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i;

        encode(c, frame, &pkt, f);
    }
    /* flush the encoder */
    encode(c, nullptr, &pkt, f);

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);

    avcodec_free_context(&c);
    av_frame_free(&frame);

    return 0;
}