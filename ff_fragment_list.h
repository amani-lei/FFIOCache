//
//  ff_fragment_list.h
//  ffmpeg_demo
//
//  Created by helei on 2018/9/7.
//  Copyright © 2018年 何磊 <helei0908@hotmail.com>. All rights reserved.
//

#ifndef ff_fragment_list_h
#define ff_fragment_list_h

#include <stdio.h>


/**
 在文件写入过程中, 可能会出现seek操作并导致'有效的写入数据'的以碎片的形式存在
 
 本模块用来帮助管理 '范围碎片' 的一个列表, 双向链表实现
 
 用begin和end两个范围数值来表示一个碎片
 如果两个碎片紧邻, 执行合并
 如果两个碎片相交, 执行合并
 如果新的碎片范围包含一个或多个已经存在的碎片, 新碎片将会替代原碎片
 
 */




typedef int64_t fft_t;
//typedef int32_t fft_t;

/**
 碎片节点结构体
 */
typedef struct FFFragmentNode{
    struct FFFragmentNode * next;   //下一个节点
    struct FFFragmentNode * prev;   //上一个节点
    fft_t begin;                    //节点的开始(包含)
    fft_t end;                      //节点的结束(包含)
}FFFragmentNode;


/**
 创建一个节点

 @param begin 节点开始(包含)
 @param end 节点结束(包含), 不能小于begin
 @return 成功返回一个节点地址, 否则返回NULL
 */
FFFragmentNode * fff_list_node_alloc(fft_t begin, fft_t end);


/**
 释放一个节点

 @param p 节点地址
 */
void fff_list_node_free(FFFragmentNode * p);

/**
 释放一个节点,并将节点指针置为NULL
 
 @param pp 节点指针的地址
 */
void fff_list_node_freep(FFFragmentNode ** pp);


/**
 碎片列表s结构体
 */
typedef struct FFFragmentList{
    FFFragmentNode * list_head;     //节点链表的头
    FFFragmentNode * list_end;      //节点链表的尾
    int nb_nodes;                   //节点数
}FFFragmentList;


/**
 创建一个碎片列表

 @return 成功返回列表结构体d指针, 否则返回NULL
 */
FFFragmentList * fff_list_alloc(void);

/**
 释放列表,并将列表指针置为NULL

 @param pp 列表指针的地址
 */
void fff_list_free(FFFragmentList ** pp);


/**
 序列化列表的

 @param list 列表
 @param out_data 用来接收输出缓存数据指针的地址
 @param out_size 序列化数据长度
 @return 成功返回序列化数据长度, 否则返回小于等于0的值
 */
int fff_list_serialize(FFFragmentList * list, unsigned char ** out_data, int *out_size);

/**
 反序列化列表到一个list
    这个操作会重置list之前的信息

 @param list 列表
 @param data 序列化的数据
 @param data_size 数据长度
 @return 成功返回0, 否则返回小于0的值
 */
int fff_list_deserialize(FFFragmentList * list, unsigned char * data, int data_size);


/**
 返回list的节点个数

 @param list 列表
 @return 成功返回节点个数, 否则返回小于0的值
 */
int fff_list_size(FFFragmentList * list);


/**
 插入一个碎片,
 如果要插入的碎片与现有节点重叠, 那么这次插入将会被忽略
 如果要插入的碎片与现有节点部分重叠, 那么这次插入会按照重叠类型来选择 扩展或合并 现有节点
 如果要插入的碎片与现有节点没有重叠, 那么这次插入会新增一个节点

 @param list 列表
 @param begin 碎片开始位置
 @param end 碎片结束位置,不能大于begin
 @return 成功返回节点个数, 否则返回小于等于0的值
 */
int fff_list_insert(FFFragmentList * list, fft_t begin, fft_t end);


/**
 删除范围内的片段
 要删除的只是一个范围
 所有在范围内的节点都会删除, 如果一个节点只有部分在要删除的范围内, 只会除掉在范围内的部分
 
 例如:
 现有节点:  0-50 55-73 90-110
 要删除范围: 7-90
 删除后:   0-6 91-110
 
 @param list 列表
 @param begin 片段开始
 @param end 片段结束, 不能大于begin
 @return 成功返回删除后的节点个数, 否则返回小于0的值
 */
int fff_list_remove(FFFragmentList * list, fft_t begin, fft_t end);


/**
 清理所有节点
 
 @param list 列表
 @return 成功返回0, 否则返回非0值
 */
int fff_list_clean(FFFragmentList * list);

//=============以下为内部接口, 除非明确其作用, 否则不建议调用    ===================//

/**
 在头部插入一个碎片
 
 @param list 列表
 @param begin 碎片开始位置
 @param end 碎片结束位置,不能大于begin
 @return 成功返回节点个数, 否则返回小于等于0的值
 */
int fff_list_insert_to_head(FFFragmentList * list, fft_t begin, fft_t end);

/**
 在尾部插入一个碎片
 
 @param list 列表
 @param begin 碎片开始位置
 @param end 碎片结束位置,不能大于begin
 @return 成功返回节点个数, 否则返回小于等于0的值
 */
int fff_list_insert_to_end(FFFragmentList * list, fft_t begin, fft_t end);


/**
 在两个节点之间插入一个节点
 注意,如果两个节点之间存在其他节点, 都会被覆盖掉

 @param list 列表
 @param node 要插入的节点, 不能为NULL
 @param prev 在哪接节点后面插入, 可以为NULL, 但不允许prev和next都为NULL
 @param next 在哪个节点前面插入, 可以为NULL, 但不允许prev和next都为NULL
 @return 成功返回节点个数, 否则返回小于等于0的值
 */
