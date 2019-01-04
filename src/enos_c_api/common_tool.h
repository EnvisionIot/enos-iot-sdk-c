/*******************************************************************************
 * Copyright 2018-present Envision Digital.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *    yang.zhang4
 *******************************************************************************/

#ifndef COMMON_TOOL_H
#define COMMON_TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------hash table
extern unsigned int ctool_bkdr_hash_fun(char *para, int len);

struct ctool_hash_element
{
    //user data
    void *data;
    
    //conflict list
    void *ptr_next;
};

struct ctool_hash_table
{
    int (*compare_fun)(void *, void *);//compare function
    int (*free_fun)(void *);//free function
    unsigned int (*hash_fun)(void *);//hash function
    int (*copy_fun)(void *, void *);//copy function,copy first to second
    int (*print_fun)(void *);//print function
    int max_num;//max elements number of hash main table,2^n
    int para_len;//size of single element
    volatile int count;//current element count
    struct ctool_hash_element *table;//hash main table
};

//init
extern struct ctool_hash_table *ctool_hashtable_init(int max_num, \
    int para_len, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    unsigned int (*hash_fun)(void *), \
    int (*copy_fun)(void *, void *), \
    int (*print_fun)(void *));
//destory
extern int ctool_hashtable_destory(struct ctool_hash_table *table_in);
//insert
extern int ctool_hashtable_insert(struct ctool_hash_table *table_in, void *para);
//search
extern void *ctool_hashtable_find(struct ctool_hash_table *table_in, void *para);
//delete
extern int ctool_hashtable_delete(struct ctool_hash_table *table_in, void *para);
//print
extern void ctool_hashtable_print(struct ctool_hash_table *table_in);





//---------------------------------------------------------circular_queue

struct ctool_cq_element
{
    //user data
    void *data;
};

struct ctool_circular_queue
{
    int (*compare_fun)(void *, void *);//compare function
    int (*free_fun)(void *);//free function
    int (*copy_fun)(void *, void *);//copy function,copy first to second
    int (*print_fun)(void *);//print function
    int max_num;//max num of elements,2^n,actual capacity is max_num-1
    int para_len;//size of single element
    int front; //front pointer,if queue is not empty, it is index of front element
    int rear; //rear pointer,if queue is not empty,it is index after tail element
    struct ctool_cq_element *circular_queue;//queue
};

//init
extern struct ctool_circular_queue *ctool_circular_queue_init(int max_num, \
    int para_len, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    int (*copy_fun)(void *, void *), \
    int (*print_fun)(void *));

//destory
extern int ctool_circular_queue_destory(struct ctool_circular_queue *cq_in);
//clear(not destory)
extern int ctool_circular_queue_clear(struct ctool_circular_queue *cq_in);
//is empty?
extern int ctool_circular_queue_is_empty(struct ctool_circular_queue *cq_in);
//is full?
extern int ctool_circular_queue_is_full(struct ctool_circular_queue *cq_in);
//get number of elements
extern int ctool_circular_queue_get_num(struct ctool_circular_queue *cq_in);
//search
extern int ctool_circular_queue_is_contain(struct ctool_circular_queue *cq_in, void *para);
//push back
extern int ctool_circular_queue_pushback(struct ctool_circular_queue *cq_in, void *para);
//pop front
extern int ctool_circular_queue_popfront(struct ctool_circular_queue *cq_in, void *para);






//---------------------------------------------------------checksum of IP header
extern unsigned short ctool_ip_cksum(unsigned short *addr, int len);






//---------------------------------------------------------merge sort for basic types
typedef unsigned short ctool_merge_type;
//mode=0 Ascending order,mode=1 Descending order
extern int ctool_merge_sort(ctool_merge_type *array, int length, int mode);





//---------------------------------------------------------merge sort for general types
struct ctool_general_merge_struct
{
    void *array;//Array of elements to be sorted
    int para_len;//length of single element
    int para_num;//number of elements
    int (*compare_fun)(void *, void *);//compare function,if first<second return -1,if first==second return 0,if first>second return 1
    int (*free_fun)(void *);//free function
    int (*copy_fun)(void *, void *);//copy function,copy second to first
};

extern int ctool_general_merge_sort(void *array, \
    int para_len, \
    int para_num, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    int (*copy_fun)(void *, void *));

//---------------------------------------------------------file operation
//get file length
extern int get_file_size(char *path);
//get len bytes form file
extern int read_file_content(char *path, char *buf, int len);
//write len bytes to file
extern int write_file_content(char *path, char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif