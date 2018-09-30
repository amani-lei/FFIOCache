//
//  ff_metadata.c
//  IJKMediaFramework
//
//  Created by helei on 2018/6/8.
//  Copyright © 2018年 bilibili. All rights reserved.
//

#include "ff_metadata.h"
#include "string.h"
#include "memory.h"

FFMetadata* ff_metadata_alloc_string_node(const char * name, const char * value){
    FFMetadata * p = (FFMetadata *)malloc(sizeof(FFMetadata));
    memset(p, 0, sizeof(FFMetadata));
    //p->type = FF_CACHE_METADATA_STRING;
    size_t name_len = strlen(name) + 1;
    p->name = malloc(name_len);
    strcpy(p->name, name);
    size_t val_len = strlen(value) + 1;
    p->s_value = malloc(val_len);
    strcpy(p->s_value, value);
    return p;
}

int ff_metadata_node_free(FFMetadata ** metadata){
    if(metadata == NULL || *metadata == NULL){
        return -1;
    }
    if((*metadata)->s_value){
        free((*metadata)->s_value);
    }
    if((*metadata)->name){
        free((*metadata)->name);
    }
    free(*metadata);
    *metadata = NULL;
    return 0;
}

FFMetadataContext * ff_metadata_context_alloc(){
    FFMetadataContext * p = (FFMetadataContext *)malloc(sizeof(FFMetadataContext));
    memset(p, 0, sizeof(FFMetadataContext));
    return p;
}
void ff_metadata_context_free(FFMetadataContext * ctx){
    if(ctx == NULL)return;
    int i = 0;
    for(;i < FF_METADATA_MAX_SIZE; i++){
        if(ctx->g_metadata[i]){
            ff_metadata_node_free(&ctx->g_metadata[i]);
        }
        if(ctx->v_metadata[i]){
            ff_metadata_node_free(&ctx->v_metadata[i]);
        }
        if(ctx->a_metadata[i]){
            ff_metadata_node_free(&ctx->a_metadata[i]);
        }
    }
}

int ff_metadata_add(FFMetadataContext * ctx, FFMetadata * meta, char type){
    if(ctx == NULL || meta == NULL){
        return -1;
    }
    FFMetadata ** meta_begin = NULL;
    switch(type){
        case 'g':
            meta_begin = ctx->g_metadata;
            break;
        case 'v':
            meta_begin = ctx->v_metadata;
            break;
        case 'a':
            meta_begin = ctx->a_metadata;
            break;
        default:
            return -1;
    }
    //查找空闲的位置进行保存
    int i = 0;
    for(i = 0; i < FF_METADATA_MAX_SIZE; i++){
        if(meta_begin[i] == NULL){
            meta_begin[i] = meta;
            return 0;
        }
    }
    return -1;
    
}
int ff_metadata_add_g(FFMetadataContext * ctx, FFMetadata * meta){
    return ff_metadata_add(ctx, meta, 'g');
}
int ff_metadata_add_v(FFMetadataContext * ctx, FFMetadata * meta){
    return ff_metadata_add(ctx, meta, 'v');
}
int ff_metadata_add_a(FFMetadataContext * ctx, FFMetadata * meta){
    return ff_metadata_add(ctx, meta, 'a');
}
int ff_metadata_del(FFMetadataContext * ctx, const char * key, char type){
    if(ctx == NULL || key == NULL){
        return -1;
    }
    FFMetadata ** meta_begin = NULL;
    switch(type){
        case 'g':
            meta_begin = ctx->g_metadata;
            break;
        case 'v':
            meta_begin = ctx->v_metadata;
            break;
        case 'a':
            meta_begin = ctx->a_metadata;
            break;
        default:
            return -1;
    }
    //查找空闲的位置进行保存
    int i = 0;
    FFMetadata * metadata = NULL;
    for(i = 0; i < FF_METADATA_MAX_SIZE; i++){
        metadata = meta_begin[i];
        if(metadata && strcmp(metadata->name, key) == 0){
            ff_metadata_node_free(&meta_begin[i]);
            return 0;
        }
    }
    return -1;
}
int ff_metadata_del_g(FFMetadataContext * ctx, const char * key){
    return ff_metadata_del(ctx, key, 'g');
}
int ff_metadata_del_v(FFMetadataContext * ctx, const char * key){
    return ff_metadata_del(ctx, key, 'v');
}
int ff_metadata_del_a(FFMetadataContext * ctx, const char * key){
    return ff_metadata_del(ctx, key, 'a');
}

FFMetadata * ff_metadata_get(FFMetadataContext * ctx, const char * key, char type){
    if(ctx == NULL){
        return NULL;
    }
    FFMetadata ** meta_begin = NULL;
    switch(type){
        case 'g':
            meta_begin = ctx->g_metadata;
            break;
        case 'v':
            meta_begin = ctx->v_metadata;
            break;
        case 'a':
            meta_begin = ctx->a_metadata;
            break;
        default:
            return NULL;
    }
    int i = 0;
    FFMetadata * meta = NULL;
    for(i = 0; i < FF_METADATA_MAX_SIZE; i++){
        meta = meta_begin[i];
        if(meta){
            if(key == NULL){
                return meta;
            }
            else if(strcmp(meta->name, key) == 0){
                return meta;
            }
        }
    }
    return NULL;
}
FFMetadata * ff_metadata_get_g(FFMetadataContext * ctx, const char * key){
    return ff_metadata_get(ctx, key, 'g');
}
FFMetadata * ff_metadata_get_v(FFMetadataContext * ctx, const char * key){
    return ff_metadata_get(ctx, key, 'v');
}
FFMetadata * ff_metadata_get_a(FFMetadataContext * ctx, const char * key){
    return ff_metadata_get(ctx, key, 'a');
}

