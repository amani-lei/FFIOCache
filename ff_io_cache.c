//
//  ff_io_cache.c
//  ffmpeg_demo
//
//  Created by helei on 2018/8/9.
//  Copyright © 2018年 何磊 <helei0908@hotmail.com>. All rights reserved.
//

#include "ff_io_cache.h"
#include "ff_header.h"
#include "ff_md5.h"
#include "dirent.h"
#include "sys/stat.h"
#include "unistd.h"
#include "ff_log.h"
#include "ff_macro.h"
#include "ff_remuxer.h"
#include "ff_file.h"

#define FF_IO_CACHE_DIR_LENGHT  (1024)
char g_cache_dir[FF_IO_CACHE_DIR_LENGHT] = {0};
int ff_io_cache_global_check(){
    if(strlen(g_cache_dir) == 0){
        LOGE("%s:cache dir is null, did called ff_io_cache_global_set_path() ?", __FUNCTION__);
        return -1;
    }
    if(access(g_cache_dir, S_IRWXU) != 0){
        LOGE("%s:Can`t read/write cache path '%s', is it exist and can be read & write ?\n", __FUNCTION__, g_cache_dir);
        return -1;
    }
    return 0;
}

int ff_io_cache_global_set_path(const char * cache_dir){
    if(cache_dir == NULL){
        LOGE("%s:param error\n", __FUNCTION__);
        return -1;
    }
    size_t len = strlen(cache_dir);
    if(len == 0){
        LOGE("%s:seting a invalid cache path\n", __FUNCTION__);
        return -1;
    }
    if(len >= FF_IO_CACHE_DIR_LENGHT){
        LOGE("%s:cache dir is too long, max lenght is %d\n", __FUNCTION__, FF_IO_CACHE_DIR_LENGHT - 1);
    }
    int ret = access(cache_dir, F_OK);
    if(ret != 0){
        ret = mkdir(cache_dir, S_IRWXU);
    }
    if(ret != 0){
        LOGE("%s:Can`t access [%s]! \n", __FUNCTION__, cache_dir);
        return -1;
    }
    
    strcpy(g_cache_dir, cache_dir);
    if(g_cache_dir[len - 1] == '/'){
        g_cache_dir[len - 1] = 0;
    }
    return 0;
}

int ff_io_cache_global_del_all(){
    int ret = access(g_cache_dir, F_OK);
    if(ret != 0){
        LOGW("%s:dir '%s' is not exsit", __FUNCTION__, g_cache_dir);
        return 0;
    }
    ret = remove_dir(g_cache_dir);
    if(mkdir(g_cache_dir, S_IRWXU)){
        LOGE("%s:recreate cache_dir '%s' failed\n", __FUNCTION__, g_cache_dir);
        return -1;
    }
    return 0;
}

int ff_io_cache_global_del(const char * url){
    if(ff_io_cache_global_check() != 0){
        LOGE("%s:global check failed, please set cache path use ff_io_cache_gloabl_set_path()\n",__FUNCTION__);
        return -1;
    }
    char md5[36] = {0};
    ff_str_md5(url, md5);
    char path[1024] = {0};
    snprintf(path, 1023, "%s/%s", g_cache_dir, md5);
    if(access(path, S_IRWXU) == 0){
        LOGE("%s:Can`t read/write path '%s', is it exist and can be read & write ?\n", __FUNCTION__, path);
        return -1;
    }
    remove(path);
    return 0;
}

int ff_io_cache_global_export(const char * url, const char * file_name, const char * fmt_name){
    if(ff_io_cache_global_check() != 0){
        LOGE("%s:global check failed, please set cache path use ff_io_cache_gloabl_set_path()\n",__FUNCTION__);
        return -1;
    }
    int ret = -1;
    AVFormatContext * in_format = NULL;
    RemuxerContext * remuxer = NULL;
    ret = ff_io_cache_format_open_input(&in_format, url, NULL, NULL, FFCACHE_FLAG_ENABLE_CACHE_READ);
    if(ret != 0){
        LOGE("%s:Can`t create io_cache\n", __FUNCTION__);
        goto err;
    }
    ret = avformat_find_stream_info(in_format, NULL);
    if(ret < 0){
        LOGE("%s:Can`t find stream info from source\n", __FUNCTION__);
    }
    remuxer = remuxer_alloc2(in_format, file_name);
    if(remuxer == NULL){
        LOGE("%s:Can`t create remuxer\n", __FUNCTION__);
        goto err;
    }
    ret = remuxer_do(remuxer);
    if(ret != 0){
        LOGE("%s:Do remuxer failed\n", __FUNCTION__);
        goto err;
    }
    if(remuxer){
        remuxer_delete(&remuxer);
    }
    ff_io_cache_format_free_context(in_format);
    return 0;
err:
    if(remuxer){
        remuxer_delete(&remuxer);
        remuxer = NULL;
    }
    ff_io_cache_format_free_context(in_format);
    return -1;
}

