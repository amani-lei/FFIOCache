//
//  ff_fragment_list.c
//  ffmpeg_demo
//
//  Created by helei on 2018/9/7.
//  Copyright © 2018年 何磊. All rights reserved.
//

#include "ff_fragment_list.h"
#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "ff_macro.h"
#include "ff_log.h"

FFFragmentNode * fff_list_node_alloc(fft_t begin, fft_t end){
    FFFragmentNode * node = (FFFragmentNode*)malloc(sizeof(FFFragmentNode));
    node->next = NULL;
    node->prev = NULL;
    node->begin = begin;
    node->end = end;
    return node;
}

void fff_list_node_free(FFFragmentNode * p){
    if(p){
        free(p);
    }else{
        LOGW("%s:free an NULL pointer ", __FUNCTION__);
    }
}

void fff_list_node_freep(FFFragmentNode ** pp){
    if(pp == NULL || *pp == NULL){
        LOGW("%s:free an NULL pointer ", __FUNCTION__);
        return;
    }
    fff_list_node_free(*pp);
    *pp = NULL;
}

FFFragmentList * fff_list_alloc(void){
    FFFragmentList * list = (FFFragmentList*)malloc(sizeof(FFFragmentList));
    memset(list, 0, sizeof(FFFragmentList));
    return list;
}

void fff_list_free(FFFragmentList ** pp){
    if(pp == NULL && *pp == NULL){
        LOGW("%s:free an NULL pointer ", __FUNCTION__);
        return;
    }
    fff_list_clean(*pp);
    free(*pp);
    *pp = NULL;
}
int fff_list_serialize(FFFragmentList * list, unsigned char ** out_data, int *out_size){
    /**
     nb_nodes   4bytes
     node0_begin     8byte
     node0_end       8bytes
     .....
     nodeN_begin     8byte
     nodeN_end       8bytes
     */
    int nodes = list->nb_nodes;
    int buf_size = 4 + 4 + (16 * nodes);
    unsigned char * buf = malloc(4 + 4 + (16 * nodes));
    if(buf == NULL){
        LOGE("%s:alloc serialize buffer failed\n", __FUNCTION__);
        return -1;
    }
    memset(buf, 0, buf_size);
    FFFragmentNode * node = list->list_head;
    int index = 0;
    DATA_FROM_32(buf + index, nodes);   index += 4;
    int i = 0;
    LOGI("%s:node size = %d\n", __FUNCTION__, nodes);
    for(i = 0; i < nodes;){
        DATA_FROM_64(buf + index, node->begin); index += 8;
        DATA_FROM_64(buf + index, node->end);   index += 8;
        i++;
        if(node->next == NULL){
            break;
        }
        node = node->next;
    }
    if(nodes != i){
        DATA_FROM_32(buf + 4, nodes);;
    }
    *out_data = buf;
    *out_size = index;
    return 0;
}

int fff_list_deserialize(FFFragmentList * list, unsigned char * data, int data_size){
    int index = 0;
    if(list == NULL || data_size < 4){
        LOGE("%s:list is NULL or data length short then 4 bytes\n", __FUNCTION__);
        return -1;
    }
    int nodes = DATA_TO_I32(data); index += 4;
    if(nodes < 0){
        LOGE("%s:invalid node count(%d)\n", __FUNCTION__, nodes);
        return -1;
    }
    LOGI("%s:ndoe size = %d\n", __FUNCTION__, nodes);
    if(data_size < (nodes * 16 + 4)){
        LOGE("%s:data length is %d, short then required(%d)\n", __FUNCTION__, data_size, (nodes * 16 + 4));
        return -1;
    }
    int i = 0;
    int64_t begin = 0;
    int64_t end = 0;
    fff_list_clean(list);
    for(i = 0; i < nodes; i++){
        begin = DATA_TO_I64(data + index); index += 8;
        end = DATA_TO_I64(data + index); index += 8;
        if(fff_list_insert(list, begin, end) < 1){
            LOGE("%s:insert (%lld,%lld) to list recoder failed\n", __FUNCTION__, begin, end);
        }
    }
    return 0;
}

