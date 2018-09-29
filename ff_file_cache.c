//
//  ff_file_cache.c
//  ffmpeg_demo
//
//  Created by helei on 2018/8/27.
//  Copyright © 2018年 何磊. All rights reserved.
//

#include "ff_file_cache.h"
#include "string.h"
#include <memory.h>
#include <stdlib.h>

#include<unistd.h>
#include "ff_macro.h"
#include "ff_log.h"
#include "ff_header.h"
FFFileCacheContext * fff_cache_alloc(const char * dir, const char * file_name){
    if(dir == NULL || file_name == NULL){
        LOGE("%s:dir or file name is NULL\n", __FUNCTION__);
        return NULL;
    }
    int len = 0;
    int ret = 0;
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(file_name);
    if(dir_len == 0 || name_len == 0){
        LOGE("%s:dir or file name is empty\n", __FUNCTION__);
        return NULL;
    }
    FFFileCacheContext * ctx = (FFFileCacheContext *)malloc(sizeof(FFFileCacheContext));
    if(ctx == NULL){
        LOGE("%s:memory error(%d)\n", __FUNCTION__, __LINE__);
        goto err;
    }
    memset(ctx, 0, sizeof(FFFileCacheContext));
    
    ctx->file_size = -1;
    ctx->dir = malloc(dir_len + 1);
    if(ctx->dir == NULL){
        LOGE("%s:memory error(%d)\n", __FUNCTION__, __LINE__);
        goto err;
    }
    strcpy(ctx->dir, dir);
    while(ctx->dir[dir_len - 1] == '/' || ctx->dir[dir_len - 1] == '\\'){
        ctx->dir[dir_len - 1] = 0;
        dir_len --;
    }
    ctx->file_name = malloc(name_len + 1);
    if(ctx->file_name == NULL){
        LOGE("%s:memory error(%d)\n", __FUNCTION__, __LINE__);
        goto err;
    }
    strcpy(ctx->file_name, file_name);
    
    ctx->full_name = malloc(dir_len + name_len + 1);
    sprintf(ctx->full_name, "%s/%s", ctx->dir, ctx->file_name);
    ctx->fragment_list = fff_list_alloc();
    
    unsigned char * serialize_data = NULL;

    int db_name_len = (int)(dir_len + 1 + name_len + 3);
    ctx->db_file_full_name = malloc(db_name_len + 1);
    sprintf(ctx->db_file_full_name, "%s/%s.db", dir, file_name);
    len = fff_cache_read_db_data(ctx, &serialize_data);
    if(len > 0){
        ret = fff_cache_deserialization(ctx, serialize_data, len);
        if(ret < 0){
            LOGW("%s:deserialization cache failed\n", __FUNCTION__);
            FILE * f = fopen(ctx->db_file_full_name, "wb");
            fclose(f);
            f = NULL;
        }
    }
    if(serialize_data){
        free(serialize_data);
        serialize_data = NULL;
    }
    return ctx;
err:
    if(ctx){
        if(ctx->dir){
            free(ctx->dir);
        }
        if(ctx->file_name){
            free(ctx->file_name);
        }
        if(ctx->full_name){
            free(ctx->full_name);
        }
        if(ctx->fragment_list){
            fff_list_free(&ctx->fragment_list);
        }
        free(ctx);
    }
    return NULL;
}

void fff_cache_free(FFFileCacheContext ** pp){
    if(pp == NULL || *pp == NULL){
        return;
    }
    FFFileCacheContext * ctx = *pp;
    if(ctx){
        if(ctx->updated){
            unsigned char * out_data = NULL;
            int out_data_len = fff_cache_serialization(ctx, &out_data);
            if(out_data_len > 0){
                fff_cache_write_db_data(ctx, out_data, out_data_len);
            }
            fff_freep((void**)&out_data);
        }
        if(ctx->inf){
            fclose(ctx->inf);
            ctx->inf = NULL;
        }
        if(ctx->outf){
            fclose(ctx->outf);
            ctx->outf = NULL;
        }
    }
    *pp = NULL;
}

