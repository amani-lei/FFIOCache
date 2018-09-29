//
//  ff_file_cache.h
//  ffmpeg_demo
//
//  Created by helei on 2018/8/27.
//  Copyright © 2018年 何磊. All rights reserved.
//

#ifndef ff_file_cache_h
#define ff_file_cache_h

#include <stdio.h>
#include "ff_fragment_list.h"
#define FF_FILE_CACHE_FRAGMENT_SIZE         (100*1024)      //bytes
#define FF_FILE_CACHE_MAX_FRAGMENT_COUNT   (1024)

typedef struct FFFileCacheContext{
    int updated;
    int16_t version;
    char * dir;
    char * file_name;
    char * full_name;
    char * db_file_full_name;
    int64_t file_size;//-1 or size
    uint8_t cache_full;
    FFFragmentList * fragment_list;
    
    FILE * inf;
    int64_t read_pos;
    
    FILE * outf;
    int64_t write_pos;
}FFFileCacheContext;


/**
 创建一个文件缓存

 @param dir 缓存目录
 @param file_name 缓存文件名
 @return 成功返回FFFileCacheContext地址, 否则返回NULL
 */
FFFileCacheContext * fff_cache_alloc(const char * dir, const char * file_name);

/**
 释放一个缓存对象

 @param pp 缓存上下文指针的地址, 会将指针置为NULL
 */
void fff_cache_free(FFFileCacheContext ** pp);
int fff_cache_read_db_data(FFFileCacheContext * ctx, unsigned char ** out_data);
int fff_cache_write_db_data(FFFileCacheContext * ctx, unsigned char * data, int data_len);
int fff_cache_is_complete(FFFileCacheContext * ctx);
int fff_cache_set_size(FFFileCacheContext * ctx, int64_t size);
int64_t fff_cache_get_size(FFFileCacheContext * ctx);
int fff_cache_serialization(FFFileCacheContext * ctx, unsigned char ** out_buf);
int fff_cache_deserialization(FFFileCacheContext * ctx, unsigned char * data, int data_len);

void fff_free(void *p);
void fff_freep(void **p);

/**
 初始化缓存读取,主要是用来打开文件读取句柄

 @param ctx 缓存上下文
 @return 成功返回0, 否则返回小于0的值
 */
int fff_cache_init_read(FFFileCacheContext * ctx);

/**
 初始化缓存写入,主要是用来打开文件写入句柄
 
 @param ctx 缓存上下文
 @return 成功返回0, 否则返回小于0的值
 */
int fff_cache_init_write(FFFileCacheContext * ctx);

/**
 读取一段数据

 @param ctx 缓存上下文
 @param out_data 用来接收数据缓冲区的地址, 不能为NULL, 不再使用数据缓冲区时, 调用fff_free / fff_freep来释放
 @param read_size 要读取的字节数
 @return 成功返回读取的字节数, 如果数据并未缓存成功, 可能与read_size不相等, 可能为0
        其他情况返回小于0的值
 */
int fff_cache_read(FFFileCacheContext * ctx, unsigned char ** out_data, int read_size);


/**
 读取一段数据
 与fff_cache_read类似, 不同的是fff_cache_read2数据缓冲区由调用方提供
 @param ctx 缓存上下文
 @param out_buf 用来接收数据的缓冲区, 不能为NULL, 不再使用数据缓冲区时, 调用fff_free / fff_freep来释放
 @param read_size 要读取的字节数
 @return 成功返回读取的字节数, 如果数据并未缓存成功, 可能与read_size不相等, 可能为0
 其他情况返回小于0的值
 */
int fff_cache_read2(FFFileCacheContext * ctx, unsigned char * out_buf, int read_size);


/**
 移动读取指针,这个函数不会关心数据的有效性, 即无论数据是否已经缓存成功, 都会执行seek

 @param ctx 缓存上下文
 @param seek_pos 要移动到的读取指针位置
 @param whence SEEH_SET/SEEK_CUR/SEEK_END
 @return 成功返回0, 否则返回小于0的值
 */
int64_t fff_cache_read_seek(FFFileCacheContext * ctx, int64_t seek_pos, int whence);


/**
 获取读取指针位置

 @param ctx 缓存上下文
 @return 成功返回读取指针位置, 否则返回小于0的值
 */
int64_t fff_cache_read_pos(FFFileCacheContext * ctx);

/**
 写入缓存数据

 @param ctx 缓存上下文
 @param data 数据地址
 @param data_size 数据字节数
 @return 成功返回写入成功的数据字节数, 失败返回小于0的值
 */
int fff_cache_write(FFFileCacheContext * ctx, unsigned char * data, int data_size);

/**
 返回写入指针的位置

 @param ctx 缓存上下文
 @return 成功返回大于等于0的值, 否则返回小于0的值
 */
int64_t fff_cache_write_pos(FFFileCacheContext * ctx);
/**
 移动写入指针
 
 @param ctx 缓存上下文
 @param seek_pos 要移动到的写入指针位置
 @return 成功返回0, 否则返回小于0的值
 */
int64_t fff_cache_write_seek(FFFileCacheContext * ctx, int64_t seek_pos);



/**
 命中测试, 测试缓存的一段的数据,是否有效(是否已经缓存)

 @param ctx 缓存上下文
 @param begin_pos 开始位置(包含)
 @param end_pos 结束位置(包含)
 @return 命中返回大于0的值, 未命中返回0, 其他情况返回小于0的值
 */
int fff_cache_read_pos_hit(FFFileCacheContext * ctx, int64_t begin_pos, int64_t end_pos);

int fff_cache_read_eof(FFFileCacheContext * ctx);
#endif /* ff_file_cache_h */