int fff_list_size(FFFragmentList * list){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    return list->nb_nodes;
}

int fff_list_insert(FFFragmentList * list, fft_t begin, fft_t end){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end < begin){
        LOGE("%s:end(%lld) is small then begin(%lld)\n", __FUNCTION__, end, begin);
        return -1;
    }
    if(begin > 11209566 || end > 11209566){
        LOGD("%s:\n", __FUNCTION__);
    }
    if(list->nb_nodes == 0){
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        list->list_head = node;
        list->list_end = node;
        list->nb_nodes = 1;
        return list->nb_nodes;
    }

    //如果有任何一个为NULL,就执行前/后插入
    FFFragmentNode * begin_node = NULL;
    fff_list_find_node(list, begin, FFF_FIND_FLAG_PREV, &begin_node);
    FFFragmentNode * end_node = NULL;
    fff_list_find_node(list, end, FFF_FIND_FLAG_NEXT, &end_node);
    if(begin_node == NULL && end_node == NULL){
        fff_list_clean(list);
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        list->list_head = node;
        list->list_end = node;
        list->nb_nodes = 1;
        return list->nb_nodes;
    }
    if(begin_node == NULL){
        fff_list_remove_to_head_except(list, end_node);
        return fff_list_insert_to_head(list, begin, end);
    }
    if(end_node == NULL){
        fff_list_remove_to_end_except(list, begin_node);
        return fff_list_insert_to_end(list, begin, end);
    }
    
    //这里是中间插入
    fff_list_remove_node2_except(list, begin_node, end_node);//移除开始节点(不包含)和结束节点(不包含)之间的所有节点
    
    if((begin <= begin_node->end + 1) && (end >= end_node->begin - 1)){//前后都能衔接
        begin_node->end = end_node->end;
        begin_node->next = end_node->next;
        if(end_node->next){
            end_node->next->prev = begin_node;
        }
        if(end_node == list->list_end){
            list->list_end = begin_node;
        }
        fff_list_node_freep(&end_node);
        list->nb_nodes --;
    }else if((begin > begin_node->end + 1) && (end < end_node->begin - 1)){ //前后都不能衔接
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        fff_list_insert_node(list, node, begin_node, end_node);
    }else if(begin <= begin_node->end + 1){                             //能和前面衔接
        begin_node->end = end;
    }else{                                                              //能和后面衔接
        end_node->begin = begin;
    }
    return list->nb_nodes;
}

int fff_list_insert_to_head(FFFragmentList * list, fft_t begin, fft_t end){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end < begin){
        LOGE("%s:end(%lld) is small then begin(%lld)\n", __FUNCTION__, end, begin);
        return -1;
    }
    if(list->list_head == NULL){
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        list->list_head = node;
        list->list_end = node;
        list->nb_nodes = 1;
        return list->nb_nodes;
    }
    
    FFFragmentNode * begin_node = list->list_head;
    if(begin_node->begin - 1 <= end){//可以衔接
        begin_node->begin = begin;
    }else{
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        begin_node->prev = node;
        node->next = begin_node;
        list->list_head = node;
        list->nb_nodes ++;
    }
    return list->nb_nodes;
}

int fff_list_insert_to_end(FFFragmentList * list, fft_t begin, fft_t end){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end < begin){
        LOGE("%s:end(%lld) is small then begin(%lld)\n", __FUNCTION__, end, begin);
        return -1;
    }
    if(list->list_end == NULL){
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        list->list_head = node;
        list->list_end = node;
        list->nb_nodes = 1;
        return list->nb_nodes;
    }
    
    FFFragmentNode * end_node = list->list_end;
    if(end_node->end + 1 >= begin){//可以衔接
        end_node->end = end;
    }else{
        FFFragmentNode * node = fff_list_node_alloc(begin, end);
        end_node->next = node;
        node->prev = end_node;
        list->list_end = node;
        list->nb_nodes ++;
    }
    return list->nb_nodes;
}