int fff_cache_serialization(FFFileCacheContext * ctx, unsigned char ** out_buf){
/*
 name               datatype        val         size
 ----------------------------------
 File Tag           chars           "CADB"      4bytes
 verison            int16_t         0001        2bytes
 File size          int64_t         0x.....     8bytes
 cache_full         int8_t          0/1         1bytes
    fff_list序列化数据 ...             ...         N
 */
    int size = 4 + 2 + 8 + 1;
    int is_cache_full = fff_cache_is_complete(ctx);
    int ret = 0;
    unsigned char * list_data = NULL;
    int list_data_size = 0;
    
    if(is_cache_full == 0){

        ret = fff_list_serialize(ctx->fragment_list, &list_data, &list_data_size);
        if(list_data_size <= 0){
            //err;
            if(list_data){
                fff_freep((void**)&list_data);
            }
            is_cache_full = 0;
        }else{
            size += list_data_size;
        }
    }
    int index = 0;
    unsigned char * buf = malloc(size);
    buf[index] = 'C';   index++;
    buf[index] = 'H';   index++;
    buf[index] = 'D';   index++;
    buf[index] = 'B';   index++;
    
    DATA_FROM_16(buf + index, 1);   index += 2; //version
    DATA_FROM_64(buf + index, ctx->file_size);  index += 8; //file size
    buf[index] = is_cache_full != 0 ? 1 : 0;    index ++;
    if(is_cache_full == 0 && list_data){
        //fragmens count
        memcpy(buf + index, list_data, list_data_size);
        index += list_data_size;
    }
    *out_buf = buf;
    return size;
err:

    if(buf){
        fff_freep(&buf);
    }
}

int fff_cache_deserialization(FFFileCacheContext * ctx, unsigned char * data, int data_len){
    int index = 0;
    if(data[0] != 'C' || data[1] != 'H' || data[2] != 'D' || data[3] != 'B'){
        return -1;
    }
    int ret = 0;
    index = 4;
    int16_t version = DATA_TO_I16(data + index);    index += 2;
    if(version <= 0){
        goto err;
    }
    int64_t file_size = DATA_TO_I64(data + index);  index += 8;
    unsigned char cache_full = data[index];   index ++;
    FFFragmentList * list = fff_list_alloc();

    if(!cache_full){
        ret = fff_list_deserialize(list, data + index, data_len - index);
        if(ret < 0){
            //err
            LOGW("%s:deserialize db file failed\n", __FUNCTION__);
            return -1;
        }
    }
    ctx->version = version;
    ctx->file_size = file_size;
    ctx->cache_full = cache_full;
    if(ctx->fragment_list){
        fff_list_free(&ctx->fragment_list);
    }
    ctx->fragment_list = list;
    return 0;
err:
    return -1;
}

void fff_free(void *p){
    if(p == NULL){
        return;
    }
    free(p);
}

void fff_freep(void **p){
    if(p == NULL){
        return;
    }
    fff_free(*p);
    *p = NULL;
}

int fff_cache_read_db_data(FFFileCacheContext * ctx, unsigned char ** out_data){
    if(ctx == NULL || ctx->db_file_full_name == NULL || out_data == NULL){
        return -1;
    }
    FILE * f = fopen(ctx->db_file_full_name, "rb");
    if(f == NULL){
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    if(size == 0){
        return 0;
    }
    unsigned char * buf = malloc(size);
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, size, f);
    fclose(f);
    f = NULL;
    *out_data = buf;
    return (int)size;
}

int fff_cache_write_db_data(FFFileCacheContext * ctx, unsigned char * data, int data_len){
    if(ctx == NULL || ctx->db_file_full_name == NULL || data == NULL || data_len <= 0){
        return -1;
    }
    FILE * f = fopen(ctx->db_file_full_name, "wb");
    if(f == NULL){
        return 0;
    }
    fseek(f, 0, SEEK_SET);
    int64_t r = fwrite(data, 1, data_len, f);
    fclose(f);
    f = NULL;
    return (int)r;
}

