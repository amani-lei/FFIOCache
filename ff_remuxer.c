    //
//  ff_remuxer.c
//
//  Created by helei on 2017/12/21.
//  Copyright © 2017年 <helei0908@hotmail.com>. All rights reserved.
//

#include "ff_remuxer.h"
#include "libavformat/avformat.h"
#include "ff_log.h"

int remuxer_init_ffmpeg(){
    av_register_all();
    avformat_network_init();
    return 0;
}

static int REMUXER_FF_INIT_FLAG = 0;

RemuxerContext * remuxer_context_alloc(void){
    if(REMUXER_FF_INIT_FLAG == 0){
        remuxer_init_ffmpeg();
        REMUXER_FF_INIT_FLAG = 1;
    }
    RemuxerContext * ctx = malloc(sizeof(RemuxerContext));
    if(ctx){
        ctx->ready = 0;
        ctx->error_code = REMUXER_ERROR_NONE;
        ctx->in_fmt_ctx = NULL;
        ctx->out_fmt_ctx = NULL;
    }
    return ctx;
}

//int remuxer_out_stream_init(AVStream *out_stream, AVStream *in_stream){
//    int ret = 0;
//    ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
//    if(ret < 0){
//        return -1;
//    }
//    out_stream->codecpar->codec_tag = 0;
//    return 0;
//}

int remuxer_config_hls(AVFormatContext * in_ctx, AVFormatContext * out_ctx){
    //用来格式化hls的ts文件名
    char ts_format[1024] = {0};
    strcpy(ts_format, out_ctx->filename);
    char *ext=strrchr(ts_format,'.');
    if(ext == NULL){
        LOGE("%s:Can`t juege output format\n", __FUNCTION__);
        return -1;
    }
    if (ext)*ext=0;
    strcat(ts_format, "_%d.ts");
    if(av_opt_set(out_ctx->priv_data, "hls_segment_filename", ts_format, AV_OPT_SEARCH_CHILDREN)){
        LOGW("%s:config hls_segment_filename failed ! \n", __FUNCTION__);
    }
    //按时长切片

    int suggested_size = 1024 * 1024 * 8;
    int in_bitrate = 0;
    for(int i = 0; i < in_ctx->nb_streams; i++){
        in_bitrate += in_ctx->streams[i]->codecpar->bit_rate;
    }
    double hls_time = 1;
    if(in_bitrate != 0){
        hls_time = suggested_size / in_bitrate;
    }
    if(hls_time == 0){
        hls_time = 1;
    }
    
    av_opt_set_double(out_ctx->priv_data, "hls_time", hls_time, AV_OPT_SEARCH_CHILDREN);
    //start number of ts name format
    if(av_opt_set_int(out_ctx->priv_data, "start_number", 1, AV_OPT_SEARCH_CHILDREN)){
        LOGW("%s:config start_number failed ! \n", __FUNCTION__);
    }
    if(av_opt_set_int(out_ctx->priv_data, "hls_list_size", 0, AV_OPT_SEARCH_CHILDREN)){
        LOGW("%s:config hls_list_size failed ! \n", __FUNCTION__);
    }
    return 0;
}

int remuxer_config_mp4(AVFormatContext * out_ctx){
    int ret = 0;
    //将moov-box放到最前面,加快首播速度
    ret = av_opt_set(out_ctx->priv_data, "movflags", "+faststart", AV_OPT_SEARCH_CHILDREN);
    return 0;
}

AVStream * remuxer_add_out_stream(RemuxerContext * ctx, AVStream * in_stream, AVFormatContext * out_fmt_ctx){
    AVStream * out_stream = NULL;
    if(!avformat_query_codec(out_fmt_ctx->oformat, in_stream->codecpar->codec_id, FF_COMPLIANCE_UNOFFICIAL)){
        LOGE("%s: unsupported format %s in output format %s ! \n",__FUNCTION__, avcodec_get_name(in_stream->codecpar->codec_id), out_fmt_ctx->oformat->name);
        goto err;
    }
    out_stream = avformat_new_stream(out_fmt_ctx, NULL);
    if(out_stream == NULL){
        LOGE("%s:Can`t add new stream to output file\n", __FUNCTION__);
        goto err;
    }
    avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
    out_stream->codecpar->codec_tag = 0;
    if(in_stream->metadata){
        av_dict_copy(&out_stream->metadata, in_stream->metadata, 0);
    }
    ctx->stream_map[in_stream->index] = out_stream->index;
    return out_stream;
err:
    return NULL;
}