int fff_list_insert_node(FFFragmentList * list, FFFragmentNode * node, FFFragmentNode * prev, FFFragmentNode * next){
    if(list == NULL || node == NULL || prev == NULL || next == NULL){
        if(list == NULL){
            LOGE("%s:input list is NULL\n", __FUNCTION__);
        }
        if(node == NULL){
            LOGE("%s:input node is NULL\n", __FUNCTION__);
        }
        if(prev == NULL){
            LOGE("%s:input prev is NULL\n", __FUNCTION__);
        }
        if(next == NULL){
            LOGE("%s:input next is NULL\n", __FUNCTION__);
        }
        return -1;
    }
    if(prev == next){
        LOGE("%s:prev == next, is invalid\n", __FUNCTION__);
        return -1;
    }
    node->next = next;
    next->prev = node;
    prev->next = node;
    node->prev = prev;
    list->nb_nodes ++;
    return 0;
}

const FFFragmentNode * fff_list_peek(FFFragmentList * list, int node_index){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return NULL;
    }
    
    if(list->list_head == NULL || list->nb_nodes <= 0){
        LOGW("%s:list is empty\n", __FUNCTION__);
        return NULL;
    }
    int i = 0;
    FFFragmentNode * node = list->list_head;
    FFFragmentNode * ret_node = NULL;
    while(node){
        if(i == node_index){
            ret_node = node;
            break;
        }
        node = node->next;
        i ++;
    }
    return ret_node;
}

int fff_list_remove(FFFragmentList * list, fft_t begin, fft_t end){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end < begin){
        LOGE("%s:input begin(%lld) is large then end(%lld)\n", __FUNCTION__, begin, end);
        return -1;
    }
    if(list->list_head == NULL){
        return 0;
    }
    //查找开始结束所在节点
    FFFragmentNode * begin_node = NULL;
    fff_list_find_node(list, begin, FFF_FIND_FLAG_PREV, &begin_node);
    FFFragmentNode * end_node = NULL;
    fff_list_find_node(list, end, FFF_FIND_FLAG_NEXT, &end_node);
    if(begin_node == NULL && end_node == NULL){
        return list->nb_nodes;
    }
    if(begin_node == NULL){
        fff_list_cutting_node(list, end_node, end, 1);
    }
    else if(end_node == NULL){
        fff_list_cutting_node(list, begin_node, begin, 0);
    }else{
        fff_list_cutting_node(list, begin_node, begin, 0);
        fff_list_cutting_node(list, end_node, end, 1);
        if(begin_node != end_node){
            fff_list_bonding_node(list, begin_node, end_node);
        }
    }
    
    return list->nb_nodes;
}
int fff_list_remove_node(FFFragmentList * list, FFFragmentNode * node){
    if(list == NULL || node == NULL){
        LOGE("%s:input list or node is NULL\n", __FUNCTION__);
        return -1;
    }
    FFFragmentNode * n = list->list_head;
    while (n) {
        if(n == node){
            break;
        }
        n = n->next;
    }
    
    if(n != node){
        LOGE("%s:node is not find\n", __FUNCTION__);
        return -1;
    }
    FFFragmentNode * prev = node->prev;
    FFFragmentNode * next = node->next;
    if(node->prev == NULL){
        list->list_head = node->next;
        prev->next = next;
    }
    if(node->next == NULL){
        list->list_end = node->prev;
        if(next){
            next->prev = prev;
        }
    }
    list->nb_nodes --;
    return list->nb_nodes;
}