int fff_cache_is_complete(FFFileCacheContext * ctx){
    if(ctx->file_size < 0){
        return 0;
    }
    if(ctx->cache_full != 0){
        return 1;
    }
    int size = fff_list_size(ctx->fragment_list);
    if(size != 1){
        return 0;
    }
    const FFFragmentNode * node = fff_list_peek(ctx->fragment_list, 0);
    if(node == NULL){
        return 0;
    }
    if(node->begin != 0 || node->end != ctx->file_size - 1){
        return 0;
    }
    
    return 1;
}

int fff_cache_set_size(FFFileCacheContext * ctx, int64_t size){
    if(ctx == NULL || size <= 0){
        return -1;
    }
    if(ctx->file_size != size){
        ctx->file_size = size;
        ctx->updated = 1;
    }
    return 0;
}

int64_t fff_cache_get_size(FFFileCacheContext * ctx){
    return ctx->file_size >= 0 ? ctx->file_size : -1;
}

int fff_cache_init_read(FFFileCacheContext * ctx){
    if(ctx->inf){
        return 0;
    }
    FILE * f = NULL;
    if(access(ctx->full_name, F_OK | R_OK)){
        f = fopen(ctx->full_name, "wb");
        if(f){
            fclose(f);
        }
    }
    f = fopen(ctx->full_name, "rwb");
    if(f == NULL){
        return -1;
    }
    fseek(f, 0, SEEK_SET);
    ctx->inf = f;
    return 0;
}

int fff_cache_init_write(FFFileCacheContext * ctx){
    if(ctx->outf){
        return 0;
    }
    FILE * f = fopen(ctx->full_name, "wb");
    if(f == NULL){
        return -1;
    }
    fseek(f, 0, SEEK_SET);
    ctx->outf = f;
    return 0;
}

int fff_cache_read(FFFileCacheContext * ctx, unsigned char ** out_data, int read_size){
    if(ctx == NULL || out_data == NULL || read_size <= 0){
        LOGE("%s:ctx is NULL or out_data == NULL or  read_size <= 0\n", __FUNCTION__);
        return -1;
    }
    if(fff_cache_init_read(ctx)){
        LOGE("%s:Can`t init reader file\n", __FUNCTION__);
        return -1;
    }
    unsigned char * buf = malloc(read_size);
    if(buf == NULL){
        LOGE("%s:memory error\n", __FUNCTION__);
        return -1;
    }
    int r =  fff_cache_read2(ctx, buf, read_size);
    if(r != read_size){
        LOGE("%s:read failed\n", __FUNCTION__);
        goto err;
    }
    *out_data = buf;
    return r;
err:
    if(buf){
        free(buf);
    }
    return 0;
}

int fff_cache_read2(FFFileCacheContext * ctx, unsigned char * out_buf, int read_size){
    if(ctx == NULL || out_buf == NULL || read_size <= 0){
        LOGE("%s:input ctx is NULL,or out_buf == NULL || read_size <= 0\n", __FUNCTION__);
        return -1;
    }
    if(fff_cache_init_read(ctx)){
        LOGE("%s:Can`t init reader file\n", __FUNCTION__);
        return -1;
    }
    
    if(ctx->cache_full == 0 && !fff_list_range_hit(ctx->fragment_list, ctx->read_pos, ctx->read_pos + read_size - 1)){
        return 0;
    }
    LOGD("%s:current read pos = %ld\n", __FUNCTION__, ftell(ctx->inf));
    if(ftell(ctx->inf) != ctx->read_pos){
        int64_t pos = ctx->read_pos;
        if(fff_cache_read_seek(ctx, ctx->read_pos, SEEK_SET) != pos){
            LOGE("%s:Can`t seek to %lld\n", __FUNCTION__, pos);
            goto err;
        }
    }
    size_t r = fread(out_buf, 1, read_size, ctx->inf);
    if(r == 0){
        //如果到达结尾, 并且文件指针不等于预定大小-1
        if(feof(ctx->inf) && ctx->cache_full != 0){
            if(ctx->file_size > 0 && (ftell(ctx->inf) != ctx->file_size - 1)){
                LOGE("%s:The cache file is corrupted, reset cache info...\n", __FUNCTION__);
                fff_list_clean(ctx->fragment_list);//重置缓存
                ctx->cache_full = 0;
                ctx->updated = 1;
            }else{
                r = AVERROR_EOF;
            }
        }
    }
    ctx->read_pos += r;
    return (int)r;
err:
    return 0;
}

