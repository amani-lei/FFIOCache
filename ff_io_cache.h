//
//  ff_io_cache.h
//  ffmpeg_demo
//
//  Created by helei on 2018/8/9.
//  Copyright © 2018年 何磊 <helei0908@hotmail.com>. All rights reserved.
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


/**
 删除所有缓存

 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_global_del_all(void);

/**
 删除一个url的缓存

 @param url url地址
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_global_del(const char * url);


#define FFCACHE_EXPORT_FORMAT_MP4       "mp4"
#define FFCACHE_EXPORT_FORMAT_MPEGTS    "mpegts"
#define FFCACHE_EXPORT_FORMAT_HLS       "hls"

/**
 导出一个缓存

 @param url url地址
 @param path 到处路径,包含文件名
 @param fmt_name 导出格式 参考:FFCACHE_EXPORT_FORMAT_XXX
 @return 成功返回0 否则返回小于0的值
 */
int ff_io_cache_global_export(const char * url, const char * path, const char * fmt_name);

//普通优先级
#define FFCACHE_FLAG_ENABLE_CACHE_READ      0x10   //允许缓存
#define FFCACHE_FLAG_ENABLE_CACHE_WRITE     0x20   //允许缓存

//默认的flag, 启用缓存(自动),允许回源
#define FFCACHE_FLAG_DEFAULT    (FFCACHE_FLAG_ENABLE_CACHE_READ | FFCACHE_FLAG_ENABLE_CACHE_WRITE)


/**
 用于替换avformat_open_input

 @param src_ctx AVFormatContext指针的地址, *src_ctx 可以为NULL
                参考avformat_open_input
 @param url url地址
 @param fmt 参考avformat_open_input
 @param options 参考avformat_open_input
 @param flags 参考FFCACHE_FLAG_XXXX
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_format_open_input(AVFormatContext ** src_ctx, const char * url, AVInputFormat * fmt, AVDictionary **options, int flags);


/**
 用于替换avformat_free_context

 @param src_ctx AVFormatContext的地址, 参考:avformat_free_context
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_format_free_context(AVFormatContext * src_ctx);

/**
 用于替换avformat_close_input
 
 @param src_ctx AVFormatContext指针的地址 参考:avformat_close_input
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_format_close_input(AVFormatContext ** src_ctx);

//==================以下为内部接口===================//


/**
 AVIOContext的回调, 返回文件大小,优先通过缓存获取文件大小, 如果获取失败,再通过源来获取

 @param ctx 缓存上下文
 @return 成功返回0, 否则返回小于0的值
 */
int64_t ff_io_cache_on_size_(FFCacheContext * ctx);


/**
 AVIOContext的回调, 用于读取数据

 @param ctx 缓存上下文
 @param buf 用来保存读取数据的buf
 @param buf_size buf的大小
 @return 成功返回读取到的数据长度, 否则返回小于0的值
 */
int ff_io_cache_on_read_(FFCacheContext * ctx, unsigned char * buf, int buf_size);


/**
 AVIOContext的回调, 用于seek操作

 @param ctx 缓存上下文
 @param offset 跳转点
 @param whence 如何跳转
 @return 成功返回跳转到的索引地址, 相对于文件开始
 */
int64_t ff_io_cache_on_seek_(FFCacheContext * ctx, int64_t offset, int whence);

/**
 初始化源

 @param ctx 缓存上下文
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_init_src_io(FFCacheContext * ctx);

/**
 从源读取数据

 @param ctx 缓存上下文
 @param buf 要用来保存读取到的数据的缓冲区, 缓冲区大小不能小于read_size
 @param read_size 读取大小
 @return 成功返回读取到的数据长度, 否则返回小于等于0的值
 */
int ff_io_cache_read_src(FFCacheContext * ctx, unsigned char * buf, int read_size);

/**
 从c缓存读取数据
 
 @param ctx 缓存上下文
 @param buf 要用来保存读取到的数据的缓冲区, 缓冲区大小不能小于read_size
 @param read_size 读取大小
 @return 成功返回读取到的数据长度, 否则返回小于等于0的值
 */
int ff_io_cache_read_cache(FFCacheContext * ctx, unsigned char * buf, int read_size);


/**
 AVIOContext的回调, 用于读取数据
    参考:avio_alloc_context
 
 @param opaque 回调传递的FFCacheContext指针
 @param buf 用来保存读取数据的buf
 @param buf_size  buf的大小
 @return 成功返回读取到的数据长度, 否则返回小于0的值
 */
int ff_io_cache_read_packet(void *opaque, unsigned char * buf, int buf_size);


/**
 AVIOContext的回调, 用于seek和size操作
    参考:avio_alloc_context
 
 @param opaque 回调传递的FFCacheContext指针
 @param offset seek点
 @param whence seek方式
 @return 参考avio_alloc_context
 */
int64_t ff_io_cache_read_seek(void * opaque, int64_t offset, int whence);


/**
 写入数据到缓存

 @param ctx 缓存上下文
 @param data 数据缓冲区
 @param data_size 数据长度
 @param begin_pos 文件写指针位置, 用来描述从文件何处写入
 @return 成功返回写入数据的长度, 否则返回小于等于0的值
 */
int ff_io_cache_write_cache(FFCacheContext * ctx, unsigned char * data, int data_size, int64_t begin_pos);


/**
 移动缓存写指针

 @param ctx 缓存上下文
 @param seek_pos 文件写指针位置, 相对于缓存开始
 @return 成功返回新的写指针位置, 否则返回小于0的值
 */
int64_t ff_io_cache_write_seek(FFCacheContext * ctx, int64_t seek_pos);


/**
 卸载当前的缓存任务

 @param ctx 缓存上下文
 @return 成功返回0, 否则返回小于0的值
 */
int ff_io_cache_uninstall_current(FFCacheContext * ctx);
#endif /* ff_io_cache_h */