int fff_list_remove_node2(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(begin_node == NULL && end_node == NULL){
        LOGE("%s:begin and end node is NULL\n", __FUNCTION__);
    }
    if(begin_node == NULL || begin_node == list->list_head){
        return fff_list_remove_to_head(list, end_node);
    }
    if(end_node == NULL || end_node == list->list_end){
        return fff_list_remove_to_end(list, end_node);
    }
    
    if(begin_node->prev == NULL || end_node->next == NULL){
        return -1;
    }
    
    FFFragmentNode * tmp_node = NULL, *head = begin_node->prev, *end = end_node->next;
    FFFragmentNode * node = begin_node;
    while(node){
        tmp_node = node;
        node = node->next;
        fff_list_node_free(tmp_node);
        list->nb_nodes --;
        if(tmp_node == end_node){
            break;
        }
    }
    head->next = end;
    end->prev = head;
    return list->nb_nodes;
}

int fff_list_remove_node2_except(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return list->nb_nodes;
    }
    if(begin_node == end_node || begin_node->next == end_node){
        return list->nb_nodes;
    }
    return fff_list_remove_node2(list, begin_node->next, end_node->prev);
}

int fff_list_remove_to_end(FFFragmentList * list, FFFragmentNode * begin_node){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(begin_node == NULL){
        return list->nb_nodes;
    }
    if(begin_node == list->list_head){
        fff_list_clean(list);
        return 0;
    }
    FFFragmentNode * tmp_node = NULL, *begin = begin_node->prev;
    FFFragmentNode * node = begin_node;
    while(node){
        tmp_node = node;
        node = node->next;
        fff_list_node_freep(&tmp_node);
        list->nb_nodes --;
    }
    begin->next = NULL;
    list->list_end = begin;
    return list->nb_nodes;
}

int fff_list_remove_to_head(FFFragmentList * list, FFFragmentNode * end_node){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end_node == NULL){
        return list->nb_nodes;
    }
    if(end_node == list->list_end){
        fff_list_clean(list);
        return 0;
    }
    FFFragmentNode * tmp_node = NULL, *end = end_node->next;
    FFFragmentNode * node = list->list_head;
    while(node){
        tmp_node = node;
        node = node->next;
        fff_list_node_free(tmp_node);
        list->nb_nodes --;
        if(tmp_node == end_node){
            break;
        }
    }
    end->prev = NULL;
    list->list_head = end;
    return list->nb_nodes;
}

int fff_list_remove_to_end_except(FFFragmentList * list, FFFragmentNode * begin_node){
    if(list == NULL || begin_node == NULL){
        LOGE("%s:input list or begin_node is NULL\n", __FUNCTION__);
        return -1;
    }
    
    if(begin_node->next == NULL){
        return list->nb_nodes;
    }
    return fff_list_remove_to_end(list, begin_node->next);
}

int fff_list_remove_to_head_except(FFFragmentList * list, FFFragmentNode * end_node){
    if(list == NULL || end_node == NULL){
        LOGE("%s:input list or end_node is NULL\n", __FUNCTION__);
        return -1;
    }
    if(end_node->prev == NULL){
        return list->nb_nodes;
    }
    return fff_list_remove_to_head(list, end_node->prev);
}

int fff_list_bonding_node(FFFragmentList * list, FFFragmentNode * node1, FFFragmentNode * node2){
    FFFragmentNode * node = node1;
    FFFragmentNode * prev = node1->prev;
    FFFragmentNode * next = node2->next;
    if(prev == next){
        return -1;
    }
    if(prev == NULL){
        list->list_head = next;
        next->prev = NULL;
    }
    if(next == NULL){
        list->list_end = prev;
        prev->next = NULL;
    }
    node1->prev = NULL;
    node2->next = NULL;
    while(node != node2){
        if(node2->next == NULL){
            break;
        }
        FFFragmentNode * tmp = node;
        node = node->next;
        fff_list_node_free(tmp);
        list->nb_nodes --;
    }
    return list->nb_nodes;
}

int fff_list_cutting_node(FFFragmentList * list, FFFragmentNode * node, fft_t point, int is_cut_head){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    if(node->begin > point || node->end < point){
        LOGE("%s:point(%lld) is not in node(%lld,%lld)\n", __FUNCTION__, point, node->begin, node->end);
        return -1;
    }
    if(is_cut_head){
        node->begin = point + 1;
    }else{
        node->end = point - 1;
    }
    return list->nb_nodes;
}