int ff_io_cache_init_src_io(FFCacheContext * ctx){
    if(ctx == NULL || ctx->curr_src_url == NULL || strlen(ctx->curr_src_url) == 0){
        return -1;
    }
    if(ctx->back_src_ic != NULL){
        return 0;
    }
    
    AVIOContext * back_src_ic = NULL;
    int ret = avio_open(&back_src_ic, ctx->curr_src_url, AVIO_FLAG_READ);
    if(ret < 0){
        goto err;
    }
    if(!fff_cache_is_complete(ctx->cache_file_ctx)){
        int64_t size = avio_size(back_src_ic);
        if(size >= 0){
            fff_cache_set_size(ctx->cache_file_ctx, size);
        }
    }
    ctx->back_src_ic = back_src_ic;
    return 0;
err:
    return -1;
}

int ff_io_cache_on_open(struct AVFormatContext *s, AVIOContext **pb, const char *url,
               int flags, AVDictionary **options){
    LOGD("%s:on open url\"%s\"\n", __FUNCTION__, url);
    FFCacheContext * cache_ctx = (FFCacheContext*) s->opaque;
    //创建ffmpeg的回源io
    ff_io_cache_uninstall_current(cache_ctx);
    
    AVIOContext * agent_ic = avio_alloc_context(cache_ctx->io_buffer, cache_ctx->io_buffer_size, 0, cache_ctx, ff_io_cache_read_packet, NULL, ff_io_cache_read_seek);
    *pb = agent_ic;
    cache_ctx->curr_src_url = malloc(strlen(url) + 1);
    strcpy(cache_ctx->curr_src_url, url);
    cache_ctx->agent_ic = agent_ic;
    char md5[36] = {0};
    ff_str_md5(url, md5);
    cache_ctx->curr_uid = malloc(36);
    strcpy(cache_ctx->curr_uid, md5);
    FFFileCacheContext * file_cache = fff_cache_alloc(cache_ctx->cache_dir, md5);
    if(file_cache == NULL){
        return 0;
    }
    cache_ctx->cache_file_ctx = file_cache;
    return 0;
}

void ff_io_cache_on_close(struct AVFormatContext *s, struct AVIOContext *pb){
    FFCacheContext * cache_ctx = (FFCacheContext*)s->opaque;
    ff_io_cache_uninstall_current(cache_ctx);
    s->pb = NULL;
}

int ff_io_cache_read_packet(void *opaque, unsigned char * buf, int buf_size){
    
    FFCacheContext * ctx = (FFCacheContext*)opaque;
    int64_t read_pos = ctx->read_pos;
    int ret = ff_io_cache_on_read_(ctx, buf, buf_size);
    if(ret > 0){
        ff_io_cache_write_cache(ctx, buf, ret, read_pos);
    }
    return ret;
err:
    return -1;
}

int64_t ff_io_cache_read_seek(void * opaque, int64_t offset, int whence){
    FFCacheContext * ctx = (FFCacheContext*)opaque;
    if(whence == AVSEEK_SIZE){//get size
        int64_t size = ff_io_cache_on_size_(ctx);
        LOGD("%s:return %lld\n", __FUNCTION__, size);
        return size;
    }else{
        return ff_io_cache_on_seek_(ctx, offset, whence);
    }
    return -1;
err:
    return -1;
}

