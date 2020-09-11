//
// Created by Administrator on 2020/8/27.
//

#include "test4.h"

#include <stdio.h>


extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}


const int decode_write_frame(const char *outFileName, AVCodecContext *avctx,
                             struct SwsContext *img_convert_ctx, AVFrame *frame, int *frame_count,
                             AVPacket *pkt, int last) {


}

int main4(int argc, char **argv) {
    int ret;
    FILE *f;
    const char *filename, *out_filename;

    AVFormatContext *fmt_ctx = nullptr;

    const AVCodec *codec;
    AVCodecContext *c = nullptr;

    AVStream *st = nullptr;
    int stream_index;

    int frame_count;
    AVFrame *frame;

    struct SwsContext *img_convert_ctx;

    AVPacket *avPacket = nullptr;

    if (argc < 2) {
        fprintf(stderr, "   argc < 2");
        exit(0);
    }

    filename = argv[1];
    out_filename = argv[2];

    /**
     * register all formats and codecs
     */
    av_register_all();

    /**
     * open file and allocate format context
     */
    if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr)) {
        fprintf(stderr, "avformat_open_input error");
        exit(1);
    }

    /**
     * retrieve stream information
     */
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "avformat_find_stream_info error");
        exit(1);
    }

    av_dump_format(fmt_ctx, 0, filename, 0);

    av_init_packet(avPacket);

    stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (stream_index < 0) {
        fprintf(stderr, "av_find_best_stream error");
        exit(1);
    }
    st = fmt_ctx->streams[stream_index];

    /**
     * find decoder for the stream
     */
    codec = avcodec_find_encoder(st->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Failed to find %s codec", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return AVERROR(EINVAL);//EINVAL 无效参数
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Failed to find avcodec_alloc_context3");
        exit(1);
    }

    /**
     * copy codec parameters from input stream to output codec context
     */
    ret = avcodec_parameters_to_context(c, st->codecpar);
    if (ret < 0) {
        fprintf(stderr, "Failed to copy %s codec paras to decoder ",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return ret;
    }

    /**
     * for some codecs, such as msmpeg4 and mpeg4, width and height must be initialized there because this information is
     * not available in the bitstream
     * open it
     */
    if (avcodec_open2(c, codec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec");
        exit(1);
    }
    img_convert_ctx = sws_getContext(c->width, c->height, c->pix_fmt, c->width, c->height,
                                     AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                     nullptr, nullptr, nullptr);

    if (img_convert_ctx == nullptr) {
        fprintf(stderr, "sws_getContext fail !");
        exit(1);
    }



    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "av_frame_alloc fail !");
        exit(1);
    }
    frame_count = 0;
    while (av_read_frame(fmt_ctx, avPacket) >= 0) {
        /**
        avPacket->size = fread(inbuf, 1, INBUF_SIZE , f);
        if (avPacket->size == 0) break;
         */
        /**
         * NOTE1: some codecs are stream based (mpegvideo, mpegaudio), and this is the only
         * method to use them because you cannot know the compressed data size before analysing it
         *
         * BUT some other codecs (msmpeg4, mpeg4) are inherently frame based, so you must call them with
         * all the data for one frame exactly. you must also initialize 'width' and 'height' before
         * initialize them
         */
        /**
         * NOTE2: some codecs allow the raw parameters (frame size, sample rate) to be changed at any frame.
         * we handle this, so you should also take care of it
         */
        /**
         * here we use a stream based decoder(mpegvideo), so we feed decoder and see if it could decode a frame
         */
        //avPacket->data = inbuf;
        //while(avPacket->size > 0)
        if (avPacket->stream_index == stream_index) {
            //todo
            // if (decode_write_frame()) {

            //}
            av_packet_unref(avPacket);
        }
    }

}