int fff_list_clean(FFFragmentList * list){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return -1;
    }
    FFFragmentNode * node = list->list_head;
    FFFragmentNode * tmp = NULL;
    while(node){
        tmp = node;
        node = tmp->next;
        fff_list_node_free(tmp);
    }
    list->list_head = NULL;
    list->list_end = NULL;
    list->nb_nodes = 0;
    return 0;
}

int fff_list_find_node(FFFragmentList * list, fft_t point, int flag, FFFragmentNode ** out_node){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return FFF_FIND_NO;
    }
    FFFragmentNode * node = list->list_head;
    FFFragmentNode * prev = NULL;
    FFFragmentNode * find_node = NULL;
    if(flag != FFF_FIND_FLAG_ACCURATE && flag != FFF_FIND_FLAG_PREV && flag != FFF_FIND_FLAG_NEXT){
        LOGE("%s:unknown error\n", __FUNCTION__);
        return FFF_FIND_NO;
    }
    int ret = FFF_FIND_NO;
    while(node){
        if(node->begin <= point && node->end >= point){
            find_node = node;
            ret = FFF_FIND_OK;
            break;
        }else if(node->end > point){
            if(flag == FFF_FIND_FLAG_ACCURATE){
                break;
            }else if(flag == FFF_FIND_FLAG_PREV){
                ret = prev ? FFF_FIND_2TH : FFF_FIND_NO;
                find_node = prev;
                break;
            }else if(flag == FFF_FIND_FLAG_NEXT){
                ret = node ? FFF_FIND_2TH : FFF_FIND_NO;;
                find_node = node;
                break;
            }
        }else{
            if(node->next == NULL && flag == FFF_FIND_FLAG_PREV){
                ret = FFF_FIND_2TH;
                find_node = node;
                break;
            }
        }
        prev = node;
        node = node->next;
    }
    *out_node = find_node;
    return ret;
}
int fff_list_find_node_index(FFFragmentList * list, int index, FFFragmentNode ** out_node){
    if(list == NULL || index < 0){
        LOGE("%s:input list is NULL or index < 0 \n", __FUNCTION__);
        return FFF_FIND_NO;
    }
    if(out_node == NULL ){
        LOGE("%s:out_node is\n", __FUNCTION__);
    }
    if(list->nb_nodes == 0 || list->list_head == NULL){
        LOGE("%s:input list is empty\n", __FUNCTION__);
        return FFF_FIND_NO;
    }
    FFFragmentNode * node = list->list_head, * find_node = NULL;
    int i = 0;
    while(node){
        if(i == index){
            find_node = node;
            break;
        }
        node = node->next;
    }
    if(find_node){
        *out_node = find_node;
        return FFF_FIND_OK;
    }
    return FFF_FIND_NO;
}

const FFFragmentNode * fff_list_range_node(FFFragmentList * list, fft_t begin, fft_t end){
    if(list == NULL){
        LOGE("%s:input list is NULL\n", __FUNCTION__);
        return NULL;
    }
    if(begin < 0 || end <= 0 || end < begin){
        LOGE("%s:begin or end small then zero, or end(%lld) < begin(%lld)\n", __FUNCTION__, begin, end);
        return NULL;
    }
    FFFragmentNode * begin_node = NULL;
    fff_list_find_node(list, begin, FFF_FIND_FLAG_ACCURATE, &begin_node);
    FFFragmentNode * end_node = NULL;
    fff_list_find_node(list, end, FFF_FIND_FLAG_ACCURATE, &end_node);
    if(begin_node != NULL && (begin_node == end_node)){
        return begin_node;
    }
    return NULL;
}

int fff_list_range_hit(FFFragmentList * list, fft_t begin, fft_t end){
    return fff_list_range_node(list, begin, end) != NULL;
}