int ff_io_cache_format_open_input(AVFormatContext ** src_ctx, const char * url, AVInputFormat * fmt, AVDictionary **options, int flags){
    if(src_ctx == NULL){
        return -1;
    }
    
    AVFormatContext * src = NULL;
    if(*src_ctx == NULL){
        src = avformat_alloc_context();
    }else{
        src = *src_ctx;
    }
    
    if(strlen(g_cache_dir) == 0){
        LOGE("%s:cache dir is null, did called ff_io_cache_global_set_path() ?", __FUNCTION__);
        return -1;
    }
    if(ff_io_cache_global_check() != 0){
        LOGE("%s:global check failed, please set cache path use ff_io_cache_gloabl_set_path()\n",__FUNCTION__);
        return -1;
    }
    
    FFCacheContext * cache_ctx = (FFCacheContext*)malloc(sizeof(FFCacheContext));
    memset(cache_ctx, 0, sizeof(FFCacheContext));
    
    if(flags & FFCACHE_FLAG_ENABLE_CACHE_READ){
        cache_ctx->is_enable_cache_read = 1;
    }
    if(flags & FFCACHE_FLAG_ENABLE_CACHE_WRITE){
        cache_ctx->is_enable_cache_write = 1;
    }
    char md5[36] = {0};
    int ret = 0;

    AVFormatContext * ctx = src;
    ctx->io_open = ff_io_cache_on_open;
    ctx->io_close = ff_io_cache_on_close;
    ctx->opaque = cache_ctx;
    
    ff_str_md5(url, md5);
    cache_ctx->curr_uid = malloc(strlen(md5) + 1);
    strcpy(cache_ctx->curr_uid, md5);
    cache_ctx->cache_dir = malloc(strlen(g_cache_dir) + strlen(md5) + 2);
    sprintf(cache_ctx->cache_dir, "%s/%s", g_cache_dir, md5);
    ret = access(cache_ctx->cache_dir, F_OK);
    if(ret != 0){
        ret = mkdir(cache_ctx->cache_dir, S_IRWXU);
    }
    if(ret != 0){
        LOGE("%s:Can`t access [%s]! \n", __FUNCTION__, cache_ctx->cache_dir);
        goto err;
    }
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ret = avformat_open_input(&ctx, url, fmt, options);
    if(ret < 0){
        LOGE("%s:avformat_open_input return negative\n", __FUNCTION__);
        goto err;
    }
    if(*src_ctx == NULL){
        *src_ctx = ctx;
    }
    return 0;
err:
    if(cache_ctx){
        if(cache_ctx->cache_file_ctx){
            fff_cache_free(&cache_ctx->cache_file_ctx);
        }
        if(cache_ctx->cache_dir){
            free(cache_ctx->cache_dir);
        }
    }
    return -1;
}

int ff_io_cache_format_free_context(AVFormatContext * src_ctx){
    if(src_ctx == NULL){
        return -1;
    }
    FFCacheContext * ctx = (FFCacheContext*)src_ctx->opaque;
    avformat_free_context(src_ctx);
    if(ctx == NULL){
        return 0;
    }
    ctx->agent_ic = NULL;
    if(ctx->back_src_ic){
        avio_closep(&ctx->back_src_ic);
    }
    if(ctx->cache_file_ctx){
        fff_cache_free(&ctx->cache_file_ctx);
    }
    if(ctx->cache_dir){
        fff_freep((void**)&ctx->cache_dir);
    }
    if(ctx->curr_src_url){
        fff_freep((void**)&ctx->curr_src_url);
    }
    if(ctx->io_buffer){
        av_freep((void**)&ctx->io_buffer);
    }
    if(ctx->curr_uid){
        fff_freep((void**)&ctx->curr_uid);
    }
    free(ctx);
    ctx = NULL;
    return 0;
}

int ff_io_cache_format_close_input(AVFormatContext ** src_ctx){
    if(src_ctx == NULL || *src_ctx){
        return -1;
    }
    ff_io_cache_format_free_context(*src_ctx);
    *src_ctx = NULL;
    return 0;
}

int64_t ff_io_cache_on_size_(FFCacheContext * ctx){
    int64_t size = AVERROR_UNKNOWN;
    if(ctx->is_enable_cache_read){
        size = fff_cache_get_size(ctx->cache_file_ctx);
    }
    if(size < 0){
        size = avio_size(ctx->back_src_ic);
    }
    return size;
}

int ff_io_cache_on_read_(FFCacheContext * ctx, unsigned char * buf, int buf_size){
    int ret = AVERROR_UNKNOWN;
    //int64_t debug_pos = ctx->read_pos;
    ret = ff_io_cache_read_cache(ctx, buf, buf_size);
    if(ret == AVERROR_EOF){
        return AVERROR_EOF;
    }
    if(ret > 0){
        return ret;
    }
    if(ff_io_cache_init_src_io(ctx) < 0){
        goto err;
    }
    ret = ff_io_cache_read_src(ctx, buf, buf_size);
    if(ret == AVERROR_EOF){
        LOGI("%s:end of source\n", __FUNCTION__);
        return ret;
    }else if(ret < 0){
        LOGE("%s:read source data failed\n", __FUNCTION__);
        goto err;
    }
    return ret;
err:
    return ret;
}

