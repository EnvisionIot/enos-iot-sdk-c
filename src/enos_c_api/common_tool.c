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

#include "common_tool.h"

extern unsigned int ctool_bkdr_hash_fun(char *para, int len)
{//BKDR Hash Function
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    int ii = 0;
    for(ii = 0; ii < len; ii++)
    {
        hash = hash * seed + (*para++);
    }

    return hash;
}

static int ctool_get_2n(int value)
{
    int len = 0;
    int value_tmp = value;
    int value_ret = 0;
    while(value_tmp != 0)
    {
        value_tmp = value_tmp >> 1;
        len++;
    }
    value_ret = 1 << len;
    if((value != 1) && (value * 2 == value_ret))
    {
        value_ret = value;
    }
    return value_ret;
}

extern struct ctool_hash_table *ctool_hashtable_init(int max_num, \
    int para_len, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    unsigned int (*hash_fun)(void *), \
    int (*copy_fun)(void *, void *), \
    int (*print_fun)(void *))
{
    struct ctool_hash_table *table_ret = NULL;
    
    if((max_num <= 0) || (para_len <= 0) || (compare_fun == NULL) || (hash_fun == NULL) || (copy_fun == NULL))
    {
        return NULL;
    }
    
    table_ret = (struct ctool_hash_table *)malloc(sizeof(struct ctool_hash_table));
    if(table_ret == NULL)
    {
        return NULL;
    }

    table_ret->compare_fun = (int (*)(void *, void *))compare_fun;
    table_ret->free_fun = (int (*)(void *))free_fun;
    table_ret->hash_fun = (unsigned int (*)(void *))hash_fun;
    table_ret->copy_fun = (int (*)(void *, void *))copy_fun;
    table_ret->print_fun = (int (*)(void *))print_fun;
    table_ret->max_num = ctool_get_2n(max_num);
    table_ret->para_len = para_len;
    table_ret->count = 0;
    
    table_ret->table = (struct ctool_hash_element *)malloc(table_ret->max_num * sizeof(struct ctool_hash_element));
    if(table_ret->table == NULL)
    {
        free(table_ret);
        return NULL;
    }

    memset((void *)(table_ret->table), 0, table_ret->max_num * sizeof(struct ctool_hash_element));
    return table_ret;
}

extern int ctool_hashtable_destory(struct ctool_hash_table *table_in)
{
    int ii = 0;
    struct ctool_hash_element *tmp1;
    struct ctool_hash_element *tmp2;
    for(ii = 0; ii < table_in->max_num; ii++)
    {
        tmp1 = &(table_in->table[ii]);
        tmp2 = (struct ctool_hash_element *)(tmp1->ptr_next);
        if((tmp1->data) != NULL && (table_in->free_fun != NULL))
        {
            table_in->free_fun(tmp1->data);
            tmp1->data = NULL;
        }
        tmp1 = tmp2;
        if(tmp2 != NULL)
        {
            tmp2 = (struct ctool_hash_element *)(tmp2->ptr_next);
        }

        while(tmp1 != NULL)
        {
            if((tmp1->data) != NULL && (table_in->free_fun != NULL))
            {
                table_in->free_fun(tmp1->data);
                tmp1->data = NULL;
            }
            free(tmp1);
            tmp1 = tmp2;
            if(tmp2 != NULL)
            {
                tmp2 = (struct ctool_hash_element *)(tmp2->ptr_next);
            }
        }
    }

    free(table_in->table);
    free(table_in);
    return 0;
}

extern int ctool_hashtable_insert(struct ctool_hash_table *table_in, void *para)
{
    unsigned int hash_tmp = table_in->hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);
    struct ctool_hash_element *tmp1 = NULL;
    if(table_in->table[index].data == NULL)
    {
        table_in->table[index].data = (void *)malloc(table_in->para_len);
        if(table_in->table[index].data == NULL)
        {
            return -1;
        }
        table_in->copy_fun(para, table_in->table[index].data);
        table_in->count++;
        return 0;
    }

    tmp1 = (struct ctool_hash_element *)malloc(sizeof(struct ctool_hash_element));
    if(tmp1 == NULL)
    {
        return -1;
    }
    tmp1->data = (void *)malloc(table_in->para_len);
    if(tmp1->data == NULL)
    {
        free(tmp1);
        return -1;
    }
    table_in->copy_fun(para, tmp1->data);
    tmp1->ptr_next = table_in->table[index].ptr_next;
    table_in->table[index].ptr_next = tmp1;
    table_in->count++;
    return 0;
}