FFMetadata * ff_metadata_get_next(FFMetadataContext * ctx, FFMetadata * prev){
    if(ctx == NULL || prev == NULL){
        return NULL;
    }
    int i = 0;
    int begin_index = 0;
    FFMetadata ** meta_begin = NULL;
    for(i = 0; i < FF_METADATA_MAX_SIZE; i++){
        //查找所在组(全局,视频,音频)
        if(prev == ctx->g_metadata[i]){
            meta_begin = ctx->g_metadata;
            begin_index = i;
            break;
        }
        else if(prev == ctx->v_metadata[i]){
            meta_begin = ctx->v_metadata;
            begin_index = i;
            break;
        }
        else if(prev == ctx->a_metadata[i]){
            meta_begin = ctx->a_metadata;
            begin_index = i;
            break;
        }
    }
    
    //如果找到
    if(meta_begin){
        begin_index ++;
        for(i = begin_index; i< FF_METADATA_MAX_SIZE; i++){
            if(meta_begin[i]){
                return meta_begin[i];
            }
        }
    }
    return NULL;
}

FFMetadataContext * ff_metadata_parse(const char * str_meta){
    if(str_meta ==NULL){
        return NULL;
    }
    FFMetadataContext * ctx = ff_metadata_context_alloc();
    int meta_len = (int)strlen(str_meta);
    char * _str_meta = (char*)malloc(meta_len + 1);
    char * meta = _str_meta;
    memcpy(_str_meta, str_meta, meta_len + 1);

    char type = 0;
    char *type_meta = 0;
    int ret = -1;
    while(1){
        //按类型拆分成多组
        type = ff_metadata_parse_type_cut(&meta, &type_meta);
        if(type == 0){
            break;
        }else if(type < 0){
            goto err;
        }
        
        //解析一组
        ret = ff_metadata_parse_type(ctx, type_meta, type);
        if(ret < 0){
            goto err;
        }
    }
    if(_str_meta){
        free(_str_meta);
        _str_meta = NULL;
    }
    return ctx;
err:
    if(ctx){
        ff_metadata_context_free(ctx);
        ctx = NULL;
    }
    if(_str_meta){
        free(_str_meta);
        _str_meta = NULL;
    }
    return NULL;
}

char ff_metadata_parse_type_cut(char ** str_meta, char **type_meta){
    int ret = 0;
    int index = 0;
    
    char type = (*str_meta)[index++];
    
    if(type == 'g'){
    }else if(type == 'v'){
    }else if(type == 'a'){
    }else{
        goto out;
    }
    //找到开始标记"
    if((*str_meta)[index++] != '\"'){
        goto out;
    }
    *str_meta = *str_meta + index;
    *type_meta = *str_meta;
    index = 0;
    char c = 0;
    int len = (int)strlen(*type_meta);
    while(index < len){
        c = (*type_meta)[index];
        if(c == '\\'){//如果有转移符,跳过随后的一个字符
            index += 2;
            continue;
        }
        //找到结束标记
        if(c == '"'){
            (*type_meta)[index] = 0;
            *str_meta = (*type_meta) + index + 1;
            ret = type;
            break;
        }
        index ++;
    }
out:
    return ret;
}

int ff_metadata_parse_type(FFMetadataContext *ctx, char * str_meta, char type){
    if(ctx == NULL){
        return -1;
    }
    int ret = -1;

    FFMetadata * meta = NULL;
    char * pair = NULL;
    while(str_meta != NULL){
        meta = NULL;
        ret = ff_metadata_parse_pair_cut(&str_meta, &pair);
        if(ret < 0){
            goto err;
        }
        if(ret == 0){
            break;
        }
        
        meta = ff_metadata_parse_pair(pair);
        if(meta == NULL){
            goto err;
        }
        ret = ff_metadata_add(ctx, meta, type);
        if(ret < 0){
            goto err;
        }
    }
    return 0;
err:
    if(meta){
        ff_metadata_node_free(&meta);
    }
    return -1;
}

int ff_metadata_parse_pair_cut(char ** str_meta, char **pair){
    int index = 0;
    char c = 0;
    int len = (int)strlen(*str_meta);
    *pair = *str_meta;
    while(index < len){
        c = (*str_meta)[index];
        if(c == '\\'){
            index += 2;
            continue;
        }
        if(c == ','){
            (*pair)[index] = 0;
            *str_meta = (*str_meta) + index + 1;
            break;
        }else if(c == 0){
            (*pair)[index] = 0;
            *str_meta = NULL;
            break;
        }
        index ++;
    }
    if(index >= len){
        *str_meta = NULL;
    }
    return 1;
}

FFMetadata * ff_metadata_parse_pair(char * pair){
    int index = 0;
    int len = (int)strlen(pair);
    char c = 0;
    char *name = pair;
    char *val = NULL;
    while(index < len){
        c = pair[index];
        if(c == '\\'){
            index +=2;
            continue;
        }
        
        if(c == '='){
            pair[index] = 0;
            val = pair + index + 1;
            break;
        }
        index ++;
    }
    if(val == NULL){
        val = "";
    }
    return ff_metadata_alloc_string_node(name, val);
}