RemuxerContext * remuxer_alloc(const char * inFile, const char * outFile){
    int ret = 0;
    AVFormatContext *in_fmt_ctx = NULL;

    if(inFile == NULL || outFile == NULL){
        LOGE("%s:input/output file is invalid \n", __FUNCTION__);
        goto err;
    }
    
    //输入信息分析
    ret = avformat_open_input(&in_fmt_ctx, inFile, NULL, NULL);
    if(ret != 0){
        LOGE("%s:Can`t open input file !\n", __FUNCTION__);
        goto err;
    }
    if(avformat_find_stream_info(in_fmt_ctx, NULL) < 0){
        goto err;
    }
    RemuxerContext * remuxer = remuxer_alloc2(in_fmt_ctx, outFile);
    remuxer->is_alloc2 = 0;
err:
    if(in_fmt_ctx){
        avformat_free_context(in_fmt_ctx);
        in_fmt_ctx = NULL;
    }

    return NULL;
}

RemuxerContext* remuxer_alloc2(AVFormatContext * in_fmt_ctx, const char * outFile){
    AVFormatContext * out_fmt_ctx = NULL;
    RemuxerContext * remux_ctx = remuxer_context_alloc();
    if(remux_ctx == NULL){
        LOGE("%s:memory error ! \n", __FUNCTION__);
        goto err;
    }
    int ret = 0;
    //构造输出数据
    ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, NULL, outFile);
    if(ret < 0){
        LOGE("%s: Unsupported output format!\n", __FUNCTION__);
        remux_ctx->error_code = REMUXER_ERROR_IO;
        goto err;
    }
    if(in_fmt_ctx->metadata){
        av_dict_copy(&out_fmt_ctx->metadata, in_fmt_ctx->metadata, 0);
    }
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        remuxer_add_out_stream(remux_ctx, in_fmt_ctx->streams[i], out_fmt_ctx);
    }
    if(!(out_fmt_ctx->flags & AVFMT_NOFILE)){
        ret = avio_open(&out_fmt_ctx->pb, outFile, AVIO_FLAG_WRITE);
        if(ret < 0){
            LOGE("%s:Can`t open output file %s, Is folder exist ?\n", __FUNCTION__, outFile);
            goto err;
        }
        
        ret = 0;
        
        //这里对参数配置做一些判断, 一般来说,配置错误,不会引起其他异常
        if(strcmp(out_fmt_ctx->oformat->name, "hls") == 0){
            ret = remuxer_config_hls(in_fmt_ctx, out_fmt_ctx);
        }else if(strcmp(out_fmt_ctx->oformat->name, "mp4") == 0){
            ret = remuxer_config_mp4(out_fmt_ctx);
        }else{
            //other format....
        }
        if(ret != 0){
            LOGW("%s:config param failed for output format -> %s !\n", __FUNCTION__, out_fmt_ctx->oformat->name);
        }
    }
    ret = avformat_write_header(out_fmt_ctx, NULL);
    if(ret < 0){
        LOGE("%s:write header to output file failed !\n", __FUNCTION__);
        remux_ctx->error_code = REMUXER_ERROR_IO;
        goto err;
    }else if(ret == AVSTREAM_INIT_IN_WRITE_HEADER || ret == AVSTREAM_INIT_IN_INIT_OUTPUT){
        //succeed
    }
    
    //为RemuxerContext赋值
    remux_ctx->in_fmt_ctx = in_fmt_ctx;
    remux_ctx->out_fmt_ctx = out_fmt_ctx;
    remux_ctx->ready = 1;
    remux_ctx->is_alloc2 = 1;
    return remux_ctx;
err:
    if(in_fmt_ctx){
        avformat_close_input(&in_fmt_ctx);
    }
    if(out_fmt_ctx){
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
    }
    if(remux_ctx){
        remuxer_delete(&remux_ctx);
    }
    return NULL;
}