extern void *ctool_hashtable_find(struct ctool_hash_table *table_in, void *para)
{
    unsigned int hash_tmp = table_in->hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);

    struct ctool_hash_element *tmp1 = (struct ctool_hash_element *)&(table_in->table[index]);
    while(tmp1 != NULL)
    {
        if(tmp1->data == NULL)
        {
            tmp1 = (struct ctool_hash_element *)(tmp1->ptr_next);
            continue;
        }
        if(table_in->compare_fun(para, tmp1->data) == 0)
        {
            return tmp1->data;
        }
        tmp1 = (struct ctool_hash_element *)(tmp1->ptr_next);
    }

    return NULL;
}

extern int ctool_hashtable_delete(struct ctool_hash_table *table_in, void *para)
{
    unsigned int hash_tmp = table_in->hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);

    struct ctool_hash_element *tmp1 = (struct ctool_hash_element *)&(table_in->table[index]);
    struct ctool_hash_element *tmp2 = (struct ctool_hash_element *)(tmp1->ptr_next);
    if(table_in->compare_fun(para, tmp1->data) == 0)
    {
        if((tmp1->data) != NULL && (table_in->free_fun != NULL))
        {
            table_in->free_fun(tmp1->data);
            tmp1->data = NULL;
        }

        if(tmp2 == NULL)
        {
            memset((void *)tmp1, 0, sizeof(struct ctool_hash_element));
        }
        else
        {
            memcpy((void *)tmp1, (void *)tmp2, sizeof(struct ctool_hash_element));
            free(tmp2);
        }
        return 0;
    }

    while(tmp2 != NULL)
    {
        if(tmp2->data == NULL)
        {
            tmp1 = tmp2;
            if(tmp2 != NULL)
            {
                tmp2 = (struct ctool_hash_element *)(tmp2->ptr_next);
            }
            continue;
        }
        if(table_in->compare_fun(para, tmp2->data) == 0)
        {
            if((tmp2->data) != NULL && (table_in->free_fun != NULL))
            {
                table_in->free_fun(tmp2->data);
                tmp2->data = NULL;
            }
            tmp1->ptr_next = tmp2->ptr_next;
            free(tmp2);
            return 0;
        }

        tmp1 = tmp2;
        if(tmp2 != NULL)
        {
            tmp2 = (struct ctool_hash_element *)(tmp2->ptr_next);
        }
    }

    return 0;
}

extern void ctool_hashtable_print(struct ctool_hash_table *table_in)
{
    int ii = 0;
    struct ctool_hash_element *tmp1 = NULL;

    if(table_in->print_fun == NULL)
    {
        return;
    }

    for(ii = 0; ii < table_in->max_num; ii++)
    {
        tmp1 = (struct ctool_hash_element *)&(table_in->table[ii]);
        while(tmp1 != NULL)
        {
            if(tmp1->data == NULL)
            {
                tmp1 = (struct ctool_hash_element *)(tmp1->ptr_next);
                continue;
            }
            table_in->print_fun(tmp1->data);
            tmp1 = (struct ctool_hash_element *)(tmp1->ptr_next);
        }
    }
    
    return;
}



extern struct ctool_circular_queue *ctool_circular_queue_init(int max_num, \
    int para_len, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    int (*copy_fun)(void *, void *), \
    int (*print_fun)(void *))
{
    struct ctool_circular_queue *cq_ret = NULL;
    
    if((max_num <= 0) || (para_len <= 0) || (compare_fun == NULL) || (copy_fun == NULL))
    {
        return NULL;
    }

    cq_ret = (struct ctool_circular_queue *)malloc(sizeof(struct ctool_circular_queue));
    if(cq_ret == NULL)
    {
        return NULL;
    }

    cq_ret->compare_fun = (int (*)(void *, void *))compare_fun;
    cq_ret->free_fun = (int (*)(void *))free_fun;
    cq_ret->copy_fun = (int (*)(void *, void *))copy_fun;
    cq_ret->print_fun = (int (*)(void *))print_fun;
    cq_ret->max_num = ctool_get_2n(max_num);
    cq_ret->para_len = para_len;
    cq_ret->front = 0;
    cq_ret->rear = 0;

    cq_ret->circular_queue = (struct ctool_cq_element *)malloc(cq_ret->max_num * sizeof(struct ctool_cq_element));
    if(cq_ret->circular_queue == NULL)
    {
        free(cq_ret);
        return NULL;
    }

    memset((void *)(cq_ret->circular_queue), 0, cq_ret->max_num * sizeof(struct ctool_cq_element));
    return cq_ret;
}

