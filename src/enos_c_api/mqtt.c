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

#include "enos_c_api/enos_encrypt_api.h"
#include "enos_c_api/enos_mqtt_api.h"
#include "common_tool.h"

//get child of json object
static int get_child_obj_num(cJSON *cjson_ptr);

//get child of json object
static int get_child_obj_num(cJSON *cjson_ptr)
{
	int num = 0;
	cJSON *cjson_ptr1 = cjson_ptr->child;
	while(cjson_ptr1 != NULL)
	{
		num++;
		cjson_ptr1 = cjson_ptr1->next;
	}

	return num;
}

//min 2^n >= value
static int custom_get_2n(int value);
//hash
static unsigned int custom_hash_fun(custom_hash_type *value);
//init
static struct custom_hash_table *custom_hashtable_init(int max_num);
//destory
static int custom_hashtable_destory(struct custom_hash_table *table_in);
//delete timeout elements
static int custom_hashtable_del_overtime(struct custom_hash_table *table_in, long long int sec_now);
//insert
static int custom_hashtable_insert(struct custom_hash_table *table_in, custom_hash_type *para, long long int sec);
//search
static struct custom_hash_element *custom_hashtable_search(struct custom_hash_table *table_in, custom_hash_type *para);
//search,insert and return the inserted element if it does not exist, and return the search result if it does,is_exist==0 element does not exist before, ==1 element exist before
static struct custom_hash_element *custom_hashtable_search_and_insert(struct custom_hash_table *table_in, custom_hash_type *para, long long int sec, int *is_exist);

//min 2^n >= value
static int custom_get_2n(int value)
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

static unsigned int custom_hash_fun(custom_hash_type *value)
{
    unsigned int hash = (value->token & 0xffffffff);

    return hash;
}

static struct custom_hash_table *custom_hashtable_init(int max_num)
{
    if((max_num <= 0) || (max_num > 268435456))
    {
        return NULL;
    }

    struct custom_hash_table *table_ret = (struct custom_hash_table *)malloc(sizeof(struct custom_hash_table));
    if(table_ret == NULL)
    {
        return NULL;
    }
    memset(table_ret, 0, sizeof(struct custom_hash_table));

    table_ret->max_num = custom_get_2n(max_num);

    table_ret->table = (struct custom_hash_element *)malloc(table_ret->max_num * sizeof(struct custom_hash_element));
    if(table_ret->table == NULL)
    {
        free(table_ret);
        return NULL;
    }

    memset((void *)(table_ret->table), 0, table_ret->max_num * sizeof(struct custom_hash_element));
    return table_ret;
}

static int custom_hashtable_destory(struct custom_hash_table *table_in)
{
    if(table_in == NULL)
    {
        return 0;
    }
    int ii = 0;
    struct custom_hash_element *tmp1;
    struct custom_hash_element *tmp2;
    for(ii = 0; ii < table_in->max_num; ii++)
    {
        tmp1 = &(table_in->table[ii]);
        tmp2 = (struct custom_hash_element *)(tmp1->ptr_next);
        tmp1 = tmp2;
        if(tmp2 != NULL)
        {
            tmp2 = (struct custom_hash_element *)(tmp2->ptr_next);
        }

        while(tmp1 != NULL)
        {
            free(tmp1);
            tmp1 = tmp2;
            if(tmp2 != NULL)
            {
                tmp2 = (struct custom_hash_element *)(tmp2->ptr_next);
            }
        }
    }

    free(table_in->table);
    free(table_in);
    return 0;
}

static int custom_hashtable_del_overtime(struct custom_hash_table *table_in, long long int sec_now)
{
    if(table_in == NULL)
    {
        return 0;
    }

    struct custom_hash_element *tmp1;
    struct custom_hash_element *tmp2;
    struct custom_hash_element *tmp3;
    struct custom_hash_element *tmp4;
    long long int sec_temp = 0;
    while(1)
    {
        if(table_in->time_first == NULL)
        {
            break;
        }

        tmp1 = table_in->time_first;
        tmp2 = table_in->time_first->time_next;

        sec_temp = tmp1->sec - sec_now;
        if(sec_temp < 0)
        {
            sec_temp *= -1;
        }
        if(sec_temp > ENOS_DEFAULT_WAIT_DELIVERY_HASH_TIMEOUT)
        {
            if(tmp1->ptr_pre == NULL)
            {
				tmp1->time_pre = NULL;
                tmp1->time_next = NULL;
                tmp1->sec = 0;
				memset(&(tmp1->value), 0, sizeof(custom_hash_type));
                tmp1->valid_flag = 0;
				if(tmp2 != NULL)
				{
					tmp2->time_pre = NULL;
				}
                table_in->time_first = tmp2;
                if(table_in->time_last == tmp1)
                {
                    table_in->time_last = NULL;
                }
            }
            else
            {
                tmp3 = tmp1->ptr_pre;
                tmp4 = tmp1->ptr_next;
                tmp3->ptr_next = tmp4;
                if(tmp4 != NULL)
                {
                    tmp4->ptr_pre = tmp3;
                }
				if(tmp2 != NULL)
				{
					tmp2->time_pre = NULL;
				}
                table_in->time_first = tmp2;
                if(table_in->time_last == tmp1)
                {
                    table_in->time_last = NULL;
                }
                free(tmp1);
            }
            table_in->count--;
        }
        else
        {
            break;
        }
    }

    return 0;
}

static int custom_hashtable_insert(struct custom_hash_table *table_in, custom_hash_type *para, long long int sec)
{
    if(table_in == NULL)
    {
        return -1;
    }
    unsigned int hash_tmp = custom_hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);
    if(table_in->table[index].valid_flag == 0)
    {
        table_in->table[index].valid_flag = 1;
		memcpy(&(table_in->table[index].value), para, sizeof(custom_hash_type));
        table_in->table[index].sec = sec;
        table_in->count++;
        if(table_in->time_first == NULL)
        {
            table_in->time_first = &(table_in->table[index]);
            table_in->time_last = &(table_in->table[index]);
			table_in->table[index].time_next = NULL;
			table_in->table[index].time_pre = NULL;
        }
        else
        {
            if(table_in->time_last != NULL)
            {
                table_in->time_last->time_next = &(table_in->table[index]);
				table_in->table[index].time_pre = table_in->time_last;
                table_in->time_last = &(table_in->table[index]);
            }
        }
        return 0;
    }

    struct custom_hash_element *tmp1 = (struct custom_hash_element *)malloc(sizeof(struct custom_hash_element));
    if(tmp1 == NULL)
    {
        return -1;
    }
    memset(tmp1, 0, sizeof(struct custom_hash_element));

    tmp1->valid_flag = 1;
	memcpy(&(tmp1->value), para, sizeof(custom_hash_type));
    tmp1->sec = sec;
    table_in->count++;
    tmp1->ptr_pre = &(table_in->table[index]);
    tmp1->ptr_next = table_in->table[index].ptr_next;
    if(table_in->table[index].ptr_next != NULL)
    {
        table_in->table[index].ptr_next->ptr_pre = tmp1;
    }
    table_in->table[index].ptr_next = tmp1;

    if(table_in->time_first == NULL)
    {
        table_in->time_first = tmp1;
        table_in->time_last = tmp1;
		tmp1->time_next = NULL;
		tmp1->time_pre = NULL;
    }
    else
    {
        if(table_in->time_last != NULL)
        {
            table_in->time_last->time_next = tmp1;
			tmp1->time_pre = table_in->time_last;
            table_in->time_last = tmp1;
        }
    }
    return 0;
}

static struct custom_hash_element *custom_hashtable_search(struct custom_hash_table *table_in, custom_hash_type *para)
{
    if(table_in == NULL)
    {
        return NULL;
    }
    
    unsigned int hash_tmp = custom_hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);

    struct custom_hash_element *tmp1 = (struct custom_hash_element *)&(table_in->table[index]);
    while(tmp1 != NULL)
    {
        if(tmp1->valid_flag == 0)
        {
            tmp1 = (struct custom_hash_element *)(tmp1->ptr_next);
            continue;
        }
		if(tmp1->value.token == para->token)
        {
            return tmp1;
        }
        tmp1 = (struct custom_hash_element *)(tmp1->ptr_next);
    }

    return NULL;
}

static struct custom_hash_element *custom_hashtable_search_and_insert(struct custom_hash_table *table_in, custom_hash_type *para, long long int sec, int *is_exist)
{
    if(table_in == NULL)
    {
        return NULL;
    }
    
    unsigned int hash_tmp = custom_hash_fun(para);
    unsigned int index = hash_tmp & (table_in->max_num -1);

    struct custom_hash_element *tmp1 = (struct custom_hash_element *)&(table_in->table[index]);
    while(tmp1 != NULL)
    {
        if(tmp1->valid_flag == 0)
        {
            tmp1 = (struct custom_hash_element *)(tmp1->ptr_next);
            continue;
        }
		if(tmp1->value.token == para->token)
        {
            *is_exist = 1;
            return tmp1;
        }
        tmp1 = (struct custom_hash_element *)(tmp1->ptr_next);
    }

    if(table_in->table[index].valid_flag == 0)
    {
        table_in->table[index].valid_flag = 1;
		memcpy(&(table_in->table[index].value), para, sizeof(custom_hash_type));
        table_in->table[index].sec = sec;
        table_in->count++;
        if(table_in->time_first == NULL)
        {
            table_in->time_first = &(table_in->table[index]);
            table_in->time_last = &(table_in->table[index]);
			table_in->table[index].time_next = NULL;
			table_in->table[index].time_pre = NULL;
        }
        else
        {
            if(table_in->time_last != NULL)
            {
                table_in->time_last->time_next = &(table_in->table[index]);
				table_in->table[index].time_pre = table_in->time_last;
                table_in->time_last = &(table_in->table[index]);
            }
        }
        *is_exist = 0;
        return &(table_in->table[index]);
    }

    tmp1 = (struct custom_hash_element *)malloc(sizeof(struct custom_hash_element));
    if(tmp1 == NULL)
    {
        *is_exist = 0;
        return NULL;
    }
    memset(tmp1, 0, sizeof(struct custom_hash_element));

    tmp1->valid_flag = 1;
    memcpy(&(tmp1->value), para, sizeof(custom_hash_type));
    tmp1->sec = sec;
    table_in->count++;
    tmp1->ptr_pre = &(table_in->table[index]);
    tmp1->ptr_next = table_in->table[index].ptr_next;
    if(table_in->table[index].ptr_next != NULL)
    {
        table_in->table[index].ptr_next->ptr_pre = tmp1;
    }
    table_in->table[index].ptr_next = tmp1;

    if(table_in->time_first == NULL)
    {
        table_in->time_first = tmp1;
        table_in->time_last = tmp1;
		tmp1->time_next = NULL;
		tmp1->time_pre = NULL;
    }
    else
    {
        if(table_in->time_last != NULL)
        {
            table_in->time_last->time_next = tmp1;
			tmp1->time_pre = table_in->time_last;
            table_in->time_last = tmp1;
        }
    }

    *is_exist = 0;
    return tmp1;
}

//delete
static int custom_hashtable_delete(struct custom_hash_table *table_in, struct custom_hash_element *para)
{
	if(para == NULL)
	{
		return 0;
	}
	
	if(table_in == NULL)
    {
        return -1;
    }

	struct custom_hash_element *tmp1;
    struct custom_hash_element *tmp2;
    struct custom_hash_element *tmp3;
    struct custom_hash_element *tmp4;

	tmp1 = para->time_pre;
	tmp2 = para->time_next;

	if(para->ptr_pre == NULL)
	{
		if(tmp1 != NULL)
		{
			tmp1->time_next = tmp2;
		}
		if(tmp2 != NULL)
		{
			tmp2->time_pre = tmp1;
		}
		para->time_pre = NULL;
        para->time_next = NULL;
        para->sec = 0;
		memset(&(para->value), 0, sizeof(custom_hash_type));
        para->valid_flag = 0;
		if(table_in->time_first == para)
		{
			table_in->time_first = tmp2;
		}
        
        if(table_in->time_last == para)
        {
            table_in->time_last = tmp1;
        }
	}
	else
	{
		tmp3 = para->ptr_pre;
        tmp4 = para->ptr_next;
        tmp3->ptr_next = tmp4;
        if(tmp4 != NULL)
        {
            tmp4->ptr_pre = tmp3;
        }

		if(tmp1 != NULL)
		{
			tmp1->time_next = tmp2;
		}
		if(tmp2 != NULL)
		{
			tmp2->time_pre = tmp1;
		}
		
		if(table_in->time_first == para)
		{
			table_in->time_first = tmp2;
		}
        
        if(table_in->time_last == para)
        {
            table_in->time_last = tmp1;
        }
		
        free(para);
	}
	table_in->count--;
	
	return 0;
}





