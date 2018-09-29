//
//  ff_fragment_list.h
//  ffmpeg_demo
//
//  Created by helei on 2018/9/7.
//  Copyright © 2018年 何磊. All rights reserved.
//

#ifndef ff_fragment_list_h
#define ff_fragment_list_h

#include <stdio.h>

typedef int64_t fft_t;
//typedef int32_t fft_t;

typedef struct FFFragmentNode{
    struct FFFragmentNode * next;
    struct FFFragmentNode * prev;
    fft_t begin;
    fft_t end;
}FFFragmentNode;

FFFragmentNode * fff_list_node_alloc(fft_t begin, fft_t end);
void fff_list_node_free(FFFragmentNode * p);
void fff_list_node_freep(FFFragmentNode ** pp);

typedef struct FFFragmentList{
    FFFragmentNode * list_head;
    FFFragmentNode * list_end;
    int nb_nodes;
}FFFragmentList;


FFFragmentList * fff_list_alloc(void);
void fff_list_free(FFFragmentList ** pp);
int fff_list_serialize(FFFragmentList * list, unsigned char ** out_data, int *out_size);
int fff_list_deserialize(FFFragmentList * list, unsigned char * data, int data_size);
int fff_list_size(FFFragmentList * list);

int fff_list_insert(FFFragmentList * list, fft_t begin, fft_t end);
int fff_list_insert_to_head(FFFragmentList * list, fft_t begin, fft_t end);
int fff_list_insert_to_end(FFFragmentList * list, fft_t begin, fft_t end);
int fff_list_insert_node(FFFragmentList * list, FFFragmentNode * node, FFFragmentNode * prev, FFFragmentNode * next);
const FFFragmentNode * fff_list_peek(FFFragmentList * list, int node_index);
int fff_list_remove(FFFragmentList * list, fft_t begin, fft_t end);
int fff_list_remove_node(FFFragmentList * list, FFFragmentNode * node);
int fff_list_remove_node2(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node);
int fff_list_remove_node2_except(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node);
int fff_list_remove_to_end(FFFragmentList * list, FFFragmentNode * begin_node);
int fff_list_remove_to_head(FFFragmentList * list, FFFragmentNode * end_node);
int fff_list_remove_to_end_except(FFFragmentList * list, FFFragmentNode * begin_node);
int fff_list_remove_to_head_except(FFFragmentList * list, FFFragmentNode * end_node);

int fff_list_bonding_node(FFFragmentList * list, FFFragmentNode * node1, FFFragmentNode * node2);
int fff_list_cutting_node(FFFragmentList * list, FFFragmentNode * node, fft_t point, int is_cut_head);
int fff_list_clean(FFFragmentList * list);

#define FFF_FIND_FLAG_ACCURATE  0   //精确查找
#define FFF_FIND_FLAG_PREV  1       //前向查找,先执行精确查找,未找到的话,返回前一个节点,如果不存在前一个节点, 返回NULL
#define FFF_FIND_FLAG_NEXT  2       //后向查找,先执行精确查找,未找到的话,返回后一个节点,如果不存在后一个节点, 返回NULL

#define FFF_FIND_NO     0
#define FFF_FIND_OK     1
#define FFF_FIND_2TH    2

int fff_list_find_node(FFFragmentList * list, fft_t point, int flag, FFFragmentNode ** out_node);
int fff_list_find_node_index(FFFragmentList * list, int index, FFFragmentNode ** out_node);
const FFFragmentNode * fff_list_range_node(FFFragmentList * list, fft_t begin, fft_t end);
int fff_list_range_hit(FFFragmentList * list, fft_t begin, fft_t end);
#endif /* ff_fragment_list_h */