extern int ctool_circular_queue_destory(struct ctool_circular_queue *cq_in)
{
    int ii = cq_in->front;
    while(ii != cq_in->rear)
    {
        if(cq_in->free_fun != NULL)
        {
            cq_in->free_fun(cq_in->circular_queue[ii].data);
            cq_in->circular_queue[ii].data = NULL;
        }
        else
        {
            free(cq_in->circular_queue[ii].data);
            cq_in->circular_queue[ii].data = NULL;
        }
        ii = (ii + 1) & (cq_in->max_num - 1);
    }
    free(cq_in->circular_queue);
    free(cq_in);
    return 0;
}

extern int ctool_circular_queue_clear(struct ctool_circular_queue *cq_in)
{
    int ii = cq_in->front;
    while(ii != cq_in->rear)
    {
        if(cq_in->free_fun != NULL)
        {
            cq_in->free_fun(cq_in->circular_queue[ii].data);
            cq_in->circular_queue[ii].data = NULL;
        }
        else
        {
            free(cq_in->circular_queue[ii].data);
            cq_in->circular_queue[ii].data = NULL;
        }
        ii = (ii + 1) & (cq_in->max_num - 1);
    }
    memset((void *)(cq_in->circular_queue), 0, cq_in->max_num * sizeof(struct ctool_cq_element));
    cq_in->front = 0;
    cq_in->rear = 0;
    return 0;
}

extern int ctool_circular_queue_is_empty(struct ctool_circular_queue *cq_in)
{
    if(cq_in->front == cq_in->rear)
    {
        return 1;
    }
    return 0;
}

extern int ctool_circular_queue_is_full(struct ctool_circular_queue *cq_in)
{
    if(((cq_in->rear + 1) & (cq_in->max_num - 1)) == cq_in->front)
    {
        return 1;
    }
    return 0;
}

extern int ctool_circular_queue_get_num(struct ctool_circular_queue *cq_in)
{
    return (cq_in->rear - cq_in->front + cq_in->max_num) & (cq_in->max_num - 1);
}

extern int ctool_circular_queue_is_contain(struct ctool_circular_queue *cq_in, void *para)
{
    int ii = cq_in->front;
    while(ii != cq_in->rear)
    {
        if(cq_in->compare_fun(para, cq_in->circular_queue[ii].data) == 0)
        {
            return 1;
        }
        ii = (ii + 1) & (cq_in->max_num - 1);
    }
    return 0;
}

extern int ctool_circular_queue_pushback(struct ctool_circular_queue *cq_in, void *para)
{
    if(((cq_in->rear + 1) & (cq_in->max_num - 1)) == cq_in->front)
    {
        return -1;
    }

    cq_in->circular_queue[cq_in->rear].data = (void *)malloc(cq_in->para_len);
    if(cq_in->circular_queue[cq_in->rear].data == NULL)
    {
        return -1;
    }
    cq_in->copy_fun(para, cq_in->circular_queue[cq_in->rear].data);
    cq_in->rear = (cq_in->rear + 1) & (cq_in->max_num - 1);
    return 0;
}

extern int ctool_circular_queue_popfront(struct ctool_circular_queue *cq_in, void *para)
{
    if(cq_in->front == cq_in->rear)
    {
        return -1;
    }

    cq_in->copy_fun(cq_in->circular_queue[cq_in->front].data, para);
    if(cq_in->free_fun != NULL)
    {
        cq_in->free_fun(cq_in->circular_queue[cq_in->front].data);
        cq_in->circular_queue[cq_in->front].data = NULL;
    }
    else
    {
        free(cq_in->circular_queue[cq_in->front].data);
        cq_in->circular_queue[cq_in->front].data = NULL;
    }
    cq_in->front = (cq_in->front + 1) & (cq_in->max_num - 1);
    return 0;
}