int64_t fff_cache_read_seek(FFFileCacheContext * ctx, int64_t seek_pos, int whence){
    if(ctx == NULL || seek_pos < 0){
        return -1;
    }
    if(ctx->file_size <= 0){
        LOGE("%s:unknow file size, seek failed\n", __FUNCTION__);
        return -1;
    }
    if(fff_cache_init_read(ctx)){
        LOGE("%s:Can`t init reader file\n", __FUNCTION__);
        return -1;
    }
    switch (whence) {
        case SEEK_SET:
        case SEEK_CUR:
        case SEEK_END:
            break;
        case 0x10000://size
            return ctx->file_size;
        case 0x20000:
            whence = SEEK_SET;
            break;
        default:
            goto err;
            break;
    }
    int r = fseek(ctx->inf, seek_pos, whence);
    
    if(r != 0){
        goto err;
    }
    int64_t cur_pos = ftell(ctx->inf);
    ctx->read_pos = cur_pos;
    return cur_pos;
err:
    return -1;
}

int64_t fff_cache_read_pos(FFFileCacheContext * ctx){
    return ctx->read_pos;
}

int fff_cache_write(FFFileCacheContext * ctx, unsigned char * data, int data_size){
    LOGD("%s:write data len = %d\n", __FUNCTION__, data_size);
    if(ctx == NULL || data == NULL || data_size < 0){
        return -1;
    }
    ctx->updated = 1;
    if(fff_cache_init_write(ctx)){
        LOGE("%s:Can`t init reader file\n", __FUNCTION__);
        return -1;
    }
    const FFFragmentNode *node = fff_list_range_node(ctx->fragment_list, ctx->write_pos, ctx->write_pos + data_size - 1);
    if(node){
        return data_size;
    }
    if(ftell(ctx->outf) != ctx->write_pos){
        fseek(ctx->outf, ctx->write_pos, SEEK_SET);
    }
    LOGD("%s:write data size = %d, pos(%lld-%lld)\n", __FUNCTION__, data_size, ctx->write_pos, ctx->write_pos + data_size - 1);
    size_t r = fwrite(data, 1, data_size, ctx->outf);
    if(r != data_size){
        LOGE("%s:cache file write failed\n", __FUNCTION__);
        return -1;
    }
    fflush(ctx->outf);
    if(fff_list_insert(ctx->fragment_list, ctx->write_pos, ctx->write_pos + data_size - 1) < 0){
        LOGE("%s:fragment list insert failed\n", __FUNCTION__);
        return -1;
    }
    ctx->write_pos += r;
    return data_size;
}

int64_t fff_cache_write_pos(FFFileCacheContext * ctx){
    if(ctx == NULL){
        LOGE("%s:ctx is NULL\n", __FUNCTION__);
        return -1;
    }
    return ctx->write_pos;
}

int64_t fff_cache_write_seek(FFFileCacheContext * ctx, int64_t seek_pos){
    if(ctx == NULL || seek_pos < 0){
        LOGE("%s:ctx is NULL or seek_pos < 0\n", __FUNCTION__);
        return -1;
    }
    if(fff_cache_init_write(ctx)){
        LOGE("%s:Can`t init reader file\n", __FUNCTION__);
        return -1;
    }
    ctx->write_pos = seek_pos;
    return seek_pos;
}

int fff_cache_read_pos_hit(FFFileCacheContext * ctx, int64_t begin_pos, int64_t end_pos){
    if(ctx == NULL || begin_pos > end_pos){
        return -1;
    }
    if(ctx->cache_full){
        if(begin_pos >= 0 && begin_pos < ctx->file_size){
            return 1;
        }
        return 0;
    }
    const FFFragmentNode * begin_node = fff_list_range_node(ctx->fragment_list, begin_pos, end_pos);
    if(begin_node){
        return 1;
    }
    return 0;
}

int fff_cache_read_eof(FFFileCacheContext * ctx){
    if(ctx == NULL){
        return 0;
    }
    if(ctx->cache_full && ctx->file_size > 0 && ctx->read_pos == ctx->file_size){
        return 1;
    }
    return 0;
}

