//
//  ff_io_cache.h
//  ffmpeg_demo
//
//  Created by helei on 2018/8/9.
//  Copyright © 2018年 何磊. All rights reserved.
//

#ifndef ff_io_cache_h
#define ff_io_cache_h

#include <stdio.h>
#include "ff_header.h"
#include "ff_file_cache.h"

typedef struct FFCacheContext{
    
    int is_enable_cache_read;
    int is_enable_cache_write;
    
    char * cache_dir;
    unsigned char * io_buffer;
    int io_buffer_size;
    char * curr_src_url;
    char * curr_uid;
    
    int64_t read_pos;
    
    AVIOContext * back_src_ic;//回源ic,缓存模块用这个源来读取

    AVIOContext * agent_ic;//代理ic, 用这个ic替掉上层AVFormatContext的ic

    FFFileCacheContext * cache_file_ctx;//本地文件缓存
}FFCacheContext;

int ff_io_cache_global_check(void);

/**
 设置缓存主目录
 
 @param cache_dir 缓存目录不能为NULL
 @return 成功返货0, 否则返回小于0的值
 */
int ff_io_cache_global_set_path(const char * cache_dir);
int ff_io_cache_global_del_all(void);
int ff_io_cache_global_del(const char * url);
int ff_io_cache_global_export(const char * url, const char * path, const char * fmt_name);

//普通优先级
#define FFCACHE_FLAG_ENABLE_CACHE_READ      0x10   //允许缓存
#define FFCACHE_FLAG_ENABLE_CACHE_WRITE     0x20   //允许缓存

//默认的flag, 启用缓存(自动),允许回源
#define FFCACHE_FLAG_DEFAULT    (FFCACHE_FLAG_ENABLE_CACHE_READ | FFCACHE_FLAG_ENABLE_CACHE_WRITE)

int ff_io_cache_format_open_input(AVFormatContext ** src_ctx, const char * url, AVInputFormat * fmt, AVDictionary **options, int flags);
int ff_io_cache_format_free_context(AVFormatContext * src_ctx);
int ff_io_cache_format_close_input(AVFormatContext ** src_ctx);

//=====================================//

int64_t ff_io_cache_on_size_(FFCacheContext * ctx);
int ff_io_cache_on_read_(FFCacheContext * ctx, unsigned char * buf, int buf_size);
int64_t ff_io_cache_on_seek_(FFCacheContext * ctx, int64_t offset, int whence);

int ff_io_cache_init_src_io(FFCacheContext * ctx);
int ff_io_cache_db_read(FFCacheContext * ctx, const char * db_file);
int ff_io_cache_db_write(FFCacheContext * ctx, const char * db_file);

int ff_io_cache_read_src(FFCacheContext * ctx, unsigned char * buf, int read_size);
int ff_io_cache_read_cache(FFCacheContext * ctx, unsigned char * buf, int read_size);

int ff_io_cache_read_packet(void *opaque, unsigned char * buf, int buf_size);
int64_t ff_io_cache_read_seek(void * opaque, int64_t offset, int whence);

int ff_io_cache_write_cache(FFCacheContext * ctx, unsigned char * buf, int read_size, int64_t begin_pos);
int64_t ff_io_cache_write_seek(FFCacheContext * ctx, int64_t seek_pos);
int ff_io_cache_reset_current(FFCacheContext * ctx);
#endif /* ff_io_cache_h */
