//
//  ff_remuxer.h
//  IJKMediaFramework
//
//  Created by helei on 2017/12/21.
//  Copyright © 2017年. All rights reserved.
//

#ifndef ff_remuxer_h
#define ff_remuxer_h

#include <stdlib.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "inttypes.h"
#include "stdint.h"
/**
 返回值,错误码
 */
typedef enum RemuxerError {
    REMUXER_ERROR_NONE = 0,             //执行成功
    REMUXER_ERROR_PARAM = -1,          //参数错误
    REMUXER_ERROR_UNINITED = -2,        //未初始化
    REMUXER_ERROR_IO = -3,              //IO错误
    REMUXER_ERROR_UNSUPPORT_FORMAT = -4,//不支持的格式
    REMUXER_ERROR_MEMORY = -5,          //内存错误
    
    //新增错误后,记得更新remuxer_get_error_desc函数
    REMUXER_ERROR_UNKNOW = (-0x7FFFFFFF)                //未知的/未命名的错误
}RemuxerError;


/**
 再复用器上下文
 */
typedef struct RemuxerContext{
    int is_alloc2;
    int ready;                      //是否就绪的标志,当remuxer_context_alloc执行成功后, 会置为1,如果ready不为0
                                    //remuxer_do会执行失败
    int error_code;                 //一个错误码,如果某一步执行错误会设置error_code
                                    //error_code始终保存了最后一次的错误信息
    AVFormatContext * in_fmt_ctx;   //输入上下文
    AVFormatContext * out_fmt_ctx;  //输出上下文
    int stream_map[64];
}RemuxerContext;

/**
 #####----外部不需要使用这个接口----#####
 为一个再复用器上下文,申请内存,返回上下文对象地址

 @return 成功返回创建的上下文地址,失败返回null
 */
RemuxerContext * remuxer_context_alloc(void);

/**
 传入一个源文件路径和目标文件路径,
 内部会做一些初步的分析和判断(在需要的时候,内部会创建对应的文件夹)
 创建一个复用器上下文,
 由mp4转hls
 由hls转mp4
 等其他封装的转换
 目前没有转码操作
 
 @param inFile  源文件的完整路径, 支持网络地址,
                例如
                C:\\demo.mp4
                C:/demo.mp4
                /home/demo.mp4
                http://ffmpeg.org/test.m3u8
 @param outFile 输出文件的完整路径(包含后缀)
 @return        成功返回创建的上下文地址,失败返回null
 */
RemuxerContext* remuxer_alloc(const char * inFile, const char * outFile);
RemuxerContext* remuxer_alloc2(AVFormatContext * in_fmt, const char * outFile);
/**
 执行再复用过程
 这是一个阻塞的函数,执行完毕之前不会返回

 @param ctx 调用remuxer_alloc时返回的再复用器地址
 @return 成功返回0, 否则返回一个小于0的值
 */
int remuxer_do(RemuxerContext * ctx);
/**
 再复用器上下文对象的释放接口,当不需要这个再复用对象时,
 都应该调用这个接口来释放

 @param pctx remuxer_alloc/remuxer_context_alloc/remuxer_context_alloc2的返回指针地址
 */
void remuxer_delete(RemuxerContext ** pctx);

int remuxer_copy_metadata(AVDictionary ** dst, AVDictionary * src);

/**
 如果其他接口执行失败,调用此接口获取错误描述,
 如果其他接口执行成功,调用这个接口会返回之前执行失败/成功的结果

 @param ctx 再复用器上下文地址
 @return 返回可读的文字描述信息
 */
const char * remuxer_get_error_desc(RemuxerContext * ctx);

/**
 与remuxer_get_error_desc类似,只不过这个接口返回一个错误码

 @param ctx 再复用器上下文地址
 @return 错误码,参考枚举类型RemuxerError
 */
int remuxer_get_error_code(RemuxerContext * ctx);
#endif /* ff_remuxer_h */
