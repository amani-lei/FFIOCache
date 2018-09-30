//
//  ff_metadata.h
//  IJKMediaFramework
//
//  Created by helei on 2018/6/8.
//  Copyright © 2018年 bilibili. All rights reserved.
//

#ifndef ff_metadata_h
#define ff_metadata_h

#include <stdio.h>
#include "stdlib.h"
typedef enum FFMetadataType{
    FF_CACHE_METADATA_STRING,
    FF_CACHE_METADATA_INT,
}FFMetadataType;

typedef struct FFMetadata{
    //    FFMetadataType type;
    char * name;
    char * s_value;
}FFMetadata;

FFMetadata* ff_metadata_alloc_string_node(const char * name, const char * value);
int ff_metadata_node_free(FFMetadata ** metadata);


#define FF_METADATA_MAX_SIZE    16
typedef struct FFMetadataContext{
    FFMetadata *g_metadata[FF_METADATA_MAX_SIZE];//全局meta
    FFMetadata *v_metadata[FF_METADATA_MAX_SIZE];//视频流meta
    FFMetadata *a_metadata[FF_METADATA_MAX_SIZE];//音频meta
}FFMetadataContext;

FFMetadataContext * ff_metadata_context_alloc();
void ff_metadata_context_free(FFMetadataContext * ctx);


/**
 添加一个metadata

 @param ctx metadata上下文
 @param meta metadata结构体,不能为null
 @param type 一个字符,代表类型, g=全局, v=视频, a=音频 s=字母(暂不支持)
 @return 成功返回0, 否则返回小于0的值
 */
int ff_metadata_add(FFMetadataContext * ctx, FFMetadata * meta, char type);
int ff_metadata_add_g(FFMetadataContext * ctx, FFMetadata * meta);
int ff_metadata_add_v(FFMetadataContext * ctx, FFMetadata * meta);
int ff_metadata_add_a(FFMetadataContext * ctx, FFMetadata * meta);

/**
 删除一个metadata
 
 @param ctx metadata上下文
 @param key metadata的名字,不能为null
 @param type 一个字符,代表类型, g=全局, v=视频, a=音频 s=字母(暂不支持)
 @return 成功返回0, 否则返回小于0的值
 */
int ff_metadata_del(FFMetadataContext * ctx, const char * key, char type);
int ff_metadata_del_g(FFMetadataContext * ctx, const char * key);
int ff_metadata_del_v(FFMetadataContext * ctx, const char * key);
int ff_metadata_del_a(FFMetadataContext * ctx, const char * key);

/**
 获取一个Meta

 @param ctx metadata上下文
 @param key 一个关键字, 可以为null(表示获取第一个)
 @param type 'g v a'其中一个字符, 代表全局,视频,音频
 @return 成功返回meta地址, 否则返回null
 */
FFMetadata * ff_metadata_get(FFMetadataContext * ctx, const char * key, char type);
FFMetadata * ff_metadata_get_g(FFMetadataContext * ctx, const char * key);
FFMetadata * ff_metadata_get_v(FFMetadataContext * ctx, const char * key);
FFMetadata * ff_metadata_get_a(FFMetadataContext * ctx, const char * key);

/**
 获取给定meta的下一个meta
 只查找与prev同类型的

 @param ctx meta上下文
 @param prev 一个meta地址, 一般是调用ff_metadata_get的返回值, 不能为null
 @return 成功返回meta地址, 否则返回null
 */
FFMetadata * ff_metadata_get_next(FFMetadataContext * ctx, FFMetadata * prev);


/**
 解析metadata字符串
 e.g:  g"auth=null"v"rotate=90,hflip=true"  (字符"也是字符串的一部分, 根据实际情况进行转义)
 @param str_meta 上面所示的metadata字符串
 @return 成功返回FFMetadataContext指针, 否则返回NULL
 */
FFMetadataContext * ff_metadata_parse(const char * str_meta);

/**
 获取metadata中的的一组的开始结束位置和类型
 e.g:  g"auth=null"v"rotate=90,hflip=true"  (字符"也是字符串的一部分, 根据实际情况进行转义)
 "auth=null"为第一组
 "rotate=90,hflip=true"为第二组
 函数执行成功会返回每组前面的g,v等标记
 
 这个函数只会获取第一组的开始和结束位置
 
 @param str_meta metadata字符串的开始地址,这个字符串以0结尾, 函数会将这个指针置为下一组的开始位置
                内部会修改str_meta内的数据, 如果不希望更改原始数据, 请传入一个原始数据的拷贝
 @param type_meta 返回切分后的一组字符串地址
 @return 成功返回 g v a s 四个字符中的一种, 失败返回0
 */
char ff_metadata_parse_type_cut(char ** str_meta, char **type_meta);

int ff_metadata_parse_type(FFMetadataContext *ctx, char * str_meta, char type);

/**
 获取一组中键值对的开始和结束位置
 
 @param str_meta 开始地址
 @param pair 返回一对
 @return 成功返回1, 无法获取返回0, 出现错误返回小于0的值
 */
int ff_metadata_parse_pair_cut(char ** str_meta, char **pair);


FFMetadata * ff_metadata_parse_pair(char * pair);


#endif /* ff_metedata_h */