int64_t ff_io_cache_on_seek_(FFCacheContext * ctx, int64_t offset, int whence){
    int64_t ret = AVERROR_UNKNOWN;
    if(whence == AVSEEK_FORCE){
        ctx->read_pos = offset;
        if(ctx->is_enable_cache_read || ctx->is_enable_cache_read){
            //是否命中缓存
            if(fff_cache_read_pos_hit(ctx->cache_file_ctx, offset, offset) > 0){
                ret = fff_cache_read_seek(ctx->cache_file_ctx, offset, SEEK_SET);
                if(ret >= 0){
                    ctx->read_pos = ret;
                    return ret;
                }
            }
        }
        
        if(ff_io_cache_init_src_io(ctx) < 0){
            goto err;
        }
        ret = avio_seek(ctx->back_src_ic, offset, whence);
        if(ret >= 0){
            ctx->read_pos = offset;
            return ret;
        }
    }else if(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END){
        if(ctx->is_enable_cache_read || ctx->is_enable_cache_read){
            ret = fff_cache_read_seek(ctx->cache_file_ctx, offset, whence);
        }
        if(ret >= 0){
            ctx->read_pos = ret;
            return ret;
        }else{
            ret = avio_seek(ctx->back_src_ic, offset, whence);
            ctx->read_pos = ret;
            return ret;
        }
    }
    return AVERROR_UNKNOWN;
err:
    return ret;
}

int ff_io_cache_read_src(FFCacheContext * ctx, unsigned char * buf, int read_size){
    int64_t ret64 = 0;
    int ret = 0;
    if(ff_io_cache_init_src_io(ctx) != 0){
        LOGE("%s:init back source failed\n", __FUNCTION__);
        goto err;
    }
    if(ctx->read_pos != avio_tell(ctx->back_src_ic)){
        ret64 = avio_seek(ctx->back_src_ic, ctx->read_pos, SEEK_SET);
        if(ret64 < 0){
            LOGE("%s:avio_seek failed\n", __FUNCTION__);
            goto err;
        }
    }
    ret =avio_read(ctx->back_src_ic, buf, read_size);
    if(ret > 0){
        ctx->read_pos += ret;
    }
    return ret;
    
err:
    return -1;
}

int ff_io_cache_read_cache(FFCacheContext * ctx, unsigned char * buf, int read_size){
    int ret = 0;
    if(ctx->read_pos != fff_cache_read_pos(ctx->cache_file_ctx)){
        int64_t ret64 = fff_cache_read_seek(ctx->cache_file_ctx, ctx->read_pos, SEEK_SET);
        if(ret64 < 0){
            goto err;
        }
    }
    if(fff_cache_read_pos_hit(ctx->cache_file_ctx, ctx->read_pos, ctx->read_pos + read_size - 1) <= 0){
        if(fff_cache_read_eof(ctx->cache_file_ctx)){
            return AVERROR_EOF;
        }
        goto err;
    }
    ret =fff_cache_read2(ctx->cache_file_ctx, buf, read_size);
    if(ret > 0){
        ctx->read_pos += ret;
    }else{
        
    }
    return ret;
err:
    return -1;
}

int ff_io_cache_write_cache(FFCacheContext * ctx, unsigned char * data, int data_len, int64_t begin_pos){
    if(ctx->cache_file_ctx->cache_full == 0 && ctx->is_enable_cache_write){
        if(fff_cache_write_pos(ctx->cache_file_ctx) != begin_pos){
            fff_cache_write_seek(ctx->cache_file_ctx, begin_pos);
        }
        return fff_cache_write(ctx->cache_file_ctx, data, data_len);
    }
    return 0;
}

int64_t ff_io_cache_write_seek(FFCacheContext * ctx, int64_t seek_pos){
    return fff_cache_write_seek(ctx->cache_file_ctx, seek_pos);
}

int ff_io_cache_uninstall_current(FFCacheContext * ctx){
    if(ctx->cache_file_ctx){
        fff_cache_free(&ctx->cache_file_ctx);
    }
    if(ctx->agent_ic){
        avio_context_free(&ctx->agent_ic);
    }
    if(ctx->back_src_ic){
        avio_context_free(&ctx->back_src_ic);
    }
    
    if(ctx->curr_src_url){
        free(ctx->curr_src_url);
        ctx->curr_src_url = NULL;
    }
    if(ctx->curr_uid){
        free(ctx->curr_uid);
        ctx->curr_uid = NULL;
    }
    ctx->read_pos = 0;
    return 0;
}