//thread of deal wait_delivery_hash timeout
static void *enos_deal_wait_delivery_hash_thread(void *arg)
{
//    pthread_detach(pthread_self());
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)arg;
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]enos_mas_p == NULL error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    
    struct timeval tv_now;
    while(1)
    {
        pthread_testcancel();
        memset(&tv_now, 0, sizeof(struct timeval));
        enos_gettimeofday(&tv_now, NULL);
        pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
        
        custom_hashtable_del_overtime(enos_mas_p->wait_delivery_hash, (long long int)(tv_now.tv_sec));
        
        pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
        enos_sleep(5000);
    }
    
    return NULL;
}

//compare function of struct enos_para_value
static int enos_para_value_compare_fun(void *para1, void *para2)
{
    char *tmp1 = (char *)(((struct enos_para_value *)para1)->para_name);
    char *tmp2 = (char *)(((struct enos_para_value *)para2)->para_name);
    return (strcmp(tmp1, tmp2));
}

//callback of delivered
static void enos_delivered(void *context, MQTTClient_deliveryToken dt)
{
    enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]Message with token value %d delivery confirmed(file=%s, function=%s, line=%d)\n", dt, __FILE__, __FUNCTION__, __LINE__);
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)context;
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]enos_mas_p == NULL error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    enos_delivered_callback dl_callback;
    void *dl_callback_user = NULL;

    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    dl_callback = (enos_delivered_callback)(enos_mas_p->delivered_callback);
    dl_callback_user = enos_mas_p->delivered_callback_user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));

    if(dl_callback != NULL)
    {
        dl_callback(dl_callback_user, dt);
    }
    
    struct custom_hash_element *ele_ptr = NULL;
    custom_hash_type para;
    memset(&para, 0, sizeof(custom_hash_type));
    para.token = dt;
    
    pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
    ele_ptr = custom_hashtable_search(enos_mas_p->wait_delivery_hash, &para);
    if(ele_ptr == NULL)
    {
        ;
    }
    else
    {
        custom_hashtable_delete(enos_mas_p->wait_delivery_hash, ele_ptr);
    }
    pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
    
    return;
}

//callback of message arrived
static int enos_message_arrived(void *context, char *topic_name, int topic_len, MQTTClient_message *message)
{
    if(ENOS_CURRENT_LOG_LEVEL >= ENOS_LOG_DEBUG)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]Message arrived,topic=%s,msg=\n");
        char *msg_local = malloc(message->payloadlen + 1);
        if(msg_local == NULL)
        {
            ;
        }
        else
        {
            memset(msg_local, 0, message->payloadlen + 1);
            memcpy(msg_local, message->payload, message->payloadlen);
            enos_printf(NULL, ENOS_LOG_DEBUG, "%s\n\n", msg_local);
            free(msg_local);
        }
    }
    
    
//    int topic_len_local = topic_len;
//    if(topic_len_local <= 0)
//    {
//        topic_len_local = strlen(topic_name);
//    }

    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)context;
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]enos_mas_p == NULL error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        MQTTClient_freeMessage(&message);
        MQTTClient_free(topic_name);
        return 1;
    }
    
    enos_message_arrived_callback ma_callback;
    void *ma_callback_user = NULL;

    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    ma_callback = (enos_message_arrived_callback)(enos_mas_p->message_arrived_callback);
    ma_callback_user = enos_mas_p->message_arrived_callback_user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));

    if(ma_callback != NULL)
    {
        ma_callback(ma_callback_user, topic_name, topic_len, message);
    }
    
    struct enos_mqtt_api_info_ex info_ex;
    memset(&info_ex, 0, sizeof(struct enos_mqtt_api_info_ex));
    info_ex.topic_len = topic_len;
    info_ex.topic_name = topic_name;
    
    if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_REGISTER) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Device Registration
        enos_sub_dev_register_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_register_callback)(enos_mas_p->sub_dev_register_callback);
        callback_user = enos_mas_p->sub_dev_register_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_ADD_TOPO) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Add Topological Relationships of Sub-devices
        enos_sub_dev_add_topo_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_add_topo_callback)(enos_mas_p->sub_dev_add_topo_callback);
        callback_user = enos_mas_p->sub_dev_add_topo_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DEL_TOPO) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Delete Topological Relationships of Sub-devices
        enos_sub_dev_del_topo_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_del_topo_callback)(enos_mas_p->sub_dev_del_topo_callback);
        callback_user = enos_mas_p->sub_dev_del_topo_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_GET_TOPO) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Get Topological Relationships of Sub-devices
        enos_sub_dev_get_topo_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_get_topo_callback)(enos_mas_p->sub_dev_get_topo_callback);
        callback_user = enos_mas_p->sub_dev_get_topo_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_TAG_UPDATE) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Report Tags
        enos_dev_tag_update_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_tag_update_callback)(enos_mas_p->dev_tag_update_callback);
        callback_user = enos_mas_p->dev_tag_update_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_TAG_DELETE) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Delete Tags
        enos_dev_tag_delete_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_tag_delete_callback)(enos_mas_p->dev_tag_delete_callback);
        callback_user = enos_mas_p->dev_tag_delete_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_TAG_QUERY) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Query Tags
        enos_dev_tag_query_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_tag_query_callback)(enos_mas_p->dev_tag_query_callback);
        callback_user = enos_mas_p->dev_tag_query_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_QUERY) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Get Attributes
        enos_dev_attr_query_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_attr_query_callback)(enos_mas_p->dev_attr_query_callback);
        callback_user = enos_mas_p->dev_attr_query_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_UPDATE) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Report Attributes
        enos_dev_attr_update_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_attr_update_callback)(enos_mas_p->dev_attr_update_callback);
        callback_user = enos_mas_p->dev_attr_update_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_DELETE) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Delete Attributes
        enos_dev_attr_delete_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_attr_delete_callback)(enos_mas_p->dev_attr_delete_callback);
        callback_user = enos_mas_p->dev_attr_delete_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_POINT_POST) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Report Device Measuring Points
        enos_dev_point_post_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_point_post_callback)(enos_mas_p->dev_point_post_callback);
        callback_user = enos_mas_p->dev_point_post_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST) != NULL) && (strstr(topic_name, ENOS_TOPIC_SUFFIX2_DEVICE_EVENT_POST) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Report Device Events(Non-Passthrough)
        enos_dev_event_post_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_event_post_callback)(enos_mas_p->dev_event_post_callback);
        callback_user = enos_mas_p->dev_event_post_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_EVENT_POST_RAW) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Report Device Events(Passthrough)
        enos_dev_event_post_raw_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_event_post_raw_callback)(enos_mas_p->dev_event_post_raw_callback);
        callback_user = enos_mas_p->dev_event_post_raw_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_GET_TSL) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//get tsl model
        enos_dev_get_tsl_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_get_tsl_callback)(enos_mas_p->dev_get_tsl_callback);
        callback_user = enos_mas_p->dev_get_tsl_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGIN) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Connect Sub-devices to EnOS Cloud
        enos_sub_dev_login_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_login_callback)(enos_mas_p->sub_dev_login_callback);
        callback_user = enos_mas_p->sub_dev_login_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if((strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGOUT) != NULL) && (strstr(topic_name, ENOS_TOPIC_REPLY1) != NULL))
    {//Disconnect Sub-devices from EnOS Cloud
        enos_sub_dev_logout_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_logout_callback)(enos_mas_p->sub_dev_logout_callback);
        callback_user = enos_mas_p->sub_dev_logout_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_POINT_SET) != NULL)
    {//Set Device Measuring Points(downstream)
        enos_dev_point_set_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_point_set_callback)(enos_mas_p->dev_point_set_callback);
        callback_user = enos_mas_p->dev_point_set_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_POINT_GET) != NULL)
    {//Get Device Measuring Points(downstream)
        enos_dev_point_get_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_point_get_callback)(enos_mas_p->dev_point_get_callback);
        callback_user = enos_mas_p->dev_point_get_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE) != NULL)
    {//Invoke Device Services (Non-Passthrough)(downstream)
        enos_dev_service_invoke_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_service_invoke_callback)(enos_mas_p->dev_service_invoke_callback);
        callback_user = enos_mas_p->dev_service_invoke_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE_RAW) != NULL)
    {//Invoke Device Services(Passthrough)(downstream)
        enos_dev_service_invoke_raw_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_service_invoke_raw_callback)(enos_mas_p->dev_service_invoke_raw_callback);
        callback_user = enos_mas_p->dev_service_invoke_raw_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_DISABLE) != NULL)
    {//Disable devices(downstream)
        enos_dev_disable_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_disable_callback)(enos_mas_p->dev_disable_callback);
        callback_user = enos_mas_p->dev_disable_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_ENABLE) != NULL)
    {//Enable devices(downstream)
        enos_dev_enable_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_enable_callback)(enos_mas_p->dev_enable_callback);
        callback_user = enos_mas_p->dev_enable_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_DELETE) != NULL)
    {//Delete devices(downstream)
        enos_dev_delete_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_dev_delete_callback)(enos_mas_p->dev_delete_callback);
        callback_user = enos_mas_p->dev_delete_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DISABLE) != NULL)
    {//Disable Sub-devices(downstream)
        enos_sub_dev_disable_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_disable_callback)(enos_mas_p->sub_dev_disable_callback);
        callback_user = enos_mas_p->sub_dev_disable_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_ENABLE) != NULL)
    {//Enable Sub-devices(downstream)
        enos_sub_dev_enable_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_enable_callback)(enos_mas_p->sub_dev_enable_callback);
        callback_user = enos_mas_p->sub_dev_enable_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else if(strstr(topic_name, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DELETE) != NULL)
    {//Delete Sub-devices(downstream)
        enos_sub_dev_delete_callback callback;
        void *callback_user = NULL;

        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        callback = (enos_sub_dev_delete_callback)(enos_mas_p->sub_dev_delete_callback);
        callback_user = enos_mas_p->sub_dev_delete_callback_user;
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
        if(callback != NULL)
        {
            callback(callback_user, message->payload, message->payloadlen, &info_ex);
        }
    }
    else
    {
        ;
    }
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);
    return 1;
}

//callback of connect lost
static void enos_connlost(void *context, char *cause)
{
    enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]connect lost\n");

    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)context;
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]enos_mas_p == NULL error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    enos_connect_lost_callback callback;
    void *callback_user = NULL;

    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    callback = (enos_connect_lost_callback)(enos_mas_p->connect_lost_callback);
    callback_user = enos_mas_p->connect_lost_callback_user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));

    if(callback != NULL)
    {
        callback(callback_user, cause);
    }

    MQTTClient_free(cause);
}