int remuxer_do(RemuxerContext * ctx){
    if(ctx == NULL){
        return REMUXER_ERROR_PARAM;
    }

    int ret = 0;
    AVPacket read_packet;
    AVStream *in_stream = NULL, *out_stream = NULL;
    //用来保存每个流的上一个解码时间戳
    int64_t last_dts[64] = {0};
    for(int i = 0; i < 64; i++){
        last_dts[i] = AV_NOPTS_VALUE;
    }
    
    if(ctx->ready == 0){
        ctx->error_code = REMUXER_ERROR_UNINITED;
        LOGE("%s:remuxer is not ready\n", __FUNCTION__);
        goto err;
    }
    while(1){
        ret = av_read_frame(ctx->in_fmt_ctx, &read_packet);
        if(ret < 0){
            LOGE("%s:end of input or read packet failed !\n", __FUNCTION__);
            break;
        }
        if(last_dts[read_packet.stream_index] == AV_NOPTS_VALUE){
            if(read_packet.dts == AV_NOPTS_VALUE){
                read_packet.dts = 0;
            }
            last_dts[read_packet.stream_index] = read_packet.dts;
        }else if(read_packet.dts <= last_dts[read_packet.stream_index]){
            read_packet.dts = ++last_dts[read_packet.stream_index];
        }
        
        in_stream = ctx->in_fmt_ctx->streams[read_packet.stream_index];
        out_stream = ctx->out_fmt_ctx->streams[ctx->stream_map[read_packet.stream_index]];
        
        last_dts[read_packet.stream_index] = read_packet.dts;
        
        read_packet.stream_index = out_stream->index;
        read_packet.pts = av_rescale_q_rnd(read_packet.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        read_packet.dts = av_rescale_q_rnd(read_packet.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        read_packet.duration = av_rescale_q(read_packet.duration, in_stream->time_base, out_stream->time_base);
        read_packet.pos = -1;
        ret = av_interleaved_write_frame(ctx->out_fmt_ctx, &read_packet);
        if(ret < 0){
            LOGE("%s:Can`t write packet to output file !\n", __FUNCTION__);
            ctx->error_code = REMUXER_ERROR_IO;
            //av_packet_unref(&read_packet);
            //goto err;
        }
        av_packet_unref(&read_packet);
    }
    av_packet_unref(&read_packet);
    
    av_write_trailer(ctx->out_fmt_ctx);
    avformat_flush(ctx->out_fmt_ctx);
    //文件格式才需要关闭
    if(!(ctx->out_fmt_ctx->flags & AVFMT_NOFILE)){
        avio_closep(&ctx->out_fmt_ctx->pb);
    }
    return REMUXER_ERROR_NONE;
err:
    return ctx->error_code;
}

void remuxer_delete(RemuxerContext ** pctx){

    if(pctx == NULL || *pctx == NULL)return;
    RemuxerContext * ctx = *pctx;
    if(ctx->is_alloc2 && ctx->in_fmt_ctx){
        avformat_close_input(&ctx->in_fmt_ctx);
        //avformat_free_context(ctx->in_fmt_ctx);
    }
    if(ctx->out_fmt_ctx){
        avformat_free_context(ctx->out_fmt_ctx);
    }
    free(ctx);
    ctx = NULL;
    *pctx = NULL;
}

const char * remuxer_get_error_desc(RemuxerContext * ctx){
    switch(remuxer_get_error_code(ctx)){
        case REMUXER_ERROR_NONE:    return "无错误";
        case REMUXER_ERROR_PARAM:   return "传入的参数无效";
        case REMUXER_ERROR_UNINITED:    return "再复用器初始化失败";
        case REMUXER_ERROR_IO:          return "输入/输出失败";
        case REMUXER_ERROR_UNSUPPORT_FORMAT:    return "不支持的格式";
        case REMUXER_ERROR_MEMORY:  return "内存错误";
        case REMUXER_ERROR_UNKNOW:
        default:
            return "未知错误";
    }
    return "未知错误";
}

int remuxer_get_error_code(RemuxerContext * ctx){
    if(ctx == NULL){
        return REMUXER_ERROR_UNKNOW;
    }
    return ctx->error_code;
}
