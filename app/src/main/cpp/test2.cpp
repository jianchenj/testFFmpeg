//
// Created by Administrator on 2020/8/18.
//

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "test2.h"

/**
 * mp4 -> flv
 * @param argc
 * @param argv
 * @return
 */
int main2(int argc, char *argv[]) {

    AVOutputFormat *ofmt = nullptr;
    AVFormatContext *ifmt_ctx = nullptr;
    AVFormatContext *ofmt_ctx = nullptr;
    const char *in_filename, *out_filename;
    int ret, i;
    int video_stream = -1;
    int audio_stream = -1;
    int *stream_mapping = nullptr;
    int stream_mapping_size = 0;
    AVPacket pkt;

    av_register_all();
    ret = avformat_open_input(&ifmt_ctx, "in_filename todo", nullptr, nullptr);
    if (ret < 0) {
        goto end;
    }

    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, "todo_out_filename.flv");
    if (!ofmt_ctx) {
        av_log(nullptr, 0, "avformat_alloc_output_context2 error");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = new int[stream_mapping_size];
    for (int j = 0; j < ifmt_ctx->nb_streams; j++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[j];
        AVCodecParameters *in_codec_par = in_stream->codecpar;
        if (in_codec_par->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codec_par->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codec_par->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[j] = -1;
            continue;
        }

        out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!out_stream) {
            av_log(nullptr, 0, "out_stream null");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codec_par);
        if (ret < 0) {
            av_log(nullptr, 0, "avcodec_parameters_copy error");
            goto end;
        }

        out_stream->codecpar->codec_tag = 0;
    }

    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(nullptr, 0, "avio_open error");
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, 0, "avformat_write_header error");
        goto end;
    }

    while (1) {
        AVStream *in_stream, *outStream;
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) break;
        if (pkt.stream_index >= stream_mapping_size || stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        pkt.stream_index = stream_mapping[pkt.stream_index];

    }


    end:
    return -1;
    //todo 释放内存

//    avformat_alloc_output_context2()
//    avformat_free_context()
//
//    avformat_new_stream()
//
//    avcodec_parameters_copy()
//
//    avformat_write_header()
//
//    av_write_frame()
//    av_interleaved_write_frame()
//    av_write_trailer()
}