extern int enos_calc_sign(struct enos_para_value *para_value_p, int count, char *device_secret, char *sign, int sign_max, int *sign_len)
{
    if((para_value_p == NULL) || (sign == NULL) || (sign_max <= 0) || (sign_len == NULL) || (device_secret == NULL) || (count <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    struct enos_para_value *para_value_p_tmp = NULL;
    para_value_p_tmp = (struct enos_para_value *)malloc(sizeof(struct enos_para_value) * count);
    if(para_value_p_tmp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para_value_p_tmp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memcpy(para_value_p_tmp, para_value_p, sizeof(struct enos_para_value) * count);
    
    int ret = 0;
    ret = ctool_general_merge_sort((void *)para_value_p_tmp, \
            (int)(sizeof(struct enos_para_value)), \
            count, \
            enos_para_value_compare_fun, \
            NULL, \
            NULL);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ctool_general_merge_sort error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        return -1;
    }
    
    int sign_src_buf_max = sizeof(struct enos_para_value) * count + strlen(device_secret) + 10;
    char *sign_src_buf = (char *)malloc(sign_src_buf_max);
    if(sign_src_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]sign_src_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        return -1;
    }
    memset(sign_src_buf, 0, sign_src_buf_max);
    
    int ii = 0;
    char *sign_src_buf_ptr = sign_src_buf;
    int current_len = 0;
    for(ii = 0; ii < count; ii++)
    {
        memcpy(sign_src_buf_ptr, para_value_p_tmp[ii].para_name, strlen(para_value_p_tmp[ii].para_name));
        sign_src_buf_ptr += strlen(para_value_p_tmp[ii].para_name);
        memcpy(sign_src_buf_ptr, para_value_p_tmp[ii].para_value, strlen(para_value_p_tmp[ii].para_value));
        sign_src_buf_ptr += strlen(para_value_p_tmp[ii].para_value);
    }
    memcpy(sign_src_buf_ptr, device_secret, strlen(device_secret));
    sign_src_buf_ptr += strlen(device_secret);
    
    current_len = (int)(sign_src_buf_ptr - sign_src_buf);
    
    char sign1[256];
    int sign_max1 = (int)(sizeof(sign1));
    int sign_len1 = 0;
    memset(sign1, 0, sign_max1);
    ret = enos_hash(sign_src_buf, current_len, "sha1", sign1, sign_max1, &sign_len1);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_hash error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        free(sign_src_buf);
        return -1;
    }
    
    if(sign_max <= sign_len1 * 2)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]sign_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", sign_max, sign_len1 * 2 + 1, __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        free(sign_src_buf);
        return -1;
    }
    
    char *sign_ptr = sign;
    for(ii = 0; ii < sign_len1; ii++)
    {
        snprintf(sign_ptr, sign_max - (sign_ptr - sign), "%02hhX", (unsigned char)(sign1[ii]));
        sign_ptr += 2;
    }
    
    *sign_len = strlen(sign);
    
    free(para_value_p_tmp);
    free(sign_src_buf);
    
    return 0;
}