int fff_list_insert_node(FFFragmentList * list, FFFragmentNode * node, FFFragmentNode * prev, FFFragmentNode * next);


/**
 按照索引查看一个节点

 @param list 列表
 @param node_index 节点索引
 @return 成功返回一个节点的引用, 失败返回NULL
 */
const FFFragmentNode * fff_list_peek(FFFragmentList * list, int node_index);


/**
 移除一个节点

 @param list 列表
 @param node 要移除的节点,必须是存在于list的节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_node(FFFragmentList * list, FFFragmentNode * node);

/**
 移除两个节点之间的所有节点, 包含这两个节点
 
 @param list 列表
 @param begin_node 要移除的节点,必须是存在于list的节点
            可以为NULL,如果为NULL表示移除end_node(包含end_node)前面的所有节点
 @param end_node 要移除的节点,必须是存在于list的节点, 并且必须是在begin_node后面的节点
            可以为NULL,如果为NULL表示移除begin_node后面的所有节点(包含begin_node)
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_node2(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node);

/**
 移除两个节点之间的所有节点, 不包含这两个节点
 
 @param list 列表
 @param begin_node 要移除的节点,必须是存在于list的节点
            可以为NULL,如果为NULL表示移除end_node前面的所有节点
 @param end_node 要移除的节点,必须是存在于list的节点, 并且必须是在begin_node后面的节点
            可以为NULL,如果为NULL表示移除begin_node后面的所有节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_node2_except(FFFragmentList * list, FFFragmentNode * begin_node, FFFragmentNode * end_node);

/**
 溢出一个节点后面的所有节点, 包含这个节点

 @param list 列表
 @param begin_node 开始节点, 必须是存在于list中的节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_to_end(FFFragmentList * list, FFFragmentNode * begin_node);

/**
 移除一个节点前面的所有节点, 包含这个节点
 
 @param list 列表
 @param end_node 结束节点, 必须是存在于list中的节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_to_head(FFFragmentList * list, FFFragmentNode * end_node);

/**
 移除一个节点前面的所有节点, 不包含这个节点
 
 @param list 列表
 @param begin_node 开始节点, 必须是存在于list中的节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_to_end_except(FFFragmentList * list, FFFragmentNode * begin_node);

/**
 移除一个节点前面的所有节点, 不包含这个节点
 
 @param list 列表
 @param end_node 结束节点, 必须是存在于list中的节点
 @return 成功返回移除后,list的节点数, 否则返回小于0的值
 */
int fff_list_remove_to_head_except(FFFragmentList * list, FFFragmentNode * end_node);


/**
 合并两个节点

 @param list 列表
 @param node1 第一个节点
 @param node2 第二个节点
 @return 成功返回合并后的节点数, 否则返回小于等于0的值
 */
int fff_list_bonding_node(FFFragmentList * list, FFFragmentNode * node1, FFFragmentNode * node2);


/**
 去掉一个节点的一部分

 @param list 列表
 @param node 节点
 @param point 从何处开始去除
 @param is_cut_head 是否是去头:
 例如:
 节点范围   21-40
    条件1:
    point=30
    is_cut_head = 1
            结果:31-40
    条件2:
    point=30
    is_cut_head = 0
            结果:21-29
 @return 成功返回不小于0的值
 */
int fff_list_cutting_node(FFFragmentList * list, FFFragmentNode * node, fft_t point, int is_cut_head);

#define FFF_FIND_FLAG_ACCURATE  0   //精确查找
#define FFF_FIND_FLAG_PREV  1       //前向查找,先执行精确查找,未找到的话,返回前一个节点,如果不存在前一个节点, 返回NULL
#define FFF_FIND_FLAG_NEXT  2       //后向查找,先执行精确查找,未找到的话,返回后一个节点,如果不存在后一个节点, 返回NULL

#define FFF_FIND_NO     0           //未找到
#define FFF_FIND_OK     1           //找到
#define FFF_FIND_2TH    2           //找到次要目的, 当find_flag为FFF_FIND_FLAG_NEXT或FFF_FIND_FLAG_PREV时, 可能会返回该值

/**
 查找一个节点

 @param list 列表
 @param point 点
 @param flag 查找flag, 参考FFF_FIND_FLAG_XXX
 @param out_node 返回一个节点的引用,
                不要主动释放这个节点,除非明确知道,这个节点已经通过其他接口中在list中移除
 @return 参考FFF_FIND_XXX, 出现错误返回小于0的值
 */
int fff_list_find_node(FFFragmentList * list, fft_t point, int flag, FFFragmentNode ** out_node);

/**
 通过索引查找一个节点

 @param list 列表
 @param index 节点索引
 @param out_node 输出节点
 @return 参考FFF_FIND_XX
 */
int fff_list_find_node_index(FFFragmentList * list, int index, FFFragmentNode ** out_node);

/**
 返回一个范围内的节点,
 只有在begin(包含)和end(包含)都在同一个节点范围内才会成功

 @param list 列表
 @param begin 开始
 @param end 结束
 @return 成功返回节点, 否则返回NULL
 */
const FFFragmentNode * fff_list_range_node(FFFragmentList * list, fft_t begin, fft_t end);

/**
 测试一个范围是否被节点覆盖
 
 @param list 列表
 @param begin 开始
 @param end 结束
 @return 成功返回非0值, 否则返回0
 */
int fff_list_range_hit(FFFragmentList * list, fft_t begin, fft_t end);
#endif /* ff_fragment_list_h */
