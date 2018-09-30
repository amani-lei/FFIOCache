//
//  ff_log.c
//  ffmpeg_demo
//
//  Created by helei on 2018/3/30.
//  Copyright © 2018年 何磊 <helei0908@hotmail.com>. All rights reserved.
//
#ifndef FF_LOG_H
#define FF_LOG_H

#include <stdio.h>
#define LOG_TAG "JNI_ffmedia"

#ifdef ANDROID

#include "android/log.h"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGI(...)   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#define  LOGD(...)  do{printf("[DEBUG] ");printf(__VA_ARGS__);}while(0)
#define  LOGI(...)  do{printf("[INFO] ");printf(__VA_ARGS__);}while(0)
#define  LOGW(...)  do{printf("[WARW] ");printf(__VA_ARGS__);}while(0)
#define  LOGE(...)  do{printf("[ERROR] ");printf(__VA_ARGS__);}while(0)

#endif

#endif