unsigned short ctool_ip_cksum(unsigned short *addr, int len)
{
    unsigned short cksum;
    unsigned int sum = 0;

    while(len > 1)
    {
        sum += *addr++;
        len -= 2;
    }
    if(len == 1)
    {
        sum += *(unsigned char*)addr;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    cksum = ~sum;
    return (cksum);
}








//merge low~mid,mid+1~high
//array2 is tmp array,size at least (high - low + 1) * sizeof(ctool_merge_type)
static void ctool_merge(ctool_merge_type *array, ctool_merge_type *array2, int low, int mid, int high, int mode)
{
    int i = low; // i is index of first array
    int j = mid + 1; // j is index of second array
    int k = 0; // k index of tmp array

    // scan the first and second sequences until one scan ends
    if(mode == 0)
    {
        while(i <= mid && j <= high)
        {
            // Determine which of the first and second segments is smaller, store them in the merge sequence, and continue to scan down
            if(array[i] <= array[j])
            {
                array2[k] = array[i];
                i++;
                k++;
            }
            else
            {
                array2[k] = array[j];
                j++;
                k++;
            }
        }
    }
    else
    {
        while(i <= mid && j <= high)
        {
            // Determine which of the first and second segments is bigger, store them in the merge sequence, and continue to scan down
            if(array[i] >= array[j])
            {
                array2[k] = array[i];
                i++;
                k++;
            }
            else
            {
                array2[k] = array[j];
                j++;
                k++;
            }
        }
    }

    // If the first sequence has not been scanned, copy it all to the merged sequence
    while(i <= mid)
    {
        array2[k] = array[i];
        i++;
        k++;
    }

    // If the second sequence has not been scanned, copy it all to the merged sequence.
    while(j <= high)
    {
        array2[k] = array[j];
        j++;
        k++;
    }

    // Copy the merged sequence to the original sequence
    for(k = 0, i = low; i <= high; i++, k++)
    {
        array[i] = array2[k];
    }

}

//Complete a round of mergers
//array2 is a temporary merge sequence, just take the size of the original array directly
static void ctool_merge_traverse(ctool_merge_type *array, ctool_merge_type *array2, int gap, int length, int mode)
{
    int i = 0;
    // Merging two adjacent subtables of gap length
    for(i = 0; i + 2 * gap - 1 < length; i = i + 2 * gap)
    {
        ctool_merge(array, array2, i, i + gap - 1, i + 2 * gap - 1, mode);
    }

    // The remaining two subtables, the latter less than gap in length
    if(i + gap - 1 < length)
    {
        ctool_merge(array, array2, i, i + gap - 1, length - 1, mode);
    }
}

extern int ctool_merge_sort(ctool_merge_type *array, int length, int mode)
{
    int gap = 0;
    ctool_merge_type *array2 = NULL;
    if(length <= 0)
    {
        return 0;
    }
//    int ii = 0;
    array2 = (ctool_merge_type *)malloc(sizeof(ctool_merge_type) * length);
    if(array2 == NULL)
    {
        return -1;
    }
    for(gap = 1; gap < length; gap = 2 * gap)
    {
        ctool_merge_traverse(array, array2, gap, length, mode);
//        printf("gap = %d:\t", gap);
//        for (ii = 0; ii < length; ii++)
//        {
//            printf("%d ", array[ii]);
//        }
//        printf("\n");
    }
    free(array2);
    return 0;
}




static void ctool_general_merge(struct ctool_general_merge_struct *ctool_gms_p, void *array2, int low, int mid, int high)
{
    int ii = low;
    int jj = mid + 1;
    int kk = 0;

    while(ii <= mid && jj <= high)
    {
        if(ctool_gms_p->compare_fun(((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + jj * ctool_gms_p->para_len) <= 0)
        {
            if(ctool_gms_p->copy_fun == NULL)
            {
                memcpy(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len, ctool_gms_p->para_len);
            }
            else
            {
                ctool_gms_p->copy_fun(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len);
            }
            ii++;
            kk++;
        }
        else
        {
            if(ctool_gms_p->copy_fun == NULL)
            {
                memcpy(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + jj * ctool_gms_p->para_len, ctool_gms_p->para_len);
            }
            else
            {
                ctool_gms_p->copy_fun(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + jj * ctool_gms_p->para_len);
            }
            jj++;
            kk++;
        }
    }

    while(ii <= mid)
    {
        if(ctool_gms_p->copy_fun == NULL)
        {
            memcpy(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len, ctool_gms_p->para_len);
        }
        else
        {
            ctool_gms_p->copy_fun(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len);
        }
        ii++;
        kk++;
    }

    while(jj <= high)
    {
        if(ctool_gms_p->copy_fun == NULL)
        {
            memcpy(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + jj * ctool_gms_p->para_len, ctool_gms_p->para_len);
        }
        else
        {
            ctool_gms_p->copy_fun(((char *)array2) + kk * ctool_gms_p->para_len, ((char *)(ctool_gms_p->array)) + jj * ctool_gms_p->para_len);
        }
        jj++;
        kk++;
    }

    for(kk = 0, ii = low; ii <= high; ii++, kk++)
    {
        if(ctool_gms_p->free_fun == NULL)
        {
            ;
        }
        else
        {
            ctool_gms_p->free_fun(((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len);
        }
        
        if(ctool_gms_p->copy_fun == NULL)
        {
            memcpy(((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len, ((char *)array2) + kk * ctool_gms_p->para_len, ctool_gms_p->para_len);
        }
        else
        {
            ctool_gms_p->copy_fun(((char *)(ctool_gms_p->array)) + ii * ctool_gms_p->para_len, ((char *)array2) + kk * ctool_gms_p->para_len);
        }
        
        if(ctool_gms_p->free_fun == NULL)
        {
            ;
        }
        else
        {
            ctool_gms_p->free_fun(((char *)array2) + kk * ctool_gms_p->para_len);
        }
    }
}

static void ctool_general_merge_traverse(struct ctool_general_merge_struct *ctool_gms_p, void *array2, int gap, int length)
{
    int ii = 0;

    for(ii = 0; ii + 2 * gap - 1 < length; ii = ii + 2 * gap)
    {
        ctool_general_merge(ctool_gms_p, array2, ii, ii + gap - 1, ii + 2 * gap - 1);
    }

    if(ii + gap - 1 < length)
    {
        ctool_general_merge(ctool_gms_p, array2, ii, ii + gap - 1, length - 1);
    }
}

static int ctool_general_merge_sort_local(struct ctool_general_merge_struct *ctool_gms_p)
{
    int gap = 0;
    void *array2 = NULL;
    if(ctool_gms_p->para_num <= 0)
    {
        return 0;
    }
//    int ii = 0;
    array2 = (void *)malloc(ctool_gms_p->para_len * ctool_gms_p->para_num);
    if(array2 == NULL)
    {
        return -1;
    }
    memset(array2, 0, ctool_gms_p->para_len * ctool_gms_p->para_num);
    
    for(gap = 1; gap < ctool_gms_p->para_num; gap = 2 * gap)
    {
        ctool_general_merge_traverse(ctool_gms_p, array2, gap, ctool_gms_p->para_num);
//        printf("gap = %d:\t", gap);
//        for (ii = 0; ii < length; ii++)
//        {
//            printf("%d ", array[ii]);
//        }
//        printf("\n");
    }

    free(array2);
    return 0;
}

extern int ctool_general_merge_sort(void *array, \
    int para_len, \
    int para_num, \
    int (*compare_fun)(void *, void *), \
    int (*free_fun)(void *), \
    int (*copy_fun)(void *, void *))
{
    struct ctool_general_merge_struct ctool_gms;
    int ret = 0;
    
    if((array == NULL) || (para_len <= 0) || (compare_fun == NULL))
    {
        return -1;
    }
    
    memset(&ctool_gms, 0, sizeof(struct ctool_general_merge_struct));
    ctool_gms.array = array;
    ctool_gms.para_len = para_len;
    ctool_gms.para_num = para_num;
    ctool_gms.compare_fun = compare_fun;
    ctool_gms.free_fun = free_fun;
    ctool_gms.copy_fun = copy_fun;
    
    ret = ctool_general_merge_sort_local(&ctool_gms);
    
    return ret;
}




extern int get_file_size(char *path)
{
    FILE *fp = NULL;
    int flen = 0;
    fp = fopen(path, "r+");
    if(fp == NULL)
    {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);
    if(flen <= 0)
    {
        flen = 0;
    }
    fclose(fp);

    return flen;
}


extern int read_file_content(char *path, char *buf, int len)
{
    FILE *fp = NULL;
    int ret = 0;
    fp = fopen(path, "r+");
    if(fp == NULL)
    {
        return -1;
    }

    ret = fread(buf, len, 1, fp);

    fclose(fp);

    return ret;
}


extern int write_file_content(char *path, char *buf, int len)
{
    FILE *fp = NULL;
    int ret = 0;
    fp = fopen(path, "w+b");
    if(fp == NULL)
    {
        return -1;
    }

    ret = fwrite(buf, len, 1, fp);

    fclose(fp);

    return ret;
}