extern int enos_calc_sign_var(char *device_secret, char *sign, int sign_max, int *sign_len, int para_count, ...)
{
    if((sign == NULL) || (sign_max <= 0) || (sign_len == NULL) || (device_secret == NULL) || (para_count <= 0) || (para_count % 2 != 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error1(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int element_num = para_count / 2;
    struct enos_para_value *para_value_p = (struct enos_para_value *)malloc(sizeof(struct enos_para_value) * element_num);
    if(para_value_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para_value_p malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(para_value_p, 0, sizeof(struct enos_para_value) * element_num);
    
    int ii = 0;
    int ret = 0;
    va_list args;
    va_start(args, para_count);
    char *name_ptr = NULL;
    char *value_ptr = NULL;
    int element_count = 0;
    for(ii = 0; ii < para_count; ii += 2)
    {
        name_ptr = va_arg(args, char *);
        value_ptr = va_arg(args, char *);
        
        if((name_ptr == NULL) || (value_ptr == NULL))
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error2(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            free(para_value_p);
            va_end(args);
            return -1;
        }
        
        snprintf(para_value_p[element_count].para_name, sizeof(para_value_p[element_count].para_name), "%s", name_ptr);
        snprintf(para_value_p[element_count].para_value, sizeof(para_value_p[element_count].para_value), "%s", value_ptr);
        element_count++;
    }
    va_end(args);
    
    ret = enos_calc_sign(para_value_p, element_count, device_secret, sign, sign_max, sign_len);
    
    free(para_value_p);
    
    return ret;
}

extern int enos_mqtt_api_init(struct enos_mqtt_api_struct **enos_mas_pp, char *address, char *product_key, char *device_key, char *device_secret, char *cloud_charset, char *app_charset)
{
    if((enos_mas_pp == NULL) || (address == NULL) || (product_key == NULL) || (device_key == NULL) || (device_secret == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    *enos_mas_pp = NULL;
    
    struct enos_mqtt_api_struct *enos_mas_p_ret = NULL;
    enos_mas_p_ret = (struct enos_mqtt_api_struct *)malloc(sizeof(struct enos_mqtt_api_struct));
    if(enos_mas_p_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mas_p_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(enos_mas_p_ret, 0, sizeof(struct enos_mqtt_api_struct));
    
    if(cloud_charset == NULL)
    {
        snprintf(enos_mas_p_ret->cloud_charset, sizeof(enos_mas_p_ret->cloud_charset), "%s", ENOS_DEFAULT_CLOUD_CHARSET);
    }
    else
    {
        snprintf(enos_mas_p_ret->cloud_charset, sizeof(enos_mas_p_ret->cloud_charset), "%s", cloud_charset);
    }
    
    if(app_charset == NULL)
    {
        snprintf(enos_mas_p_ret->app_charset, sizeof(enos_mas_p_ret->app_charset), "%s", ENOS_DEFAULT_APP_CHARSET);
    }
    else
    {
        snprintf(enos_mas_p_ret->app_charset, sizeof(enos_mas_p_ret->app_charset), "%s", app_charset);
    }
    char *char_temp = NULL;
    int len_temp = 0;
    char *char_temp1 = NULL;
    int len_temp1 = 0;
    int ret_len_temp = 0;
    
    MQTTClient_connectOptions client_opts = MQTTClient_connectOptions_initializer;
    memcpy(&(enos_mas_p_ret->client_opts), &client_opts, sizeof(MQTTClient_connectOptions));
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    ssl_opts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
    memcpy(&(enos_mas_p_ret->ssl_opts), &ssl_opts, sizeof(MQTTClient_SSLOptions));
    enos_mas_p_ret->client_opts.reliable = ENOS_DEFAULT_RELIABLE;
    enos_mas_p_ret->client_opts.maxInflightMessages = ENOS_DEFAULT_MAXINFLIGHTMESSAGES;
    enos_mas_p_ret->persistence_type = MQTTCLIENT_PERSISTENCE_NONE;
    enos_mas_p_ret->persistence_store = NULL;
    
    ret = 0;
    
    len_temp = (int)(strlen((char *)(address)));
    char_temp = (char *)(address);
    len_temp1 = (int)(sizeof(enos_mas_p_ret->address));
    char_temp1 = (char *)(enos_mas_p_ret->address);
    ret += enos_char_code_convert(enos_mas_p_ret->app_charset, enos_mas_p_ret->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    
    len_temp = (int)(strlen((char *)(product_key)));
    char_temp = (char *)(product_key);
    len_temp1 = (int)(sizeof(enos_mas_p_ret->product_key));
    char_temp1 = (char *)(enos_mas_p_ret->product_key);
    ret += enos_char_code_convert(enos_mas_p_ret->app_charset, enos_mas_p_ret->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);

    len_temp = (int)(strlen((char *)(device_key)));
    char_temp = (char *)(device_key);
    len_temp1 = (int)(sizeof(enos_mas_p_ret->device_key));
    char_temp1 = (char *)(enos_mas_p_ret->device_key);
    ret += enos_char_code_convert(enos_mas_p_ret->app_charset, enos_mas_p_ret->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);

    len_temp = (int)(strlen((char *)(device_secret)));
    char_temp = (char *)(device_secret);
    len_temp1 = (int)(sizeof(enos_mas_p_ret->device_secret));
    char_temp1 = (char *)(enos_mas_p_ret->device_secret);
    ret += enos_char_code_convert(enos_mas_p_ret->app_charset, enos_mas_p_ret->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);

    len_temp = (int)(strlen((char *)(device_key)));
    char_temp = (char *)(device_key);
    len_temp1 = (int)(sizeof(enos_mas_p_ret->client_id));
    char_temp1 = (char *)(enos_mas_p_ret->client_id);
    ret += enos_char_code_convert(enos_mas_p_ret->app_charset, enos_mas_p_ret->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_char_code_convert error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(enos_mas_p_ret);
        return -1;
    }
    
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    enos_gettimeofday(&tv, NULL);
    enos_mas_p_ret->time_sec = (long long int)(tv.tv_sec);
    char time_sec_str[256];
    memset(time_sec_str, 0, sizeof(time_sec_str));
    #if defined(__MINGW32__) || defined(__MINGW64__)
        snprintf(time_sec_str, sizeof(time_sec_str), "%I64d", enos_mas_p_ret->time_sec);
    #else
        snprintf(time_sec_str, sizeof(time_sec_str), "%lld", enos_mas_p_ret->time_sec);
    #endif
    
    snprintf(enos_mas_p_ret->mqtt_client_id, sizeof(enos_mas_p_ret->mqtt_client_id), "%s|securemode=2,signmethod=%s,timestamp=%s|", enos_mas_p_ret->device_key, ENOS_DEFAULT_SIGN_METHOD, time_sec_str);
    snprintf(enos_mas_p_ret->mqtt_username, sizeof(enos_mas_p_ret->mqtt_username), "%s&%s", enos_mas_p_ret->device_key, enos_mas_p_ret->product_key);
    
    ret = enos_calc_sign_var(enos_mas_p_ret->device_secret, enos_mas_p_ret->mqtt_passwd, (int)(sizeof(enos_mas_p_ret->mqtt_passwd)), &len_temp, 8, \
        "clientId", enos_mas_p_ret->client_id, \
        "deviceKey", enos_mas_p_ret->device_key, \
        "productKey", enos_mas_p_ret->product_key, \
        "timestamp", time_sec_str);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_calc_sign_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(enos_mas_p_ret);
        return -1;
    }
    
    ret = MQTTClient_create(&(enos_mas_p_ret->client), enos_mas_p_ret->address, enos_mas_p_ret->mqtt_client_id, enos_mas_p_ret->persistence_type, enos_mas_p_ret->persistence_store);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_create error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(enos_mas_p_ret);
        return -1;
    }
    
    enos_mas_p_ret->client_opts.username = (char *)(enos_mas_p_ret->mqtt_username);
    enos_mas_p_ret->client_opts.password = (char *)(enos_mas_p_ret->mqtt_passwd);
    
    ret = MQTTClient_setCallbacks(enos_mas_p_ret->client, (void *)enos_mas_p_ret, enos_connlost, enos_message_arrived, enos_delivered);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_setCallbacks error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        MQTTClient_destroy(&(enos_mas_p_ret->client));
        free(enos_mas_p_ret);
        return -1;
    }
    
    pthread_mutex_init(&(enos_mas_p_ret->wait_delivery_hash_mutex), NULL);
    pthread_mutex_init(&(enos_mas_p_ret->callback_mutex), NULL);
    
    *enos_mas_pp = enos_mas_p_ret;
    return 0;
}

extern int enos_mqtt_api_uninit(struct enos_mqtt_api_struct *enos_mas_p)
{
    if(enos_mas_p == NULL)
    {
        return 0;
    }
    
    MQTTClient_destroy(&(enos_mas_p->client));
    
    int ii = 0;
    if(enos_mas_p->servers != NULL)
    {
        for(ii = 0; ii < enos_mas_p->multi_server_count; ii++)
        {
            free(enos_mas_p->servers[ii]);
            enos_mas_p->servers[ii] = NULL;
        }
        free(enos_mas_p->servers);
        enos_mas_p->servers = NULL;
    }
    
    pthread_mutex_destroy(&(enos_mas_p->wait_delivery_hash_mutex));
    pthread_mutex_destroy(&(enos_mas_p->callback_mutex));
    custom_hashtable_destory(enos_mas_p->wait_delivery_hash);
    
    free(enos_mas_p);
    
    return 0;
}

extern int enos_mqtt_api_set_ssl(struct enos_mqtt_api_struct *enos_mas_p, char *ca_cert_path, char *local_cert_path, char *local_private_path, char *local_private_passwd)
{
    if((enos_mas_p == NULL) || (ca_cert_path == NULL) || (local_cert_path == NULL) || (local_private_path == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();
    
    snprintf(enos_mas_p->mqtt_ssl_ca_cert_path, sizeof(enos_mas_p->mqtt_ssl_ca_cert_path), "%s", ca_cert_path);
    snprintf(enos_mas_p->mqtt_ssl_local_cert_path, sizeof(enos_mas_p->mqtt_ssl_local_cert_path), "%s", local_cert_path);
    snprintf(enos_mas_p->mqtt_ssl_local_private_path, sizeof(enos_mas_p->mqtt_ssl_local_private_path), "%s", local_private_path);
    if(local_private_passwd == NULL)
    {
        memset(enos_mas_p->mqtt_ssl_local_private_passwd, 0, sizeof(enos_mas_p->mqtt_ssl_local_private_passwd));
    }
    else
    {
        snprintf(enos_mas_p->mqtt_ssl_local_private_passwd, sizeof(enos_mas_p->mqtt_ssl_local_private_passwd), "%s", local_private_passwd);
    }
    
    enos_mas_p->client_opts.ssl = &(enos_mas_p->ssl_opts);
    enos_mas_p->client_opts.ssl->trustStore = (char *)(enos_mas_p->mqtt_ssl_ca_cert_path);
    enos_mas_p->client_opts.ssl->keyStore = (char *)(enos_mas_p->mqtt_ssl_local_cert_path);
    enos_mas_p->client_opts.ssl->privateKey = (char *)(enos_mas_p->mqtt_ssl_local_private_path);
    
    if(strlen(enos_mas_p->mqtt_ssl_local_private_passwd) <= 0)
    {
        enos_mas_p->client_opts.ssl->privateKeyPassword = NULL;
    }
    else
    {
        enos_mas_p->client_opts.ssl->privateKeyPassword = (char *)(enos_mas_p->mqtt_ssl_local_private_passwd);
    }
    
    return 0;
}

extern int enos_mqtt_api_set_keepAliveInterval(struct enos_mqtt_api_struct *enos_mas_p, int keepAliveInterval)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    enos_mas_p->client_opts.keepAliveInterval = keepAliveInterval;
    
    return 0;
}

extern int enos_mqtt_api_set_cleansession(struct enos_mqtt_api_struct *enos_mas_p, int cleansession)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    enos_mas_p->client_opts.cleansession = cleansession;
    
    return 0;
}

static int enos_mqtt_api_set_connectTimeout(struct enos_mqtt_api_struct *enos_mas_p, int connectTimeout_ms)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((connectTimeout_ms > 0) && (connectTimeout_ms < 1000))
    {
        connectTimeout_ms = 1000;
    }
    enos_mas_p->client_opts.connectTimeout = connectTimeout_ms / 1000;
    
    return 0;
}

extern int enos_mqtt_api_set_multi_server(struct enos_mqtt_api_struct *enos_mas_p, int server_count, char **servers)
{
    if((enos_mas_p == NULL) || (server_count <= 0) || (servers == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error1(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    char **servers_local = (char **)malloc(server_count * sizeof(char *));
    if(servers_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]servers_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(servers_local, 0, server_count * sizeof(char *));
    
    int ii = 0;
    int jj = 0;
    int len_temp = 0;
    for(ii = 0; ii < server_count; ii++)
    {
        if(servers[ii] == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error2(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            for(jj = 0; jj < server_count; jj++)
            {
                free(servers_local[jj]);
                servers_local[jj] = NULL;
            }
            free(servers_local);
            servers_local = NULL;
            return -1;
        }
        
        len_temp = strlen(servers[ii]) + 1;
        servers_local[ii] = (char *)malloc(len_temp);
        if(servers_local[ii] == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]servers_local[%d] malloc(%d) error(file=%s, function=%s, line=%d)\n", ii, len_temp, __FILE__, __FUNCTION__, __LINE__);
            for(jj = 0; jj < server_count; jj++)
            {
                free(servers_local[jj]);
                servers_local[jj] = NULL;
            }
            free(servers_local);
            servers_local = NULL;
            return -1;
        }
        memset(servers_local[ii], 0, len_temp);
        snprintf(servers_local[ii], len_temp, "%s", servers[ii]);
    }
    
    enos_mas_p->multi_server_count = server_count;
    enos_mas_p->servers = servers_local;
    
    enos_mas_p->client_opts.serverURIcount = enos_mas_p->multi_server_count;
    enos_mas_p->client_opts.serverURIs = enos_mas_p->servers;
    
    return 0;
}

extern int enos_mqtt_api_set_multi_server_var(struct enos_mqtt_api_struct *enos_mas_p, int server_count, ...)
{
    if((enos_mas_p == NULL) || (server_count <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error1(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    char **servers_local = (char **)malloc(server_count * sizeof(char *));
    if(servers_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]servers_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(servers_local, 0, server_count * sizeof(char *));
    
    int ii = 0;
    int jj = 0;
    int len_temp = 0;
    va_list args;
    va_start(args, server_count);
    char *server_ptr = NULL;
    for(ii = 0; ii < server_count; ii++)
    {
        server_ptr = va_arg(args, char *);
        if(server_ptr == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error2(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            for(jj = 0; jj < server_count; jj++)
            {
                free(servers_local[jj]);
                servers_local[jj] = NULL;
            }
            free(servers_local);
            servers_local = NULL;
            va_end(args);
            return -1;
        }
        
        len_temp = strlen(server_ptr) + 1;
        servers_local[ii] = (char *)malloc(len_temp);
        if(servers_local[ii] == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]servers_local[%d] malloc(%d) error(file=%s, function=%s, line=%d)\n", ii, len_temp, __FILE__, __FUNCTION__, __LINE__);
            for(jj = 0; jj < server_count; jj++)
            {
                free(servers_local[jj]);
                servers_local[jj] = NULL;
            }
            free(servers_local);
            servers_local = NULL;
            va_end(args);
            return -1;
        }
        memset(servers_local[ii], 0, len_temp);
        snprintf(servers_local[ii], len_temp, "%s", server_ptr);
    }
    va_end(args);
    
    enos_mas_p->multi_server_count = server_count;
    enos_mas_p->servers = servers_local;
    
    enos_mas_p->client_opts.serverURIcount = enos_mas_p->multi_server_count;
    enos_mas_p->client_opts.serverURIs = enos_mas_p->servers;
    
    return 0;
}

extern int enos_mqtt_api_set_connect_lost_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_connect_lost_callback cb, void *user)
{
    if((enos_mas_p == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->connect_lost_callback = (void *)cb;
    enos_mas_p->connect_lost_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_mqtt_api_set_message_arrived_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_message_arrived_callback cb, void *user)
{
    if((enos_mas_p == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->message_arrived_callback = (void *)cb;
    enos_mas_p->message_arrived_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_mqtt_api_set_delivered_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_delivered_callback cb, void *user)
{
    if((enos_mas_p == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->delivered_callback = (void *)cb;
    enos_mas_p->delivered_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_mqtt_api_connect(struct enos_mqtt_api_struct *enos_mas_p, int timeout_ms)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((timeout_ms < 0) || (timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME))
    {
        timeout_ms = ENOS_DEFAULT_MAX_SYN_WAIT_TIME;
    }
    
    int ret = 0;
    ret = enos_mqtt_api_set_connectTimeout(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_set_connectTimeout error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
    custom_hashtable_destory(enos_mas_p->wait_delivery_hash);
    enos_mas_p->wait_delivery_hash = NULL;
    enos_mas_p->wait_delivery_hash = custom_hashtable_init(ENOS_DEFAULT_WAIT_DELIVERY_HASH_MAX);
    if(enos_mas_p->wait_delivery_hash == NULL)
    {
        pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]custom_hashtable_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
    
    ret = MQTTClient_connect(enos_mas_p->client, &(enos_mas_p->client_opts));
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_connect error, ret_code=%d(file=%s, function=%s, line=%d)\n", ret, __FILE__, __FUNCTION__, __LINE__);
        pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
        custom_hashtable_destory(enos_mas_p->wait_delivery_hash);
        enos_mas_p->wait_delivery_hash = NULL;
        pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
        return -1;
    }
    
    if(pthread_create(&(enos_mas_p->pth_deal_wait_delivery_hash), NULL, enos_deal_wait_delivery_hash_thread, (void *)(enos_mas_p)) != 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]pthread_create pth_deal_wait_delivery_hash error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
        custom_hashtable_destory(enos_mas_p->wait_delivery_hash);
        enos_mas_p->wait_delivery_hash = NULL;
        pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
        MQTTClient_disconnect(enos_mas_p->client, 10000);
        return -1;
    }
    
    return 0;
}

extern int enos_mqtt_api_disconnect(struct enos_mqtt_api_struct *enos_mas_p, int timeout_ms)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_cancel(enos_mas_p->pth_deal_wait_delivery_hash);
    pthread_join(enos_mas_p->pth_deal_wait_delivery_hash, NULL);
    
    pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
    custom_hashtable_destory(enos_mas_p->wait_delivery_hash);
    enos_mas_p->wait_delivery_hash = NULL;
    pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
    
    if((timeout_ms < 0) || (timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME))
    {
        timeout_ms = ENOS_DEFAULT_MAX_SYN_WAIT_TIME;
    }
    
    int ret = 0;
    ret = MQTTClient_disconnect(enos_mas_p->client, timeout_ms);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_disconnect error, ret_code=%d(file=%s, function=%s, line=%d)\n", ret, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}

static int enos_mqtt_api_send_fast_local(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len)
{
    if((enos_mas_p == NULL) || (topic == NULL) || (data == NULL) || (data_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    MQTTClient_message mqtt_msg = MQTTClient_message_initializer;
    mqtt_msg.payload = (void *)data;
    mqtt_msg.payloadlen = data_len;
    mqtt_msg.retained = 0;
    mqtt_msg.qos = 0;
//    MQTTClient_deliveryToken token;
    
    ret = MQTTClient_publishMessage(enos_mas_p->client, topic, &mqtt_msg, NULL);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_publishMessage error, ret_code=%d(file=%s, function=%s, line=%d)\n", ret, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_mqtt_api_send_fast(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len, int need_trans)
{
    if((enos_mas_p == NULL) || (topic == NULL) || (data == NULL) || (data_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(need_trans == 0)
    {
        return enos_mqtt_api_send_fast_local(enos_mas_p, topic, data, data_len);
    }
    
    if(strcmp(enos_mas_p->app_charset, enos_mas_p->cloud_charset) == 0)
    {
        return enos_mqtt_api_send_fast_local(enos_mas_p, topic, data, data_len);
    }
    
    char *char_temp = NULL;
    int len_temp = 0;
    char *char_temp1 = NULL;
    int len_temp1 = 0;
    int ret_len_temp = 0;
    int ret = 0;
    int data_len_local = 0;
    
    int topic_local_max = strlen(topic) * ENOS_CHARSET_TRANS_MULTI;
    char *topic_local = (char *)malloc(topic_local_max);
    if(topic_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_local, 0, topic_local_max);
    
    len_temp = (int)(strlen((char *)(topic)));
    char_temp = (char *)(topic);
    len_temp1 = (int)(topic_local_max);
    char_temp1 = (char *)(topic_local);
    ret = enos_char_code_convert(enos_mas_p->app_charset, enos_mas_p->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_char_code_convert topic error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        return -1;
    }
    
    int data_local_max = data_len * ENOS_CHARSET_TRANS_MULTI;
    char *data_local = (char *)malloc(data_local_max);
    if(data_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        return -1;
    }
    memset(data_local, 0, data_local_max);
    
    len_temp = (int)(data_len);
    char_temp = (char *)(data);
    len_temp1 = (int)(data_local_max);
    char_temp1 = (char *)(data_local);
    ret = enos_char_code_convert(enos_mas_p->app_charset, enos_mas_p->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_char_code_convert data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        free(data_local);
        return -1;
    }
    data_len_local = ret_len_temp;
    
    ret = enos_mqtt_api_send_fast_local(enos_mas_p, topic_local, data_local, data_len_local);
    free(topic_local);
    free(data_local);
    
    return ret;
}

static int enos_mqtt_api_send_normal_syn_local(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (topic == NULL) || (data == NULL) || (data_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    MQTTClient_message mqtt_msg = MQTTClient_message_initializer;
    mqtt_msg.payload = (void *)data;
    mqtt_msg.payloadlen = data_len;
    mqtt_msg.retained = 0;
    mqtt_msg.qos = 1;
    MQTTClient_deliveryToken token;
    
    ret = MQTTClient_publishMessage(enos_mas_p->client, topic, &mqtt_msg, &token);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_publishMessage error, ret_code=%d(file=%s, function=%s, line=%d)\n", ret, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(timeout_ms == 0)
    {
        return 0;
    }
    
    int is_exist = 0;
    struct custom_hash_element *ele_ptr = NULL;
    custom_hash_type para;
    memset(&para, 0, sizeof(custom_hash_type));
    para.token = token;
    
    struct timeval tv_now;
    memset(&tv_now, 0, sizeof(struct timeval));
    enos_gettimeofday(&tv_now, NULL);
    long long sec_now = tv_now.tv_sec;
    
    pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
    ele_ptr = custom_hashtable_search_and_insert(enos_mas_p->wait_delivery_hash, &para, sec_now, &is_exist);
    pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
    if(is_exist == 1)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]token confilict(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    long long int timeout_ms_local = timeout_ms;
    if((timeout_ms_local < 0) || (timeout_ms_local > ENOS_DEFAULT_MAX_SYN_WAIT_TIME))
    {
        timeout_ms_local = ENOS_DEFAULT_MAX_SYN_WAIT_TIME;
    }
    
    long long int elapsed = 0;
    struct timeval tv_start;
    memset(&tv_start, 0, sizeof(struct timeval));
    enos_gettimeofday(&tv_start, NULL);
    while(elapsed < timeout_ms_local)
    {
//        ret = pthread_mutex_trylock(&(enos_mas_p->wait_delivery_hash_mutex));
//        if(ret == 0)
//        {
//            ele_ptr = custom_hashtable_search(enos_mas_p->wait_delivery_hash, &para);
//            if(ele_ptr == NULL)
//            {
//                pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
//                return 0;
//            }
//            pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
//        }
//        else
//        {
//            ;
//        }
        pthread_mutex_lock(&(enos_mas_p->wait_delivery_hash_mutex));
        ele_ptr = custom_hashtable_search(enos_mas_p->wait_delivery_hash, &para);
        if(ele_ptr == NULL)
        {
            pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
            return 0;
        }
        else
        {
            ;
        }
        pthread_mutex_unlock(&(enos_mas_p->wait_delivery_hash_mutex));
        enos_sleep(500);
        elapsed = enos_cal_time_elapsed(&tv_start);
    }
    
    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]timeout(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1;
}

extern int enos_mqtt_api_send_normal_syn(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len, int timeout_ms, int need_trans)
{
    if((enos_mas_p == NULL) || (topic == NULL) || (data == NULL) || (data_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(need_trans == 0)
    {
        return enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic, data, data_len, timeout_ms);
    }
    
    if(strcmp(enos_mas_p->app_charset, enos_mas_p->cloud_charset) == 0)
    {
        return enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic, data, data_len, timeout_ms);
    }
    
    char *char_temp = NULL;
    int len_temp = 0;
    char *char_temp1 = NULL;
    int len_temp1 = 0;
    int ret_len_temp = 0;
    int ret = 0;
    int data_len_local = 0;
    
    int topic_local_max = strlen(topic) * ENOS_CHARSET_TRANS_MULTI;
    char *topic_local = (char *)malloc(topic_local_max);
    if(topic_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_local, 0, topic_local_max);
    
    len_temp = (int)(strlen((char *)(topic)));
    char_temp = (char *)(topic);
    len_temp1 = (int)(topic_local_max);
    char_temp1 = (char *)(topic_local);
    ret = enos_char_code_convert(enos_mas_p->app_charset, enos_mas_p->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_char_code_convert topic error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        return -1;
    }
    
    int data_local_max = data_len * ENOS_CHARSET_TRANS_MULTI;
    char *data_local = (char *)malloc(data_local_max);
    if(data_local == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_local malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        return -1;
    }
    memset(data_local, 0, data_local_max);
    
    len_temp = (int)(data_len);
    char_temp = (char *)(data);
    len_temp1 = (int)(data_local_max);
    char_temp1 = (char *)(data_local);
    ret = enos_char_code_convert(enos_mas_p->app_charset, enos_mas_p->cloud_charset, char_temp, len_temp, char_temp1, len_temp1, &ret_len_temp);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_char_code_convert data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_local);
        free(data_local);
        return -1;
    }
    data_len_local = ret_len_temp;
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_local, data_local, data_len_local, timeout_ms);
    free(topic_local);
    free(data_local);
    
    return ret;
}

extern int generate_enos_sub_dev_register_data(struct enos_sub_dev_register_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = NULL;
    cJSON *para_obj = NULL;
    struct enos_sub_dev_register_input_sub_dev *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", params_array);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_REGISTER));
    
    int ii = 0;
    for(ii = 0; ii < input->sub_dev_count; ii++)
    {
        data_ptr = (struct enos_sub_dev_register_input_sub_dev *)(&(input->sub_dev[ii]));
        
        array_obj = cJSON_CreateObject();
        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString(data_ptr->sub_product_key));
        
        if(data_ptr->sub_device_attr == NULL)
        {
            ;
        }
        else
        {
            para_obj = cJSON_Parse(data_ptr->sub_device_attr);
            if(para_obj == NULL)
            {
                ;
            }
            else
            {
                cJSON_AddItemToObject(array_obj, "deviceAttributes", para_obj);
            }
        }
        
        if(strlen(data_ptr->sub_device_key) > 0)
        {
            cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString(data_ptr->sub_device_key));
        }
        
        if(strlen(data_ptr->sub_device_name) > 0)
        {
            cJSON_AddItemToObject(array_obj, "deviceName", cJSON_CreateString(data_ptr->sub_device_name));
        }
        
        if(strlen(data_ptr->sub_device_desc) > 0)
        {
            cJSON_AddItemToObject(array_obj, "deviceDesc", cJSON_CreateString(data_ptr->sub_device_desc));
        }
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_register_data_response(char *data, int data_len, struct enos_sub_dev_register_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_register_output *output_ret = (struct enos_sub_dev_register_output *)malloc(sizeof(struct enos_sub_dev_register_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_register_output));
    
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    cJSON *array_item1 = NULL;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ii = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_register_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        num1 = cJSON_GetArraySize(array_item1);
        if(num1 > 0)
        {
            output_ret->sub_dev_count = num1;
            output_ret->sub_dev = (struct enos_sub_dev_register_output_sub_dev *)malloc(num1 * sizeof(struct enos_sub_dev_register_output_sub_dev));
            if(output_ret->sub_dev == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_register_output(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->sub_dev, 0, num1 * sizeof(struct enos_sub_dev_register_output_sub_dev));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(array_item1, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "iotId");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_iot_id);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_iot_id);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_product_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_device_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceSecret");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_device_secret);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_device_secret);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_register_output(struct enos_sub_dev_register_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->sub_dev != NULL)
    {
        free(output->sub_dev);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_register_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_register_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_register_callback = (void *)cb;
    enos_mas_p->sub_dev_register_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_register_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_register_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_register_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_register_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_REGISTER);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}


extern int generate_enos_sub_dev_add_topo_data(struct enos_sub_dev_add_topo_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = NULL;
    struct enos_sub_dev_add_topo_input_sub_dev *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", params_array);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_ADD_TOPO));
    
    struct timeval tv_temp;
    memset(&tv_temp, 0, sizeof(struct timeval));
    enos_gettimeofday(&tv_temp, NULL);
    long long int time_sec = (long long int)(tv_temp.tv_sec);
    char time_sec_str[256];
    memset(time_sec_str, 0, sizeof(time_sec_str));
    #if defined(__MINGW32__) || defined(__MINGW64__)
        snprintf(time_sec_str, sizeof(time_sec_str), "%I64d", time_sec);
    #else
        snprintf(time_sec_str, sizeof(time_sec_str), "%lld", time_sec);
    #endif
    
    char sign[256];
    int sign_len = 0;
    memset(sign, 0, sizeof(sign));
    
    int ret = 0;
    int ii = 0;
    for(ii = 0; ii < input->sub_dev_count; ii++)
    {
        data_ptr = (struct enos_sub_dev_add_topo_input_sub_dev *)(&(input->sub_dev[ii]));
        
        array_obj = cJSON_CreateObject();
        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "clientId", cJSON_CreateString(data_ptr->sub_device_key));
        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString(data_ptr->sub_product_key));
        cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString(data_ptr->sub_device_key));
        cJSON_AddItemToObject(array_obj, "timestamp", cJSON_CreateString(time_sec_str));
        cJSON_AddItemToObject(array_obj, "signmethod", cJSON_CreateString(ENOS_DEFAULT_SIGN_METHOD));
        
        sign_len = 0;
        memset(sign, 0, sizeof(sign));
        ret = enos_calc_sign_var(data_ptr->sub_device_secret, sign, (int)(sizeof(sign)), &sign_len, 8, \
            "clientId", data_ptr->sub_device_key, \
            "productKey", data_ptr->sub_product_key, \
            "deviceKey", data_ptr->sub_device_key, \
            "timestamp", time_sec_str);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_calc_sign_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            cJSON_Delete(root);
            return -1;
        }
        cJSON_AddItemToObject(array_obj, "sign", cJSON_CreateString(sign));
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_add_topo_data_response(char *data, int data_len, struct enos_sub_dev_add_topo_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_add_topo_output *output_ret = (struct enos_sub_dev_add_topo_output *)malloc(sizeof(struct enos_sub_dev_add_topo_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_add_topo_output));
    
    cJSON *item1 = NULL;
    cJSON *array_item1 = NULL;
    char *data_ret = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_add_topo_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        data_ret = cJSON_Print(array_item1);
        output_ret->data = data_ret;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_add_topo_output(struct enos_sub_dev_add_topo_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_add_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_add_topo_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_add_topo_callback = (void *)cb;
    enos_mas_p->sub_dev_add_topo_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_add_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_add_topo_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_add_topo_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_add_topo_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_ADD_TOPO);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_sub_dev_del_topo_data(struct enos_sub_dev_del_topo_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = NULL;
    struct enos_sub_dev_del_topo_input_sub_dev *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", params_array);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_DEL_TOPO));
    
    int ii = 0;
    for(ii = 0; ii < input->sub_dev_count; ii++)
    {
        data_ptr = (struct enos_sub_dev_del_topo_input_sub_dev *)(&(input->sub_dev[ii]));
        
        array_obj = cJSON_CreateObject();
        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString(data_ptr->sub_product_key));
        cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString(data_ptr->sub_device_key));
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_del_topo_data_response(char *data, int data_len, struct enos_sub_dev_del_topo_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_del_topo_output *output_ret = (struct enos_sub_dev_del_topo_output *)malloc(sizeof(struct enos_sub_dev_del_topo_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_del_topo_output));
    
    cJSON *item1 = NULL;
    cJSON *array_item1 = NULL;
    char *data_ret = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_del_topo_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        data_ret = cJSON_Print(array_item1);
        output_ret->data = data_ret;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_del_topo_output(struct enos_sub_dev_del_topo_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_del_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_del_topo_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_del_topo_callback = (void *)cb;
    enos_mas_p->sub_dev_del_topo_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_del_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_del_topo_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_del_topo_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_del_topo_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DEL_TOPO);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}





extern int generate_enos_sub_dev_get_topo_data(struct enos_sub_dev_get_topo_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *params_array = cJSON_CreateArray();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", params_array);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_GET_TOPO));
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_get_topo_data_response(char *data, int data_len, struct enos_sub_dev_get_topo_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_get_topo_output *output_ret = (struct enos_sub_dev_get_topo_output *)malloc(sizeof(struct enos_sub_dev_get_topo_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_get_topo_output));
    
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    cJSON *array_item1 = NULL;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ii = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_get_topo_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        num1 = cJSON_GetArraySize(array_item1);
        if(num1 > 0)
        {
            output_ret->sub_dev_count = num1;
            output_ret->sub_dev = (struct enos_sub_dev_get_topo_output_sub_dev *)malloc(num1 * sizeof(struct enos_sub_dev_get_topo_output_sub_dev));
            if(output_ret->sub_dev == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_get_topo_output(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->sub_dev, 0, num1 * sizeof(struct enos_sub_dev_get_topo_output_sub_dev));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(array_item1, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_product_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_device_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_get_topo_output(struct enos_sub_dev_get_topo_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->sub_dev != NULL)
    {
        free(output->sub_dev);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_get_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_get_topo_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_get_topo_callback = (void *)cb;
    enos_mas_p->sub_dev_get_topo_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_get_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_get_topo_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_get_topo_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_get_topo_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_GET_TOPO);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_sub_dev_login_data(struct enos_sub_dev_login_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
//    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = cJSON_CreateObject();
    struct enos_sub_dev_login_input_sub_dev *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_LOGIN));
    
    struct timeval tv_temp;
    memset(&tv_temp, 0, sizeof(struct timeval));
    enos_gettimeofday(&tv_temp, NULL);
    long long int time_sec = (long long int)(tv_temp.tv_sec);
    char time_sec_str[256];
    memset(time_sec_str, 0, sizeof(time_sec_str));
    #if defined(__MINGW32__) || defined(__MINGW64__)
        snprintf(time_sec_str, sizeof(time_sec_str), "%I64d", time_sec);
    #else
        snprintf(time_sec_str, sizeof(time_sec_str), "%lld", time_sec);
    #endif
    
    char sign[256];
    int sign_len = 0;
    memset(sign, 0, sizeof(sign));
    
    int ret = 0;
    int ii = 0;
    for(ii = 0; ii < input->sub_dev_count; ii++)
    {
        data_ptr = (struct enos_sub_dev_login_input_sub_dev *)(&(input->sub_dev[ii]));
        
//        array_obj = cJSON_CreateObject();
//        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "clientId", cJSON_CreateString(data_ptr->sub_device_key));
        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString(data_ptr->sub_product_key));
        cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString(data_ptr->sub_device_key));
        cJSON_AddItemToObject(array_obj, "timestamp", cJSON_CreateString(time_sec_str));
        cJSON_AddItemToObject(array_obj, "signMethod", cJSON_CreateString(ENOS_DEFAULT_SIGN_METHOD));
        cJSON_AddItemToObject(array_obj, "cleanSession", cJSON_CreateString(data_ptr->clean_session));
        
        sign_len = 0;
        memset(sign, 0, sizeof(sign));
        //cleanSession is not participating in computing
        ret = enos_calc_sign_var(data_ptr->sub_device_secret, sign, (int)(sizeof(sign)), &sign_len, 8, \
            "clientId", data_ptr->sub_device_key, \
            "productKey", data_ptr->sub_product_key, \
            "deviceKey", data_ptr->sub_device_key, \
            "timestamp", time_sec_str);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_calc_sign_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            cJSON_Delete(root);
            return -1;
        }
        cJSON_AddItemToObject(array_obj, "sign", cJSON_CreateString(sign));
        break;
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_login_data_response(char *data, int data_len, struct enos_sub_dev_login_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_login_output *output_ret = (struct enos_sub_dev_login_output *)malloc(sizeof(struct enos_sub_dev_login_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_login_output));
    
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    cJSON *array_item1 = NULL;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ii = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_login_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        num1 = cJSON_GetArraySize(array_item1);
        if(num1 > 0)
        {
            output_ret->sub_dev_count = num1;
            output_ret->sub_dev = (struct enos_sub_dev_login_output_sub_dev *)malloc(num1 * sizeof(struct enos_sub_dev_login_output_sub_dev));
            if(output_ret->sub_dev == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_login_output(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->sub_dev, 0, num1 * sizeof(struct enos_sub_dev_login_output_sub_dev));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(array_item1, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_product_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_device_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_login_output(struct enos_sub_dev_login_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->sub_dev != NULL)
    {
        free(output->sub_dev);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_login_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_login_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_login_callback = (void *)cb;
    enos_mas_p->sub_dev_login_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_login_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_login_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_login_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_login_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGIN);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}




extern int generate_enos_sub_dev_logout_data(struct enos_sub_dev_logout_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
//    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = cJSON_CreateObject();
    struct enos_sub_dev_logout_input_sub_dev *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_SUB_DEVICE_LOGOUT));
    
    int ii = 0;
    for(ii = 0; ii < input->sub_dev_count; ii++)
    {
        data_ptr = (struct enos_sub_dev_logout_input_sub_dev *)(&(input->sub_dev[ii]));
        
//        array_obj = cJSON_CreateObject();
//        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString(data_ptr->sub_product_key));
        cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString(data_ptr->sub_device_key));
        break;
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_sub_dev_logout_data_response(char *data, int data_len, struct enos_sub_dev_logout_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_logout_output *output_ret = (struct enos_sub_dev_logout_output *)malloc(sizeof(struct enos_sub_dev_logout_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_logout_output));
    
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    cJSON *array_item1 = NULL;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ii = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_logout_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "data");
    if(array_item1 != NULL)
    {
        num1 = cJSON_GetArraySize(array_item1);
        if(num1 > 0)
        {
            output_ret->sub_dev_count = num1;
            output_ret->sub_dev = (struct enos_sub_dev_logout_output_sub_dev *)malloc(num1 * sizeof(struct enos_sub_dev_logout_output_sub_dev));
            if(output_ret->sub_dev == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_logout_output(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->sub_dev, 0, num1 * sizeof(struct enos_sub_dev_logout_output_sub_dev));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(array_item1, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_product_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->sub_dev[ii].sub_device_key);
                    char_temp = (char *)(output_ret->sub_dev[ii].sub_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_logout_output(struct enos_sub_dev_logout_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->sub_dev != NULL)
    {
        free(output->sub_dev);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_logout_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_logout_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_logout_callback = (void *)cb;
    enos_mas_p->sub_dev_logout_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_logout_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_logout_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_logout_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_logout_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGOUT);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_tag_query_data(struct enos_dev_tag_query_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    cJSON *array_obj = cJSON_CreateArray();
    cJSON *array_item_obj = NULL;
    struct enos_dev_tag_query_input_para *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "tags", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_TAG_QUERY));
    
    int ii = 0;
    for(ii = 0; ii < input->para_count; ii++)
    {
        data_ptr = (struct enos_dev_tag_query_input_para *)(&(input->para[ii]));
        
        array_item_obj = cJSON_CreateString(data_ptr->para_name);
        cJSON_AddItemToArray(array_obj, array_item_obj);
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_query_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_query_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_tag_query_output *output_ret = (struct enos_dev_tag_query_output *)malloc(sizeof(struct enos_dev_tag_query_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_tag_query_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ii = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_query_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        num1 = get_child_obj_num(item);
        if(num1 > 0)
        {
            output_ret->para_count = num1;
            output_ret->para = (struct enos_dev_tag_query_output_para *)malloc(num1 * sizeof(struct enos_dev_tag_query_output_para));
            if(output_ret->para == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_dev_tag_query_output(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->para, 0, num1 * sizeof(struct enos_dev_tag_query_output_para));
            
            item1 = item->child;
            for(ii = 0; ii < num1; ii++)
            {
                len_temp = sizeof(output_ret->para[ii].para_name);
                char_temp = (char *)(output_ret->para[ii].para_name);
                snprintf(char_temp, len_temp, "%s", (char *)(item1->string));

                len_temp = sizeof(output_ret->para[ii].para_value);
                char_temp = (char *)(output_ret->para[ii].para_value);
                snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
                item1 = item1->next;
            }
        }
        
    }
    
    ret = parse_enos_dev_tag_query_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_tag_query_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_query_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_query_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_query_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_tag_query_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_tag_query_output_info_ex *)malloc(sizeof(struct enos_dev_tag_query_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_tag_query_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_query_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_tag_query_output(struct enos_dev_tag_query_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->para != NULL)
    {
        free(output->para);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_tag_query_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_tag_query_output_info_ex(struct enos_dev_tag_query_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_tag_query_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_query_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_tag_query_callback = (void *)cb;
    enos_mas_p->dev_tag_query_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_tag_query_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_query_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_tag_query_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_tag_query_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_TAG_QUERY);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_tag_update_data(struct enos_dev_tag_update_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *array_obj = cJSON_CreateArray();
    cJSON *array_item_obj = NULL;
    struct enos_dev_tag_update_input_para *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_TAG_UPDATE));
    
    int ii = 0;
    for(ii = 0; ii < input->para_count; ii++)
    {
        data_ptr = (struct enos_dev_tag_update_input_para *)(&(input->para[ii]));
        
        array_item_obj = cJSON_CreateObject();
        cJSON_AddItemToArray(array_obj, array_item_obj);
        
        cJSON_AddItemToObject(array_item_obj, "tagKey", cJSON_CreateString(data_ptr->para_name));
        cJSON_AddItemToObject(array_item_obj, "tagValue", cJSON_CreateString(data_ptr->para_value));
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_update_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_update_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_tag_update_output *output_ret = (struct enos_dev_tag_update_output *)malloc(sizeof(struct enos_dev_tag_update_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_tag_update_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_update_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_tag_update_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_tag_update_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_update_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_update_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_update_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_tag_update_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_tag_update_output_info_ex *)malloc(sizeof(struct enos_dev_tag_update_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_tag_update_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_update_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_tag_update_output(struct enos_dev_tag_update_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_tag_update_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_tag_update_output_info_ex(struct enos_dev_tag_update_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_tag_update_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_update_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_tag_update_callback = (void *)cb;
    enos_mas_p->dev_tag_update_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_tag_update_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_update_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_tag_update_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_tag_update_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_TAG_UPDATE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_tag_delete_data(struct enos_dev_tag_delete_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    cJSON *array_obj = cJSON_CreateArray();
    cJSON *array_item_obj = NULL;
    struct enos_dev_tag_delete_input_para *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "tags", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_TAG_DELETE));
    
    int ii = 0;
    for(ii = 0; ii < input->para_count; ii++)
    {
        data_ptr = (struct enos_dev_tag_delete_input_para *)(&(input->para[ii]));
        
        array_item_obj = cJSON_CreateString(data_ptr->para_name);
        cJSON_AddItemToArray(array_obj, array_item_obj);
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_delete_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_delete_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_tag_delete_output *output_ret = (struct enos_dev_tag_delete_output *)malloc(sizeof(struct enos_dev_tag_delete_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_tag_delete_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_delete_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_tag_delete_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_tag_delete_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_delete_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_tag_delete_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_delete_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_tag_delete_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_tag_delete_output_info_ex *)malloc(sizeof(struct enos_dev_tag_delete_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_tag_delete_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_tag_delete_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_tag_delete_output(struct enos_dev_tag_delete_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_tag_delete_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_tag_delete_output_info_ex(struct enos_dev_tag_delete_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_tag_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_delete_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_tag_delete_callback = (void *)cb;
    enos_mas_p->dev_tag_delete_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_tag_delete_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_delete_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_tag_delete_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_tag_delete_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_TAG_DELETE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_attr_query_data(struct enos_dev_attr_query_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    cJSON *array_obj = cJSON_CreateArray();
    cJSON *array_item_obj = NULL;
    struct enos_dev_attr_query_input_para *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "attributes", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_ATTR_QUERY));
    
    int ii = 0;
    for(ii = 0; ii < input->para_count; ii++)
    {
        data_ptr = (struct enos_dev_attr_query_input_para *)(&(input->para[ii]));
        
        array_item_obj = cJSON_CreateString(data_ptr->para_name);
        cJSON_AddItemToArray(array_obj, array_item_obj);
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_query_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_query_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_attr_query_output *output_ret = (struct enos_dev_attr_query_output *)malloc(sizeof(struct enos_dev_attr_query_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_attr_query_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_query_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_attr_query_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_attr_query_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_query_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_query_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_query_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_attr_query_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_attr_query_output_info_ex *)malloc(sizeof(struct enos_dev_attr_query_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_attr_query_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_query_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_attr_query_output(struct enos_dev_attr_query_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_attr_query_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_attr_query_output_info_ex(struct enos_dev_attr_query_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_attr_query_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_query_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_attr_query_callback = (void *)cb;
    enos_mas_p->dev_attr_query_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_attr_query_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_query_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_attr_query_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_attr_query_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_QUERY);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_attr_update_data(struct enos_dev_attr_update_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL) || (input->data == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *data_in_json = cJSON_Parse(input->data);
    if(data_in_json == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "attributes", data_in_json);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_ATTR_UPDATE));
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_update_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_update_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_attr_update_output *output_ret = (struct enos_dev_attr_update_output *)malloc(sizeof(struct enos_dev_attr_update_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_attr_update_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_update_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_attr_update_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_attr_update_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_update_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_update_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_update_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_attr_update_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_attr_update_output_info_ex *)malloc(sizeof(struct enos_dev_attr_update_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_attr_update_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_update_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_attr_update_output(struct enos_dev_attr_update_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_attr_update_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_attr_update_output_info_ex(struct enos_dev_attr_update_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_attr_update_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_update_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_attr_update_callback = (void *)cb;
    enos_mas_p->dev_attr_update_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_attr_update_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_update_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_attr_update_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_attr_update_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_UPDATE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_attr_delete_data(struct enos_dev_attr_delete_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    cJSON *array_obj = cJSON_CreateArray();
    cJSON *array_item_obj = NULL;
    struct enos_dev_attr_delete_input_para *data_ptr = NULL;
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "attributes", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_ATTR_DELETE));
    
    int ii = 0;
    for(ii = 0; ii < input->para_count; ii++)
    {
        data_ptr = (struct enos_dev_attr_delete_input_para *)(&(input->para[ii]));
        
        array_item_obj = cJSON_CreateString(data_ptr->para_name);
        cJSON_AddItemToArray(array_obj, array_item_obj);
    }
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_delete_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_delete_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_attr_delete_output *output_ret = (struct enos_dev_attr_delete_output *)malloc(sizeof(struct enos_dev_attr_delete_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_attr_delete_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_delete_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_attr_delete_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_attr_delete_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_delete_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_attr_delete_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_delete_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_attr_delete_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_attr_delete_output_info_ex *)malloc(sizeof(struct enos_dev_attr_delete_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_attr_delete_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_attr_delete_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_attr_delete_output(struct enos_dev_attr_delete_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_attr_delete_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_attr_delete_output_info_ex(struct enos_dev_attr_delete_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_attr_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_delete_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_attr_delete_callback = (void *)cb;
    enos_mas_p->dev_attr_delete_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_attr_delete_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_delete_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_attr_delete_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_attr_delete_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_ATTR_DELETE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_point_post_data(struct enos_dev_point_post_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL) || (input->data == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *data_in_json = cJSON_Parse(input->data);
    if(data_in_json == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "measurepoints", data_in_json);
    if(input->time_ms_is_valid)
    {
        cJSON_AddItemToObject(obj1, "time", cJSON_CreateNumber((double)(input->time_ms)));
    }
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_POINT_POST));
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_point_post_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_post_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_point_post_output *output_ret = (struct enos_dev_point_post_output *)malloc(sizeof(struct enos_dev_point_post_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_point_post_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_post_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_point_post_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_point_post_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_post_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_point_post_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_post_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_point_post_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_point_post_output_info_ex *)malloc(sizeof(struct enos_dev_point_post_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_point_post_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_post_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_point_post_output(struct enos_dev_point_post_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_point_post_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_point_post_output_info_ex(struct enos_dev_point_post_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_point_post_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_post_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_point_post_callback = (void *)cb;
    enos_mas_p->dev_point_post_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_point_post_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_point_post_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_point_post_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_point_post_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_POINT_POST);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_event_post_data(struct enos_dev_event_post_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL) || (input->data == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *data_in_json = cJSON_Parse(input->data);
    if(data_in_json == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(obj1, "events", data_in_json);
    if(input->time_ms_is_valid)
    {
        cJSON_AddItemToObject(obj1, "time", cJSON_CreateNumber((double)(input->time_ms)));
    }
    char method_temp[2048];
    memset(method_temp, 0, sizeof(method_temp));
    snprintf(method_temp, sizeof(method_temp), "%s.%s.%s", ENOS_METHOD1_DEVICE_EVENT_POST, input->event_id, ENOS_METHOD2_DEVICE_EVENT_POST);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(method_temp));
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_event_post_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_event_post_output *output_ret = (struct enos_dev_event_post_output *)malloc(sizeof(struct enos_dev_event_post_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_event_post_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_event_post_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_event_post_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_event_post_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_event_post_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_event_post_output_info_ex *)malloc(sizeof(struct enos_dev_event_post_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_event_post_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    ptr1 = strstr(topic_name, ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST);
    if(ptr1 == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    if(ptr1 - topic_name + strlen(ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST) + 1 >= topic_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST) + 1;
    
    memset(str_temp1, 0, sizeof(str_temp1));
    sscanf(ptr1, "%[^/]", str_temp1);
    snprintf(info_ret->event_id, sizeof(info_ret->event_id), "%s", str_temp1);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_event_post_output(struct enos_dev_event_post_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_event_post_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_event_post_output_info_ex(struct enos_dev_event_post_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_event_post_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_event_post_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_event_post_callback = (void *)cb;
    enos_mas_p->dev_event_post_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

//上报设备事件(非透传)
extern int enos_dev_event_post_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, char *input, int input_len, enos_dev_event_post_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (event_id == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_event_post_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_event_post_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST, event_id, ENOS_TOPIC_SUFFIX2_DEVICE_EVENT_POST);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_dev_event_post_raw_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_raw_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_event_post_raw_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_event_post_raw_output_info_ex *)malloc(sizeof(struct enos_dev_event_post_raw_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_event_post_raw_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_event_post_raw_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_event_post_raw_output_info_ex(struct enos_dev_event_post_raw_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_event_post_raw_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_event_post_raw_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_event_post_raw_callback = (void *)cb;
    enos_mas_p->dev_event_post_raw_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_event_post_raw_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_event_post_raw_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_event_post_raw_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_event_post_raw_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_EVENT_POST_RAW);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}


extern int generate_enos_general_reply_data(struct enos_general_reply *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *data_in_json = NULL;
    
    if(input->data != NULL)
    {
        data_in_json = cJSON_Parse(input->data);
        if(data_in_json == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    else
    {
        data_in_json = cJSON_CreateObject();
    }
    
    cJSON *root = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "code", cJSON_CreateNumber((double)(input->code)));
    cJSON_AddItemToObject(root, "message", cJSON_CreateString(input->message));
    cJSON_AddItemToObject(root, "data", data_in_json);
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int generate_enos_general_reply_data_var(char *id, int code, char *msg, char *data_in, char **data, int *data_len)
{
    if((id == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *data_in_json = NULL;
    
    if(data_in != NULL)
    {
        data_in_json = cJSON_Parse(data_in);
        if(data_in_json == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    else
    {
        data_in_json = cJSON_CreateObject();
    }
    
    
    cJSON *root = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(id));
    cJSON_AddItemToObject(root, "code", cJSON_CreateNumber((double)(code)));
    if(msg != NULL)
    {
        cJSON_AddItemToObject(root, "message", cJSON_CreateString(msg));
    }
    cJSON_AddItemToObject(root, "data", data_in_json);
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}


extern int parse_enos_dev_point_set_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_set_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_point_set_callback_data *output_ret = (struct enos_dev_point_set_callback_data *)malloc(sizeof(struct enos_dev_point_set_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_point_set_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_set_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_point_set_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_point_set_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_set_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_point_set_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_set_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_point_set_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_point_set_callback_data_info_ex *)malloc(sizeof(struct enos_dev_point_set_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_point_set_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_set_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_point_set_callback_data(struct enos_dev_point_set_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_point_set_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_point_set_callback_data_info_ex(struct enos_dev_point_set_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_point_set_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_set_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_point_set_callback = (void *)cb;
    enos_mas_p->dev_point_set_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_point_set_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_point_set_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_point_set_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_point_set_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_POINT_SET);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_point_set_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_POINT_SET);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_dev_point_get_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_get_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_point_get_callback_data *output_ret = (struct enos_dev_point_get_callback_data *)malloc(sizeof(struct enos_dev_point_get_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_point_get_callback_data));
    
    cJSON *item1 = NULL;
    cJSON *array_item1 = NULL;
    int ii = 0;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_get_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    array_item1 = cJSON_GetObjectItem(root, "params");
    if(array_item1 != NULL)
    {
        num1 = cJSON_GetArraySize(array_item1);
        if(num1 > 0)
        {
            output_ret->para_count = num1;
            output_ret->para = (struct enos_dev_point_get_callback_data_para *)malloc(num1 * sizeof(struct enos_dev_point_get_callback_data_para));
            if(output_ret->para == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_dev_point_get_callback_data(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->para, 0, num1 * sizeof(struct enos_dev_point_get_callback_data_para));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(array_item1, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                len_temp = sizeof(output_ret->para[ii].para_name);
                char_temp = (char *)(output_ret->para[ii].para_name);
                snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
            }
        }
        
    }
    
    ret = parse_enos_dev_point_get_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_point_get_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_get_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_point_get_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_get_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_point_get_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_point_get_callback_data_info_ex *)malloc(sizeof(struct enos_dev_point_get_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_point_get_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_get_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_point_get_callback_data(struct enos_dev_point_get_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->para != NULL)
    {
        free(output->para);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_point_get_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_point_get_callback_data_info_ex(struct enos_dev_point_get_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_point_get_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_get_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_point_get_callback = (void *)cb;
    enos_mas_p->dev_point_get_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_point_get_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_point_get_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_point_get_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_point_get_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_POINT_GET);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_point_get_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_POINT_GET);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_dev_service_invoke_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_service_invoke_callback_data *output_ret = (struct enos_dev_service_invoke_callback_data *)malloc(sizeof(struct enos_dev_service_invoke_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_service_invoke_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_service_invoke_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_service_invoke_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_service_invoke_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_service_invoke_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_service_invoke_callback_data_info_ex *)malloc(sizeof(struct enos_dev_service_invoke_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_service_invoke_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    ptr1 = strstr(topic_name, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE);
    if(ptr1 == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    if(ptr1 - topic_name + strlen(ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE) + 1 >= topic_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE) + 1;
    
    memset(str_temp1, 0, sizeof(str_temp1));
    sscanf(ptr1, "%[^/]", str_temp1);
    snprintf(info_ret->event_id, sizeof(info_ret->event_id), "%s", str_temp1);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_service_invoke_callback_data(struct enos_dev_service_invoke_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_service_invoke_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_service_invoke_callback_data_info_ex(struct enos_dev_service_invoke_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_service_invoke_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_service_invoke_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_service_invoke_callback = (void *)cb;
    enos_mas_p->dev_service_invoke_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_service_invoke_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, enos_dev_service_invoke_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (event_id == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_service_invoke_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_service_invoke_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE, event_id);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_service_invoke_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (event_id == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE, event_id);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_dev_service_invoke_raw_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_raw_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_service_invoke_raw_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_service_invoke_raw_callback_data_info_ex *)malloc(sizeof(struct enos_dev_service_invoke_raw_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_service_invoke_raw_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_raw_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_service_invoke_raw_callback_data_info_ex(struct enos_dev_service_invoke_raw_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_service_invoke_raw_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_service_invoke_raw_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_service_invoke_raw_callback = (void *)cb;
    enos_mas_p->dev_service_invoke_raw_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_service_invoke_raw_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_service_invoke_raw_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_service_invoke_raw_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_service_invoke_raw_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE_RAW);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_service_invoke_raw_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE_RAW);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}


extern int parse_enos_dev_disable_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_disable_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_disable_callback_data *output_ret = (struct enos_dev_disable_callback_data *)malloc(sizeof(struct enos_dev_disable_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_disable_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_disable_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_disable_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_disable_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_disable_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_disable_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_disable_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_disable_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_disable_callback_data_info_ex *)malloc(sizeof(struct enos_dev_disable_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_disable_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_disable_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_disable_callback_data(struct enos_dev_disable_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_disable_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_disable_callback_data_info_ex(struct enos_dev_disable_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_disable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_disable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_disable_callback = (void *)cb;
    enos_mas_p->dev_disable_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_disable_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_disable_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_disable_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_disable_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_DISABLE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_disable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_DISABLE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_dev_enable_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_enable_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_enable_callback_data *output_ret = (struct enos_dev_enable_callback_data *)malloc(sizeof(struct enos_dev_enable_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_enable_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_enable_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_enable_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_enable_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_enable_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_enable_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_enable_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_enable_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_enable_callback_data_info_ex *)malloc(sizeof(struct enos_dev_enable_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_enable_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_enable_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_enable_callback_data(struct enos_dev_enable_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_enable_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_enable_callback_data_info_ex(struct enos_dev_enable_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_enable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_enable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_enable_callback = (void *)cb;
    enos_mas_p->dev_enable_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_enable_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_enable_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_enable_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_enable_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_ENABLE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_enable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_ENABLE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}


extern int parse_enos_dev_delete_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_delete_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_delete_callback_data *output_ret = (struct enos_dev_delete_callback_data *)malloc(sizeof(struct enos_dev_delete_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_delete_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_delete_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_delete_callback_data_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_delete_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_delete_callback_data(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_delete_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_delete_callback_data_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_delete_callback_data_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_delete_callback_data_info_ex *)malloc(sizeof(struct enos_dev_delete_callback_data_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_delete_callback_data_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_delete_callback_data_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_delete_callback_data(struct enos_dev_delete_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_delete_callback_data_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_delete_callback_data_info_ex(struct enos_dev_delete_callback_data_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_delete_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_delete_callback = (void *)cb;
    enos_mas_p->dev_delete_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_delete_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_delete_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_delete_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_delete_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_DELETE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_dev_delete_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_DELETE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_sub_dev_disable_callback_data(char *data, int data_len, struct enos_sub_dev_disable_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_disable_callback_data *output_ret = (struct enos_sub_dev_disable_callback_data *)malloc(sizeof(struct enos_sub_dev_disable_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_disable_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    int ii = 0;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_disable_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        num1 = cJSON_GetArraySize(item);
        if(num1 > 0)
        {
            output_ret->para_count = num1;
            output_ret->para = (struct enos_sub_dev_disable_callback_data_para *)malloc(num1 * sizeof(struct enos_sub_dev_disable_callback_data_para));
            if(output_ret->para == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_disable_callback_data(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->para, 0, num1 * sizeof(struct enos_sub_dev_disable_callback_data_para));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(item, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_product_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_device_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_disable_callback_data(struct enos_sub_dev_disable_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->para != NULL)
    {
        free(output->para);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_disable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_disable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_disable_callback = (void *)cb;
    enos_mas_p->sub_dev_disable_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_disable_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_disable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_disable_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_disable_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DISABLE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_sub_dev_disable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DISABLE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_sub_dev_enable_callback_data(char *data, int data_len, struct enos_sub_dev_enable_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_enable_callback_data *output_ret = (struct enos_sub_dev_enable_callback_data *)malloc(sizeof(struct enos_sub_dev_enable_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_enable_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    int ii = 0;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_enable_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        num1 = cJSON_GetArraySize(item);
        if(num1 > 0)
        {
            output_ret->para_count = num1;
            output_ret->para = (struct enos_sub_dev_enable_callback_data_para *)malloc(num1 * sizeof(struct enos_sub_dev_enable_callback_data_para));
            if(output_ret->para == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_enable_callback_data(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->para, 0, num1 * sizeof(struct enos_sub_dev_enable_callback_data_para));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(item, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_product_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_device_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_enable_callback_data(struct enos_sub_dev_enable_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->para != NULL)
    {
        free(output->para);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_enable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_enable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_enable_callback = (void *)cb;
    enos_mas_p->sub_dev_enable_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_enable_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_enable_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_enable_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_enable_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_ENABLE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_sub_dev_enable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_ENABLE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int parse_enos_sub_dev_delete_callback_data(char *data, int data_len, struct enos_sub_dev_delete_callback_data **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_sub_dev_delete_callback_data *output_ret = (struct enos_sub_dev_delete_callback_data *)malloc(sizeof(struct enos_sub_dev_delete_callback_data));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_sub_dev_delete_callback_data));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    cJSON *item2 = NULL;
    int ii = 0;
    int num1 = 0;
    int len_temp = 0;
    char *char_temp = NULL;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_delete_callback_data(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "params");
    if(item != NULL)
    {
        num1 = cJSON_GetArraySize(item);
        if(num1 > 0)
        {
            output_ret->para_count = num1;
            output_ret->para = (struct enos_sub_dev_delete_callback_data_para *)malloc(num1 * sizeof(struct enos_sub_dev_delete_callback_data_para));
            if(output_ret->para == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret->para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
                free_enos_sub_dev_delete_callback_data(output_ret);
                free(data_temp);
                cJSON_Delete(root);
                return -1;
            }
            memset(output_ret->para, 0, num1 * sizeof(struct enos_sub_dev_delete_callback_data_para));
            
            for(ii = 0; ii < num1; ii++)
            {
                item1 = cJSON_GetArrayItem(item, ii);
                if(item1 == NULL)
                {
                    enos_printf(NULL, ENOS_LOG_ERROR, "cJSON_GetArrayItem %d error(file=%s, function=%s, line=%d)\n", ii, __FILE__, __FUNCTION__, __LINE__);
                    continue;
                }
                
                item2 = cJSON_GetObjectItem(item1, "productKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_product_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_product_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
                
                item2 = cJSON_GetObjectItem(item1, "deviceKey");
                if(item2 != NULL)
                {
                    len_temp = sizeof(output_ret->para[ii].sub_dev_device_key);
                    char_temp = (char *)(output_ret->para[ii].sub_dev_device_key);
                    snprintf(char_temp, len_temp, "%s", (char *)(item2->valuestring));
                }
            }
        }
        
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int free_enos_sub_dev_delete_callback_data(struct enos_sub_dev_delete_callback_data *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->para != NULL)
    {
        free(output->para);
    }
    
    free(output);
    
    return 0;
}

extern int enos_sub_dev_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_delete_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->sub_dev_delete_callback = (void *)cb;
    enos_mas_p->sub_dev_delete_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_sub_dev_delete_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_delete_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->sub_dev_delete_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->sub_dev_delete_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    
    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DELETE);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_subscribe, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_subscribe, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

extern int enos_send_sub_dev_delete_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms)
{
    if((enos_mas_p == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    char topic_subscribe[512];
    memset(topic_subscribe, 0, sizeof(topic_subscribe));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));

    snprintf(topic_subscribe, sizeof(topic_subscribe), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX2, enos_mas_p->product_key, enos_mas_p->device_key, ENOS_TOPIC_SUFFIX_SUB_DEVICE_DELETE);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_subscribe, ENOS_TOPIC_REPLY1);
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_reply, input, input_len, timeout_ms);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}



extern int generate_enos_dev_get_tsl_data(struct enos_dev_get_tsl_input *input, char **data, int *data_len)
{
    if((input == NULL) || (data == NULL) || (data_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *obj1 = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(input->id));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString(ENOS_DEFAULT_REQUEST_VERSION));
    cJSON_AddItemToObject(root, "params", obj1);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString(ENOS_METHOD_DEVICE_GET_TSL));
    
    char *data_ret = cJSON_Print(root);
    int data_len_ret = 0;
    if(data_ret == NULL)
    {
        data_len_ret = 0;
    }
    else
    {
        data_len_ret = strlen(data_ret);
    }
    *data = data_ret;
    *data_len = data_len_ret;
    
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_get_tsl_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_get_tsl_output **output)
{
    if((data == NULL) || (data_len <= 0) || (output == NULL) || (info_ex == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int data_temp_max = data_len + 1;
    char *data_temp = (char *)malloc(data_temp_max);
    if(data_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]data_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(data_temp, 0, data_temp_max);
    memcpy(data_temp, data, data_len);
    
    struct enos_dev_get_tsl_output *output_ret = (struct enos_dev_get_tsl_output *)malloc(sizeof(struct enos_dev_get_tsl_output));
    if(output_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data_temp);
        return -1;
    }
    memset(output_ret, 0, sizeof(struct enos_dev_get_tsl_output));
    
    cJSON *item = NULL;
    cJSON *item1 = NULL;
    int len_temp = 0;
    char *char_temp = NULL;
    int int_temp = 0;
    int ret = 0;
    cJSON *root = cJSON_Parse(data_temp);
    if(root == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cJSON_Parse error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_get_tsl_output(output_ret);
        free(data_temp);
        return -1;
    }
    
    item = cJSON_GetObjectItem(root, "data");
    if(item != NULL)
    {
        output_ret->data = cJSON_Print(item);
    }
    
    ret = parse_enos_dev_get_tsl_data_response_info_ex(info_ex, &(output_ret->info));
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]parse_enos_dev_get_tsl_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_get_tsl_output(output_ret);
        free(data_temp);
        cJSON_Delete(root);
        return -1;
    }
    
    item1 = cJSON_GetObjectItem(root, "id");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->id);
        char_temp = (char *)(output_ret->id);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    item1 = cJSON_GetObjectItem(root, "code");
    if(item1 != NULL)
    {
        int_temp = 0;
        int_temp = item1->valueint;
        output_ret->code = int_temp;
    }
    
    item1 = cJSON_GetObjectItem(root, "message");
    if(item1 != NULL)
    {
        len_temp = sizeof(output_ret->message);
        char_temp = (char *)(output_ret->message);
        snprintf(char_temp, len_temp, "%s", (char *)(item1->valuestring));
    }
    
    *output = output_ret;
    
    free(data_temp);
    cJSON_Delete(root);
    
    return 0;
}

extern int parse_enos_dev_get_tsl_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_get_tsl_output_info_ex **info)
{
    if(info_ex == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int topic_len = info_ex->topic_len;
    if(topic_len <= 0)
    {
        topic_len = strlen(info_ex->topic_name);
    }
    int topic_name_max = topic_len + 1;
    
    char *topic_name = (char *)malloc(topic_name_max);
    if(topic_name == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(topic_name, 0, topic_name_max);
    memcpy(topic_name, info_ex->topic_name, topic_len);
    
    struct enos_dev_get_tsl_output_info_ex *info_ret = NULL;
    info_ret = (struct enos_dev_get_tsl_output_info_ex *)malloc(sizeof(struct enos_dev_get_tsl_output_info_ex));
    if(info_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]info_ret malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(topic_name);
        return -1;
    }
    memset(info_ret, 0, sizeof(struct enos_dev_get_tsl_output_info_ex));
    
    char *ptr1 = strstr(topic_name, ENOS_TOPIC_PREFIX1);
    if((ptr1 == NULL) || (ptr1 != topic_name) || (topic_len <= strlen(ENOS_TOPIC_PREFIX1) + 1))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]topic_name=%s error(file=%s, function=%s, line=%d)\n", topic_name, __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_get_tsl_output_info_ex(info_ret);
        free(topic_name);
        return -1;
    }
    
    ptr1 += strlen(ENOS_TOPIC_PREFIX1) + 1;
    
    char str_temp1[2048];
    memset(str_temp1, 0, sizeof(str_temp1));
    char str_temp2[2048];
    memset(str_temp2, 0, sizeof(str_temp2));
    sscanf(ptr1, "%[^/]/%[^/]", str_temp1, str_temp2);
    snprintf(info_ret->dev_product_key, sizeof(info_ret->dev_product_key), "%s", str_temp1);
    snprintf(info_ret->dev_device_key, sizeof(info_ret->dev_device_key), "%s", str_temp2);
    
    *info = info_ret;
    free(topic_name);
    
    return 0;
}

extern int free_enos_dev_get_tsl_output(struct enos_dev_get_tsl_output *output)
{
    if(output == NULL)
    {
        return 0;
    }
    
    if(output->data != NULL)
    {
        free(output->data);
    }
    
    if(output->info != NULL)
    {
        free_enos_dev_get_tsl_output_info_ex(output->info);
    }
    
    free(output);
    
    return 0;
}

extern int free_enos_dev_get_tsl_output_info_ex(struct enos_dev_get_tsl_output_info_ex *info)
{
    free(info);
    return 0;
}

extern int enos_dev_get_tsl_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_get_tsl_callback cb, void *user)
{
    if(enos_mas_p == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    pthread_mutex_lock(&(enos_mas_p->callback_mutex));
    enos_mas_p->dev_get_tsl_callback = (void *)cb;
    enos_mas_p->dev_get_tsl_callback_user = user;
    pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    
    return 0;
}

extern int enos_dev_get_tsl_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_get_tsl_callback cb, void *user)
{
    if((enos_mas_p == NULL) || (dev_product_key == NULL) || (dev_device_key == NULL) || (input == NULL) || (input_len <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if((cb == NULL) && (user == NULL))
    {
        ;
    }
    else
    {
        pthread_mutex_lock(&(enos_mas_p->callback_mutex));
        if(cb != NULL)
        {
            enos_mas_p->dev_get_tsl_callback = (void *)cb;
        }
        if(user != NULL)
        {
            enos_mas_p->dev_get_tsl_callback_user = user;
        }
        pthread_mutex_unlock(&(enos_mas_p->callback_mutex));
    }
    
    int ret = 0;
    char topic_request[512];
    memset(topic_request, 0, sizeof(topic_request));
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_request, sizeof(topic_request), "%s/%s/%s/%s", ENOS_TOPIC_PREFIX1, dev_product_key, dev_device_key, ENOS_TOPIC_SUFFIX_DEVICE_GET_TSL);
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic_request, ENOS_TOPIC_REPLY1);
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn_local(enos_mas_p, topic_request, input, input_len, 0);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_mqtt_api_send_normal_syn_local error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}


