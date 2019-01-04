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

#ifndef ENOS_MQTT_API_H
#define ENOS_MQTT_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "enos_common.h"
#include "mqtt/MQTTClient.h"

//default mqtt sign method
#define ENOS_DEFAULT_SIGN_METHOD "hmacsha1"

//default mqtt reliable
#define ENOS_DEFAULT_RELIABLE 0

//default mqtt maxInflightMessages
#define ENOS_DEFAULT_MAXINFLIGHTMESSAGES 2000

//default main table elements number of wait_delivery_hash
#define ENOS_DEFAULT_WAIT_DELIVERY_HASH_MAX 4000

//default elements maximum preservation time of wait_delivery_hash(ms)
#define ENOS_DEFAULT_WAIT_DELIVERY_HASH_TIMEOUT 120000

//default max timeout of syn function(ms)
#define ENOS_DEFAULT_MAX_SYN_WAIT_TIME 30000

//default qos when subscribing a topic 
#define ENOS_DEFAULT_TOPIC_SUBSCRIBE_QOS 1

//default version of mqtt request
#define ENOS_DEFAULT_REQUEST_VERSION "1.0"

#define ENOS_TOPIC_PREFIX1 "/sys"
#define ENOS_TOPIC_PREFIX2 "/ext/session"
#define ENOS_TOPIC_REPLY1 "_reply"

//use ENOS_TOPIC_PREFIX1-->
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_REGISTER "thing/device/register"
#define ENOS_METHOD_SUB_DEVICE_REGISTER "thing.device.register"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_ADD_TOPO "thing/topo/add"
#define ENOS_METHOD_SUB_DEVICE_ADD_TOPO "thing.topo.add"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_DEL_TOPO "thing/topo/delete"
#define ENOS_METHOD_SUB_DEVICE_DEL_TOPO "thing.topo.delete"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_GET_TOPO "thing/topo/get"
#define ENOS_METHOD_SUB_DEVICE_GET_TOPO "thing.topo.get"
#define ENOS_TOPIC_SUFFIX_DEVICE_TAG_UPDATE "thing/tag/update"
#define ENOS_METHOD_DEVICE_TAG_UPDATE "thing.tag.update"
#define ENOS_TOPIC_SUFFIX_DEVICE_TAG_DELETE "thing/tag/delete"
#define ENOS_METHOD_DEVICE_TAG_DELETE "thing.tag.delete"
#define ENOS_TOPIC_SUFFIX_DEVICE_TAG_QUERY "thing/tag/query"
#define ENOS_METHOD_DEVICE_TAG_QUERY "thing.tag.query"
#define ENOS_TOPIC_SUFFIX_DEVICE_ATTR_QUERY "thing/attribute/query"
#define ENOS_METHOD_DEVICE_ATTR_QUERY "thing.attribute.query"
#define ENOS_TOPIC_SUFFIX_DEVICE_ATTR_UPDATE "thing/attribute/update"
#define ENOS_METHOD_DEVICE_ATTR_UPDATE "thing.attribute.update"
#define ENOS_TOPIC_SUFFIX_DEVICE_ATTR_DELETE "thing/attribute/delete"
#define ENOS_METHOD_DEVICE_ATTR_DELETE "thing.attribute.delete"
#define ENOS_TOPIC_SUFFIX_DEVICE_POINT_POST "thing/measurepoint/post"
#define ENOS_METHOD_DEVICE_POINT_POST "thing.measurepoint.post"
#define ENOS_TOPIC_SUFFIX1_DEVICE_EVENT_POST "thing/event"
#define ENOS_TOPIC_SUFFIX2_DEVICE_EVENT_POST "post"
#define ENOS_METHOD1_DEVICE_EVENT_POST "thing.event"
#define ENOS_METHOD2_DEVICE_EVENT_POST "post"
#define ENOS_TOPIC_SUFFIX_DEVICE_EVENT_POST_RAW "thing/model/up_raw"
#define ENOS_METHOD_DEVICE_EVENT_POST_RAW "thing.model.up_raw"
#define ENOS_TOPIC_SUFFIX_DEVICE_GET_TSL "thing/tsltemplate/get"
#define ENOS_METHOD_DEVICE_GET_TSL "thing.tsltemplate.get"
//downstream-->
#define ENOS_TOPIC_SUFFIX_DEVICE_POINT_SET "thing/service/measurepoint/set"
#define ENOS_METHOD_DEVICE_POINT_SET "thing.service.measurepoint.set"
#define ENOS_TOPIC_SUFFIX_DEVICE_POINT_GET "thing/service/measurepoint/get"
#define ENOS_METHOD_DEVICE_POINT_GET "thing.service.measurepoint.get"
#define ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE "thing/service"
#define ENOS_METHOD_DEVICE_SERVICE_INOVKE "thing.service"
#define ENOS_TOPIC_SUFFIX_DEVICE_SERVICE_INOVKE_RAW "thing/model/down_raw"
#define ENOS_METHOD_DEVICE_SERVICE_INOVKE_RAW "thing.model.down_raw"
#define ENOS_TOPIC_SUFFIX_DEVICE_DISABLE "thing/disable"
#define ENOS_METHOD_DEVICE_DISABLE "thing.disable"
#define ENOS_TOPIC_SUFFIX_DEVICE_ENABLE "thing/enable"
#define ENOS_METHOD_DEVICE_ENABLE "thing.enable"
#define ENOS_TOPIC_SUFFIX_DEVICE_DELETE "thing/delete"
#define ENOS_METHOD_DEVICE_DELETE "thing.delete"
//<--downstream
//<--use ENOS_TOPIC_PREFIX1

//use ENOS_TOPIC_PREFIX2-->
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGIN "combine/login"
#define ENOS_METHOD_SUB_DEVICE_LOGIN "combine.login"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_LOGOUT "combine/logout"
#define ENOS_METHOD_SUB_DEVICE_LOGOUT "combine.logout"

//downstream-->
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_DISABLE "combine/disable"
#define ENOS_METHOD_SUB_DEVICE_DISABLE "combine.disable"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_ENABLE "combine/enable"
#define ENOS_METHOD_SUB_DEVICE_ENABLE "combine.enable"
#define ENOS_TOPIC_SUFFIX_SUB_DEVICE_DELETE "combine/delete"
#define ENOS_METHOD_SUB_DEVICE_DELETE "combine.delete"
//<--downstream
//<--use ENOS_TOPIC_PREFIX2

//struct ENOS_C_API_DLL_EXPORT enos_string
//{
//    char str[ENOS_GENERAL_BUF_MAX];
//};

//hash table-->
struct ENOS_C_API_DLL_EXPORT enos_hash_value
{
    MQTTClient_deliveryToken token;
    char *delivered_fun_user_ptr;
    void *delivered_fun;
//    char *message_arrived_fun_user_ptr;
//    void *message_arrived_fun;
};

typedef struct enos_hash_value custom_hash_type;
struct custom_hash_element
{
    custom_hash_type value;
    long long int sec;
    int valid_flag;
    struct custom_hash_element *time_next;
	struct custom_hash_element *time_pre;
    struct custom_hash_element *ptr_next;
    struct custom_hash_element *ptr_pre;
};

struct custom_hash_table
{
    int max_num;//hash主表数组元素的最大个数,是2^n
    volatile int count;//当前元素个数
    struct custom_hash_element *time_first;//按接收时间排序时的第一个element
    struct custom_hash_element *time_last;//按接收时间排序时的最后一个element
    struct custom_hash_element *table;//hash主表
};
//<--hash table

//mqtt_api main struct
struct ENOS_C_API_DLL_EXPORT enos_mqtt_api_struct
{
    //mqtt client handle
    MQTTClient client;
    
    //MQTTClient_connectOptions
    MQTTClient_connectOptions client_opts;
    
    //MQTTClient_SSLOptions
    MQTTClient_SSLOptions ssl_opts;
    
    char product_key[ENOS_GENERAL_BUF_MAX];
    char device_key[ENOS_GENERAL_BUF_MAX];
    char device_secret[ENOS_GENERAL_BUF_MAX];
    
    //an null-terminated string specifying the server to which the client will connect
    char address[ENOS_GENERAL_BUF_MAX];
    
    int persistence_type;
    char* persistence_store;
    
    //equal to device_key
    char client_id[ENOS_GENERAL_BUF_MAX];
    
    //timestamp
    long long int time_sec;
    
    //the number of entries in the servers array             
    int multi_server_count;
    
    //an array of null-terminated strings specifying the servers to which the client will connect
    char **servers;
    
    //charset of server
    char cloud_charset[ENOS_GENERAL_BUF_MAX];
    
    //charset of application
    char app_charset[ENOS_GENERAL_BUF_MAX];
    
    char mqtt_client_id[ENOS_GENERAL_BUF_MAX];
    char mqtt_username[ENOS_GENERAL_BUF_MAX];
    char mqtt_passwd[ENOS_GENERAL_BUF_MAX];
    char mqtt_ssl_ca_cert_path[ENOS_PATH_BUF_MAX];
    char mqtt_ssl_local_cert_path[ENOS_PATH_BUF_MAX];
    char mqtt_ssl_local_private_path[ENOS_PATH_BUF_MAX];
    char mqtt_ssl_local_private_passwd[ENOS_GENERAL_BUF_MAX];
    
    //pthread of deal_wait_delivery_hash
    pthread_t pth_deal_wait_delivery_hash;
    
    //lock of deal_wait_delivery_hash
    pthread_mutex_t wait_delivery_hash_mutex;
    
    //wait delivery hash table
    struct custom_hash_table *wait_delivery_hash;
    
    //lock of operate callback
    pthread_mutex_t callback_mutex;
    
    void *connect_lost_callback;
    void *connect_lost_callback_user;
    
    void *message_arrived_callback;
    void *message_arrived_callback_user;
    
    void *delivered_callback;
    void *delivered_callback_user;
    
    void *sub_dev_register_callback;
    void *sub_dev_register_callback_user;
    
    void *sub_dev_add_topo_callback;
    void *sub_dev_add_topo_callback_user;
    
    void *sub_dev_del_topo_callback;
    void *sub_dev_del_topo_callback_user;
    
    void *sub_dev_get_topo_callback;
    void *sub_dev_get_topo_callback_user;
    
    void *sub_dev_login_callback;
    void *sub_dev_login_callback_user;
    
    void *sub_dev_logout_callback;
    void *sub_dev_logout_callback_user;
    
    void *dev_tag_update_callback;
    void *dev_tag_update_callback_user;
    
    void *dev_tag_delete_callback;
    void *dev_tag_delete_callback_user;
    
    void *dev_tag_query_callback;
    void *dev_tag_query_callback_user;
    
    void *dev_attr_update_callback;
    void *dev_attr_update_callback_user;
    
    void *dev_attr_delete_callback;
    void *dev_attr_delete_callback_user;
    
    void *dev_attr_query_callback;
    void *dev_attr_query_callback_user;
    
    void *dev_point_post_callback;
    void *dev_point_post_callback_user;
    
    void *dev_event_post_callback;
    void *dev_event_post_callback_user;
    
    void *dev_event_post_raw_callback;
    void *dev_event_post_raw_callback_user;
    
    void *dev_get_tsl_callback;
    void *dev_get_tsl_callback_user;
    
    //downstream-->
    void *dev_point_set_callback;
    void *dev_point_set_callback_user;
    
    void *dev_point_get_callback;
    void *dev_point_get_callback_user;
    
    void *dev_service_invoke_callback;
    void *dev_service_invoke_callback_user;
    
    void *dev_service_invoke_raw_callback;
    void *dev_service_invoke_raw_callback_user;
    
    void *dev_disable_callback;
    void *dev_disable_callback_user;
    
    void *dev_enable_callback;
    void *dev_enable_callback_user;
    
    void *dev_delete_callback;
    void *dev_delete_callback_user;
    
    void *sub_dev_disable_callback;
    void *sub_dev_disable_callback_user;
    
    void *sub_dev_enable_callback;
    void *sub_dev_enable_callback_user;
    
    void *sub_dev_delete_callback;
    void *sub_dev_delete_callback_user;
    //<--downstream
    

//    void *dev_point_set_reply_callback;
//    void *dev_point_set_reply_callback_user;
//    
//    void *dev_point_get_reply_callback;
//    void *dev_point_get_reply_callback_user;
//    
//    void *dev_service_invoke_reply_callback;
//    void *dev_service_invoke_reply_callback_user;
//    
//    void *dev_service_invoke_raw_reply_callback;
//    void *dev_service_invoke_raw_reply_callback_user;
//    
//    void *dev_disable_reply_callback;
//    void *dev_disable_reply_callback_user;
//    
//    void *dev_enable_reply_callback;
//    void *dev_enable_reply_callback_user;
//    
//    void *dev_delete_reply_callback;
//    void *dev_delete_reply_callback_user;
//    
//    void *sub_dev_disable_reply_callback;
//    void *sub_dev_disable_reply_callback_user;
//    
//    void *sub_dev_enable_reply_callback;
//    void *sub_dev_enable_reply_callback_user;
//    
//    void *sub_dev_delete_reply_callback;
//    void *sub_dev_delete_reply_callback_user;

    
};

//additional information, for example topic and topic_len
struct ENOS_C_API_DLL_EXPORT enos_mqtt_api_info_ex
{
    //if topic_len==0 strlen(topic_name) can be trusted, if topic_len>0 actual length of topic_name is topic_len
    int topic_len;
    
    //topic name
    char *topic_name;
};

/************************************************************
 * name:enos_connect_lost_callback
 * desc:callback function of connect lost
 *
 * para:[in] user              user pointer
 *      [in] cause             NULL
 * return:0         success
 *        <0        fail
 * tips:1.mqtt c api will not manage connect lost, you should manage it yourself
 *      2.you can manage connect lost by setting this callback
 ************************************************************/
typedef void (*enos_connect_lost_callback)(void *user, char *cause);

/************************************************************
 * name:enos_message_arrived_callback
 * desc:callback function of message_arrived
 *
 * para:[in] user              user pointer
 *      [in] topic_name        topic_name
 *      [in] topic_len         length of topic_name
 *      [in] message           MQTTClient_message
 * return:0         success
 *        <0        fail
 * tips:you can deal message_arrived yourself by setting this callback
 ************************************************************/
typedef void (*enos_message_arrived_callback)(void *user, char *topic_name, int topic_len, MQTTClient_message *message);

/************************************************************
 * name:enos_delivered_callback
 * desc:callback function of delivered
 *
 * para:[in] user              user pointer
 *      [in] dt                MQTTClient_deliveryToken
 * return:0         success
 *        <0        fail
 * tips:you can deal delivered yourself by setting this callback
 ************************************************************/
typedef void (*enos_delivered_callback)(void *user, MQTTClient_deliveryToken dt);

/************************************************************
 * name:enos_calc_sign
 * desc:caculate sign when building mqtt request
 *
 * para:[in]  para_value_p        an array of struct enos_para_value
 *      [in]  count               the number of entries in the para_value_p array
 *      [in]  device_secret       device secret
 *      [out] sign                sign result buf
 *      [in]  sign_max            max length of sign result buf
 *      [out] sign_len            actual length of sign result
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_calc_sign(struct enos_para_value *para_value_p, int count, char *device_secret, char *sign, int sign_max, int *sign_len);

/************************************************************
 * name:enos_calc_sign_var
 * desc:caculate sign when building mqtt request(varied parameter)
 *
 * para:[in]  device_secret       device secret
 *      [out] sign                sign result buf
 *      [in]  sign_max            max length of sign result buf
 *      [out] sign_len            actual length of sign result
 *      [in]  para_count          number of parameters
 *      [in]  ...                 parameters
 * return:0         success
 *        <0        fail
 * tips:1. format of varied parameter:[para1 name] [para1 value] [para2 name] [para2 value] ...
 *      2. para_count include name and value, so it is always even
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_calc_sign_var(char *device_secret, char *sign, int sign_max, int *sign_len, int para_count, ...);

/************************************************************
 * name:enos_mqtt_api_init
 * desc:initialize enos_mas_pp
 *
 * para:[out] enos_mas_pp        address of a (struct enos_para_value *) pointer
 *      [in]  address            A null-terminated string specifying the server to which the client will connect
 *      [in]  product_key        product key
 *      [in]  device_key         device key
 *      [in]  device_secret      device secret
 *      [in]  cloud_charset      charset of server
 *      [in]  app_charset        charset of application
 * return:0         success
 *        <0        fail
 * tips:enos_mqtt_api_init will call malloc for *enos_mas_pp,call enos_mqtt_api_uninit when you don't use *enos_mas_p
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_init(struct enos_mqtt_api_struct **enos_mas_pp, char *address, char *product_key, char *device_key, char *device_secret, char *cloud_charset, char *app_charset);

/************************************************************
 * name:enos_mqtt_api_init
 * desc:free enos_mas_p
 *
 * para:[in] enos_mas_p        pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_uninit(struct enos_mqtt_api_struct *enos_mas_p);

/************************************************************
 * name:enos_mqtt_api_set_ssl
 * desc:set ssl parameters
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] ca_cert_path            path of ca cert file
 *      [in] local_cert_path         path of local cert file
 *      [in] local_private_path      path of private key file
 *      [in] local_private_passwd    passwd of private key file
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_ssl(struct enos_mqtt_api_struct *enos_mas_p, char *ca_cert_path, char *local_cert_path, char *local_private_path, char *local_private_passwd);

/************************************************************
 * name:enos_mqtt_api_set_keepAliveInterval
 * desc:set mqtt keepAliveInterval
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] keepAliveInterval       keepAliveInterval(seconds)
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_keepAliveInterval(struct enos_mqtt_api_struct *enos_mas_p, int keepAliveInterval);

/************************************************************
 * name:enos_mqtt_api_set_cleansession
 * desc:set mqtt cleansession
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cleansession            cleansession, this is a boolean value
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_cleansession(struct enos_mqtt_api_struct *enos_mas_p, int cleansession);

/************************************************************
 * name:enos_mqtt_api_set_multi_server
 * desc:set multi mqtt server addresses
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] server_count            the number of entries in the servers array
 *      [in] servers                 an array of null-terminated strings specifying the servers to which the client will connect
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_multi_server(struct enos_mqtt_api_struct *enos_mas_p, int server_count, char **servers);

/************************************************************
 * name:enos_mqtt_api_set_multi_server_var
 * desc:set multi mqtt server addresses(varied parameter)
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] server_count            the number of parameters
 *      [in] servers                 varied parameters specifying the servers to which the client will connect
 * return:0         success
 *        <0        fail
 * tips:format of varied parameter:[server1] [server2] ...
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_multi_server_var(struct enos_mqtt_api_struct *enos_mas_p, int server_count, ...);

/************************************************************
 * name:enos_mqtt_api_set_connect_lost_callback
 * desc:set connect lost callback
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_connect_lost_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_connect_lost_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_connect_lost_callback cb, void *user);

/************************************************************
 * name:enos_mqtt_api_set_message_arrived_callback
 * desc:set message arrived callback
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_message_arrived_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_message_arrived_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_message_arrived_callback cb, void *user);

/************************************************************
 * name:enos_mqtt_api_set_delivered_callback
 * desc:set delivered callback
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_delivered_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_set_delivered_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_delivered_callback cb, void *user);

/************************************************************
 * name:enos_mqtt_api_connect
 * desc:mqtt connect
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME;
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_connect(struct enos_mqtt_api_struct *enos_mas_p, int timeout_ms);

/************************************************************
 * name:enos_mqtt_api_disconnect
 * desc:mqtt disconnect
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME;
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_disconnect(struct enos_mqtt_api_struct *enos_mas_p, int timeout_ms);

/************************************************************
 * name:enos_mqtt_api_send_fast
 * desc:send data fast use qos 0
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] topic                   topic
 *      [in] data                    data need to send
 *      [in] data_len                length of data
 *      [in] need_trans              if need_trans !=0,change charset of data from app charset to cloud charset
 * return:0         success
 *        <0        fail
 * tips:you can change charset youself using enos_char_code_convert in enos_common.h(recommended)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_send_fast(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len, int need_trans);

/************************************************************
 * name:enos_mqtt_api_send_normal_syn
 * desc:send data use qos 1 and waiting for server confirmation
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] topic                   topic
 *      [in] data                    data need to send
 *      [in] data_len                length of data
 *      [in] timeout_ms              timeout(ms)
 *      [in] need_trans              if need_trans !=0,change charset of data from app charset to cloud charset
 * return:0         success
 *        <0        fail
 * tips:1. you can change charset youself using enos_char_code_convert in enos_common.h(recommended)
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_mqtt_api_send_normal_syn(struct enos_mqtt_api_struct *enos_mas_p, char *topic, char *data, int data_len, int timeout_ms, int need_trans);


//Device Registration-->

//input struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_register_input_sub_dev
{
    //sub device product_key(mandatory)
    char sub_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub device device_key(optional)
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_name(optional)
	char sub_device_name[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_desc(optional)
	char sub_device_desc[ENOS_GENERAL_BUF_MAX];

    /**
     * json string of deviceAttributes(optional)
     * for example:
     * {
     *     "color":"red"
     * }
     **/
	char *sub_device_attr;
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_register_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_register_input_sub_dev
	struct enos_sub_dev_register_input_sub_dev *sub_dev;
};

//output struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_register_output_sub_dev
{
    //sub device iot id
    char sub_iot_id[ENOS_GENERAL_BUF_MAX];
    
    //sub device product_key
	char sub_product_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_key
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_secret
	char sub_device_secret[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_register_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_register_output_sub_dev
	struct enos_sub_dev_register_output_sub_dev *sub_dev;
};

/************************************************************
 * name:enos_sub_dev_register_callback
 * desc:callback function used by enos_sub_dev_register
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_register_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_register_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_register_data
 * desc:generate json data of enos_sub_dev_register
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_register_data(struct enos_sub_dev_register_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_register_data_response
 * desc:transform data received from json to struct enos_sub_dev_register_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_register_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_register_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_register_data_response(char *data, int data_len, struct enos_sub_dev_register_output **output);

/************************************************************
 * name:free_enos_sub_dev_register_output
 * desc:free output generated by parse_enos_sub_dev_register_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_register_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_register_output(struct enos_sub_dev_register_output *output);

/************************************************************
 * name:enos_sub_dev_register_set_callback
 * desc:set callback of enos_sub_dev_register
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_register_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_register_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_register_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_register_asyn
 * desc:send enos_sub_dev_register data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_register_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_register_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_register_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_register_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_register_callback cb, void *user);
//<--Device Registration

//Add Topological Relationships of Sub-devices-->
//input struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_add_topo_input_sub_dev
{
    //sub device product_key(mandatory)
    char sub_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub device device_key(mandatory)
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_secret(mandatory)
	char sub_device_secret[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_add_topo_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_add_topo_input_sub_dev
	struct enos_sub_dev_add_topo_input_sub_dev *sub_dev;
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_add_topo_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_sub_dev_add_topo_callback
 * desc:callback function used by enos_sub_dev_add_topo
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_add_topo_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_add_topo_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_add_topo_data
 * desc:generate json data of enos_sub_dev_add_topo
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_add_topo_data(struct enos_sub_dev_add_topo_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_add_topo_data_response
 * desc:transform data received from json to struct enos_sub_dev_add_topo_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_add_topo_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_add_topo_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_add_topo_data_response(char *data, int data_len, struct enos_sub_dev_add_topo_output **output);

/************************************************************
 * name:free_enos_sub_dev_add_topo_output
 * desc:free output generated by parse_enos_sub_dev_add_topo_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_add_topo_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_add_topo_output(struct enos_sub_dev_add_topo_output *output);

/************************************************************
 * name:enos_sub_dev_add_topo_set_callback
 * desc:set callback of enos_sub_dev_add_topo
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_add_topo_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_add_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_add_topo_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_add_topo_asyn
 * desc:send enos_sub_dev_add_topo data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_add_topo_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_add_topo_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_add_topo_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_add_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_add_topo_callback cb, void *user);

//<--Add Topological Relationships of Sub-devices

//Delete Topological Relationships of Sub-devices-->

//input struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_del_topo_input_sub_dev
{
    //sub device product_key(mandatory)
    char sub_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub device device_key(mandatory)
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_del_topo_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_del_topo_input_sub_dev
	struct enos_sub_dev_del_topo_input_sub_dev *sub_dev;
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_del_topo_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_sub_dev_del_topo_callback
 * desc:callback function used by enos_sub_dev_del_topo
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_del_topo_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_del_topo_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_del_topo_data
 * desc:generate json data of enos_sub_dev_del_topo
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_del_topo_data(struct enos_sub_dev_del_topo_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_del_topo_data_response
 * desc:transform data received from json to struct enos_sub_dev_del_topo_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_del_topo_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_del_topo_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_del_topo_data_response(char *data, int data_len, struct enos_sub_dev_del_topo_output **output);

/************************************************************
 * name:free_enos_sub_dev_del_topo_output
 * desc:free output generated by parse_enos_sub_dev_del_topo_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_del_topo_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_del_topo_output(struct enos_sub_dev_del_topo_output *output);

/************************************************************
 * name:enos_sub_dev_register_set_callback
 * desc:set callback of enos_sub_dev_del_topo
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_del_topo_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_del_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_del_topo_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_del_topo_asyn
 * desc:send enos_sub_dev_del_topo data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_del_topo_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_del_topo_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_del_topo_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_del_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_del_topo_callback cb, void *user);

//<--Delete Topological Relationships of Sub-devices


//Get Topological Relationships of Sub-devices-->

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_get_topo_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
};

//output struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_get_topo_output_sub_dev
{
    //sub device product_key
	char sub_product_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_key
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_get_topo_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_register_output_sub_dev
	struct enos_sub_dev_get_topo_output_sub_dev *sub_dev;
};

/************************************************************
 * name:enos_sub_dev_get_topo_callback
 * desc:callback function used by enos_sub_dev_get_topo
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_get_topo_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_get_topo_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_get_topo_data
 * desc:generate json data of enos_sub_dev_get_topo
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_get_topo_data(struct enos_sub_dev_get_topo_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_get_topo_data_response
 * desc:transform data received from json to struct enos_sub_dev_get_topo_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_get_topo_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_get_topo_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_get_topo_data_response(char *data, int data_len, struct enos_sub_dev_get_topo_output **output);

/************************************************************
 * name:free_enos_sub_dev_get_topo_output
 * desc:free output generated by parse_enos_sub_dev_get_topo_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_get_topo_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_get_topo_output(struct enos_sub_dev_get_topo_output *output);

/************************************************************
 * name:enos_sub_dev_get_topo_set_callback
 * desc:set callback of enos_sub_dev_get_topo
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_get_topo_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_get_topo_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_get_topo_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_get_topo_asyn
 * desc:send enos_sub_dev_get_topo data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_get_topo_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_get_topo_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_get_topo_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_get_topo_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_get_topo_callback cb, void *user);

//<--Get Topological Relationships of Sub-devices


//Connect Sub-devices to EnOS Cloud-->

//input struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_login_input_sub_dev
{
    //sub device product_key(mandatory)
    char sub_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub device device_key(mandatory)
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_secret(mandatory)
	char sub_device_secret[ENOS_GENERAL_BUF_MAX];
	
	//Supported value: True. This indicates to clear offline information for all sub-devices, which is information that has not been received by QoS 1
	char clean_session[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_login_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the sub_dev array,must <= 1
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_login_input_sub_dev,only the first one is valid
	struct enos_sub_dev_login_input_sub_dev *sub_dev;
};

//output struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_login_output_sub_dev
{
    //sub device product_key
	char sub_product_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_key
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_login_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_login_output_sub_dev
	struct enos_sub_dev_login_output_sub_dev *sub_dev;
};

/************************************************************
 * name:enos_sub_dev_login_callback
 * desc:callback function used by enos_sub_dev_login
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_login_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_login_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_login_data
 * desc:generate json data of enos_sub_dev_login
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_login_data(struct enos_sub_dev_login_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_login_data_response
 * desc:transform data received from json to struct enos_sub_dev_login_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_login_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_login_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_login_data_response(char *data, int data_len, struct enos_sub_dev_login_output **output);

/************************************************************
 * name:free_enos_sub_dev_login_output
 * desc:free output generated by parse_enos_sub_dev_login_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_login_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_login_output(struct enos_sub_dev_login_output *output);

/************************************************************
 * name:enos_sub_dev_login_set_callback
 * desc:set callback of enos_sub_dev_login
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_login_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_login_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_login_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_login_asyn
 * desc:send enos_sub_dev_login data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_login_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_login_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_login_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_login_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_login_callback cb, void *user);

//<--Connect Sub-devices to EnOS Cloud

//Disconnect Sub-devices from EnOS Cloud-->

//input struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_logout_input_sub_dev
{
    //sub device product_key(mandatory)
    char sub_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub device device_key(mandatory)
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_logout_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the sub_dev array,must <= 1
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_logout_input_sub_dev,only the first one is valid
	struct enos_sub_dev_logout_input_sub_dev *sub_dev;
};

//output struct--sub device struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_logout_output_sub_dev
{
    //sub device product_key
	char sub_product_key[ENOS_GENERAL_BUF_MAX];
	
	//sub device device_key
	char sub_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_logout_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//the number of entries in the sub_dev array
	int sub_dev_count;
	
	//an array of struct enos_sub_dev_logout_output_sub_dev
	struct enos_sub_dev_logout_output_sub_dev *sub_dev;
};

/************************************************************
 * name:enos_sub_dev_logout_callback
 * desc:callback function used by enos_sub_dev_logout
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_logout_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_logout_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_sub_dev_logout_data
 * desc:generate json data of enos_sub_dev_logout
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_sub_dev_logout_data(struct enos_sub_dev_logout_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_sub_dev_logout_data_response
 * desc:transform data received from json to struct enos_sub_dev_logout_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_logout_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_logout_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_logout_data_response(char *data, int data_len, struct enos_sub_dev_logout_output **output);

/************************************************************
 * name:free_enos_sub_dev_logout_output
 * desc:free output generated by parse_enos_sub_dev_logout_data_response
 *
 * para:[in] output            generated by parse_enos_sub_dev_logout_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_logout_output(struct enos_sub_dev_logout_output *output);

/************************************************************
 * name:enos_sub_dev_logout_set_callback
 * desc:set callback of enos_sub_dev_logout
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_logout_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_logout_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_logout_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_logout_asyn
 * desc:send enos_sub_dev_logout data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_sub_dev_logout_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_sub_dev_logout_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_sub_dev_logout_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_logout_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, enos_sub_dev_logout_callback cb, void *user);

//<--Disconnect Sub-devices from EnOS Cloud


//Query Tags-->

//input struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_query_input_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_query_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_tag_query_input_para
	struct enos_dev_tag_query_input_para *para;
};

//output struct--parameters name and value
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_query_output_para
{
    //parameter name
	char para_name[ENOS_GENERAL_BUF_MAX];
	
	//parameter value
	char para_value[ENOS_GENERAL_BUF_MAX];
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_query_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_query_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_tag_query_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_tag_query_output_para
	struct enos_dev_tag_query_output_para *para;
};

/************************************************************
 * name:enos_dev_tag_query_callback
 * desc:callback function used by enos_dev_tag_query
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_tag_query_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_tag_query_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_tag_query_data
 * desc:generate json data of enos_dev_tag_query
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_tag_query_data(struct enos_dev_tag_query_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_tag_query_data_response
 * desc:transform data received from json to struct enos_dev_tag_query_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_tag_query_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_query_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_query_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_query_output **output);

/************************************************************
 * name:parse_enos_dev_tag_query_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_tag_query_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_query_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_query_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_query_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_tag_query_output
 * desc:free output generated by parse_enos_dev_tag_query_data_response
 *
 * para:[in] output            generated by parse_enos_dev_tag_query_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_query_output(struct enos_dev_tag_query_output *output);

/************************************************************
 * name:free_enos_dev_tag_query_output_info_ex
 * desc:free info generated by parse_enos_dev_tag_query_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_tag_query_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_query_output_info_ex(struct enos_dev_tag_query_output_info_ex *info);

/************************************************************
 * name:enos_dev_tag_query_set_callback
 * desc:set callback of enos_dev_tag_query
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_tag_query_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_query_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_query_callback cb, void *user);

/************************************************************
 * name:enos_dev_tag_query_asyn
 * desc:send enos_dev_tag_query data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_tag_query_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_tag_query_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_tag_query_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_query_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_query_callback cb, void *user);

//<--Query Tags


//Report Tags-->

//input struct--parameters name and value
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_update_input_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
    
    //parameter value
    char para_value[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_update_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_tag_update_input_para
	struct enos_dev_tag_update_input_para *para;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_update_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_update_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_tag_update_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	/**
     * json string of dev_tag
     * for example:
     * [
     *     {
     *         "tagKey1": "Temperature",
     *         "tagValue1": "36.8"
     *     },
     *     {
     *         "tagKey2": "aaaaa",
     *         "tagValue2": "bbbbb"
     *     }
     * ]
     **/
	char *data;
};

/************************************************************
 * name:enos_dev_tag_update_callback
 * desc:callback function used by enos_dev_tag_update
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_tag_update_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_tag_update_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_tag_update_data
 * desc:generate json data of enos_dev_tag_update
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_tag_update_data(struct enos_dev_tag_update_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_tag_update_data_response
 * desc:transform data received from json to struct enos_dev_tag_update_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_tag_update_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_update_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_update_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_update_output **output);

/************************************************************
 * name:parse_enos_dev_tag_update_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_tag_update_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_update_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_update_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_update_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_tag_update_output
 * desc:free output generated by parse_enos_dev_tag_update_data_response
 *
 * para:[in] output            generated by parse_enos_dev_tag_update_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_update_output(struct enos_dev_tag_update_output *output);

/************************************************************
 * name:free_enos_dev_tag_update_output_info_ex
 * desc:free info generated by parse_enos_dev_tag_update_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_tag_update_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_update_output_info_ex(struct enos_dev_tag_update_output_info_ex *info);

/************************************************************
 * name:enos_dev_tag_update_set_callback
 * desc:set callback of enos_dev_tag_update
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_tag_update_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_update_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_update_callback cb, void *user);

/************************************************************
 * name:enos_dev_tag_update_asyn
 * desc:send enos_dev_tag_update data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_tag_update_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_tag_update_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_tag_update_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_update_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_update_callback cb, void *user);

//<--Report Tags


//Delete Tags-->

//input struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_delete_input_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_delete_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_tag_delete_input_para
	struct enos_dev_tag_delete_input_para *para;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_delete_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_tag_delete_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_tag_delete_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_dev_tag_delete_callback
 * desc:callback function used by enos_dev_tag_delete
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_tag_delete_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_tag_delete_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_tag_delete_data
 * desc:generate json data of enos_dev_tag_delete
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_tag_delete_data(struct enos_dev_tag_delete_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_tag_delete_data_response
 * desc:transform data received from json to struct enos_dev_tag_delete_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_tag_delete_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_delete_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_delete_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_delete_output **output);

/************************************************************
 * name:parse_enos_dev_tag_delete_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_tag_delete_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_tag_delete_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_tag_delete_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_tag_delete_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_tag_delete_output
 * desc:free output generated by parse_enos_dev_tag_delete_data_response
 *
 * para:[in] output            generated by parse_enos_dev_tag_delete_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_delete_output(struct enos_dev_tag_delete_output *output);

/************************************************************
 * name:free_enos_dev_tag_delete_output_info_ex
 * desc:free info generated by parse_enos_dev_tag_delete_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_tag_delete_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_tag_delete_output_info_ex(struct enos_dev_tag_delete_output_info_ex *info);

/************************************************************
 * name:enos_dev_tag_delete_set_callback
 * desc:set callback of enos_dev_tag_delete
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_tag_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_tag_delete_callback cb, void *user);

/************************************************************
 * name:enos_dev_tag_delete_asyn
 * desc:send enos_dev_tag_delete data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_tag_delete_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_tag_delete_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_tag_delete_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_tag_delete_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_tag_delete_callback cb, void *user);

//<--Delete Tags


//Get Attributes-->

//input struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_query_input_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_query_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_attr_query_input_para
	struct enos_dev_attr_query_input_para *para;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_query_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_query_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_attr_query_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	/**
     * json string of attr
     * for example:
     * {
     *     "attr1": {
     *         "value": 1.0,
     *         "value2": "9"
     *     },
     *     "attr2": 1.02,
     *     "attr3": [1.02, 2.02, 7.93]
     * }
     **/
	char *data;
};

/************************************************************
 * name:enos_dev_attr_query_callback
 * desc:callback function used by enos_dev_attr_query
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_attr_query_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_attr_query_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_attr_query_data
 * desc:generate json data of enos_dev_attr_query
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_attr_query_data(struct enos_dev_attr_query_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_attr_query_data_response
 * desc:transform data received from json to struct enos_dev_attr_query_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_attr_query_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_query_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_query_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_query_output **output);

/************************************************************
 * name:parse_enos_dev_attr_query_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_attr_query_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_query_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_query_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_query_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_attr_query_output
 * desc:free output generated by parse_enos_dev_attr_query_data_response
 *
 * para:[in] output            generated by parse_enos_dev_attr_query_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_query_output(struct enos_dev_attr_query_output *output);

/************************************************************
 * name:free_enos_dev_attr_query_output_info_ex
 * desc:free info generated by parse_enos_dev_attr_query_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_attr_query_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_query_output_info_ex(struct enos_dev_attr_query_output_info_ex *info);

/************************************************************
 * name:enos_dev_attr_query_set_callback
 * desc:set callback of enos_dev_attr_query
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_attr_query_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_query_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_query_callback cb, void *user);

/************************************************************
 * name:enos_dev_attr_query_asyn
 * desc:send enos_dev_attr_query data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_attr_query_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_attr_query_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_attr_query_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_query_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_query_callback cb, void *user);

//<--Get Attributes


//Report Attributes-->

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_update_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    /**
     * json string of attr
     * for example:
     * {
     *     "attr1": {
     *         "value": 1.0,
     *         "value2": "9"
     *     },
     *     "attr2": 1.02,
     *     "attr3": [1.02, 2.02, 7.93]
     * }
     **/
	char *data;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_update_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_update_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_attr_update_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_dev_attr_update_callback
 * desc:callback function used by enos_dev_attr_update
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_attr_update_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_attr_update_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_attr_update_data
 * desc:generate json data of enos_dev_attr_update
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_attr_update_data(struct enos_dev_attr_update_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_attr_update_data_response
 * desc:transform data received from json to struct enos_dev_attr_update_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_attr_update_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_update_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_update_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_update_output **output);

/************************************************************
 * name:parse_enos_dev_attr_update_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_attr_update_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_update_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_update_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_update_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_attr_update_output
 * desc:free output generated by parse_enos_dev_attr_update_data_response
 *
 * para:[in] output            generated by parse_enos_dev_attr_update_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_update_output(struct enos_dev_attr_update_output *output);

/************************************************************
 * name:free_enos_dev_attr_update_output
 * desc:free output generated by parse_enos_dev_attr_update_data_response
 *
 * para:[in] output            generated by parse_enos_dev_attr_update_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_update_output_info_ex(struct enos_dev_attr_update_output_info_ex *info);

/************************************************************
 * name:enos_dev_attr_update_set_callback
 * desc:set callback of enos_dev_attr_update
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_attr_update_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_update_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_update_callback cb, void *user);

/************************************************************
 * name:enos_dev_attr_update_asyn
 * desc:send enos_dev_attr_update data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_attr_update_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_attr_update_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_attr_update_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_update_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_update_callback cb, void *user);

//<--Report Attributes


//Delete Attributes-->

//input struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_delete_input_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
};

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_delete_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_attr_delete_input_para
	struct enos_dev_attr_delete_input_para *para;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_delete_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_attr_delete_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_attr_delete_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_dev_attr_delete_callback
 * desc:callback function used by enos_dev_attr_delete
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_attr_delete_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_attr_delete_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_attr_delete_data
 * desc:generate json data of enos_dev_attr_delete
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_attr_delete_data(struct enos_dev_attr_delete_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_attr_delete_data_response
 * desc:transform data received from json to struct enos_dev_attr_delete_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_attr_delete_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_delete_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_delete_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_delete_output **output);

/************************************************************
 * name:parse_enos_dev_attr_delete_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_attr_delete_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_attr_delete_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_attr_delete_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_attr_delete_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_attr_delete_output
 * desc:free output generated by parse_enos_dev_attr_delete_data_response
 *
 * para:[in] output            generated by parse_enos_dev_attr_delete_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_delete_output(struct enos_dev_attr_delete_output *output);

/************************************************************
 * name:free_enos_dev_attr_delete_output_info_ex
 * desc:free info generated by parse_enos_dev_attr_delete_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_attr_delete_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_attr_delete_output_info_ex(struct enos_dev_attr_delete_output_info_ex *info);

/************************************************************
 * name:enos_dev_attr_delete_set_callback
 * desc:set callback of enos_dev_attr_delete
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_attr_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_attr_delete_callback cb, void *user);

/************************************************************
 * name:enos_dev_attr_delete_asyn
 * desc:send enos_dev_attr_delete data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_attr_delete_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_attr_delete_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_attr_delete_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_attr_delete_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_attr_delete_callback cb, void *user);

//<--Delete Attributes


//Report Device Measuring Points-->

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_point_post_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    /**
     * json string of measuring points
     * for example:
     * {
     *     "Power": {
     *         "value": 1.0,
     *         "quality": 9
     *     },
     *     "temp": 1.02,
     *     "branchCurr": [
     *         "1.02", "2.02", "7.93"
     *     ]
     * }
     **/
	char *data;
	
	//if time_ms_is_valid==0, use server time, if time_ms_is_valid!=0, use time_ms
	int time_ms_is_valid;
	
	//the timestamp for this request topic(ms)
	long long int time_ms;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_point_post_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_point_post_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_point_post_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_dev_point_post_callback
 * desc:callback function used by enos_dev_point_post
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_point_post_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_point_post_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_point_post_data
 * desc:generate json data of enos_dev_point_post
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_point_post_data(struct enos_dev_point_post_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_point_post_data_response
 * desc:transform data received from json to struct enos_dev_point_post_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_point_post_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_post_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_post_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_post_output **output);

/************************************************************
 * name:parse_enos_dev_point_post_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_point_post_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_post_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_post_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_post_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_point_post_output
 * desc:free output generated by parse_enos_dev_point_post_data_response
 *
 * para:[in] output            generated by parse_enos_dev_point_post_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_post_output(struct enos_dev_point_post_output *output);

/************************************************************
 * name:free_enos_dev_point_post_output_info_ex
 * desc:free info generated by parse_enos_dev_point_post_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_point_post_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_post_output_info_ex(struct enos_dev_point_post_output_info_ex *info);

/************************************************************
 * name:enos_dev_point_post_set_callback
 * desc:set callback of enos_dev_point_post
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_point_post_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_post_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_post_callback cb, void *user);

/************************************************************
 * name:enos_dev_point_post_asyn
 * desc:send enos_dev_point_post data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_point_post_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_point_post_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_point_post_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_post_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_point_post_callback cb, void *user);

//<--Report Device Measuring Points


//Report Device Events(Non-Passthrough)-->

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_event_post_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //event id
    char event_id[ENOS_GENERAL_BUF_MAX];
    
    /**
     * json string of event
     * for example:
     * {
     *     "Power": {
     *         "value": 1.0,
     *         "quality": 9
     *     }
     * }
     **/
	char *data;
	
	//if time_ms_is_valid==0, use server time, if time_ms_is_valid!=0, use time_ms
	int time_ms_is_valid;
	
	//the timestamp for this request topic(ms)
	long long int time_ms;
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_event_post_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
	
	//event id
	char event_id[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_event_post_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_event_post_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//return data
	char *data;
};

/************************************************************
 * name:enos_dev_event_post_callback
 * desc:callback function used by enos_dev_event_post
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_event_post_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_event_post_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_event_post_data
 * desc:generate json data of enos_dev_event_post
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_event_post_data(struct enos_dev_event_post_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_event_post_data_response
 * desc:transform data received from json to struct enos_dev_event_post_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_event_post_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_event_post_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_event_post_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_output **output);

/************************************************************
 * name:parse_enos_dev_event_post_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_event_post_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_event_post_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_event_post_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_event_post_output
 * desc:free output generated by parse_enos_dev_event_post_data_response
 *
 * para:[in] output            generated by parse_enos_dev_event_post_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_event_post_output(struct enos_dev_event_post_output *output);

/************************************************************
 * name:free_enos_dev_event_post_output_info_ex
 * desc:free info generated by parse_enos_dev_event_post_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_event_post_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_event_post_output_info_ex(struct enos_dev_event_post_output_info_ex *info);

/************************************************************
 * name:enos_dev_event_post_set_callback
 * desc:set callback of enos_dev_event_post
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_event_post_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_event_post_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_event_post_callback cb, void *user);

/************************************************************
 * name:enos_dev_event_post_asyn
 * desc:send enos_dev_event_post data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] event_id                event id specifying the event which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_event_post_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_event_post_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_event_post_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_event_post_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, char *input, int input_len, enos_dev_event_post_callback cb, void *user);

//<--Report Device Events(Non-Passthrough)


//Report Device Events(Passthrough)-->

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_event_post_raw_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

/************************************************************
 * name:enos_dev_event_post_raw_callback
 * desc:callback function used by enos_dev_event_post_raw
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_event_post_raw_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_event_post_raw_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_event_post_raw_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_event_post_raw_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_event_post_raw_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_event_post_raw_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_event_post_raw_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_event_post_raw_output_info_ex
 * desc:free info generated by parse_enos_dev_event_post_raw_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_event_post_raw_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_event_post_raw_output_info_ex(struct enos_dev_event_post_raw_output_info_ex *info);

/************************************************************
 * name:enos_dev_event_post_raw_set_callback
 * desc:set callback of enos_dev_event_post_raw
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_event_post_raw_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_event_post_raw_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_event_post_raw_callback cb, void *user);

/************************************************************
 * name:enos_dev_event_post_raw_asyn
 * desc:send enos_dev_event_post_raw data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_event_post_raw_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_event_post_raw_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_event_post_raw_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_event_post_raw_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_event_post_raw_callback cb, void *user);

//<--Report Device Events(Passthrough)

//general reply-->

//general reply struct
struct ENOS_C_API_DLL_EXPORT enos_general_reply
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //reply code
    int code;
    
    //reply message
    char message[ENOS_GENERAL_BUF_MAX];
    
    //reply data
	char *data;
};

/************************************************************
 * name:generate_enos_general_reply_data
 * desc:generate json data of enos_general_reply
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_general_reply_data(struct enos_general_reply *input, char **data, int *data_len);

/************************************************************
 * name:generate_enos_general_reply_data_var
 * desc:generate json data of enos_general_reply(variable parameters)
 *
 * para:[in]  id               message id
 *      [in]  code             reply code
 *      [in]  msg              reply message
 *      [in]  data_in          reply data
 *      [out] data             generated json data
 *      [out] data_len         actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_general_reply_data_var(char *id, int code, char *msg, char *data_in, char **data, int *data_len);

//<--general reply

//Set Device Measuring Points-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_point_set_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_point_set_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_point_set_callback_data_info_ex *info;
    
    /**
     * json string of dev_point_set
     * for example:
     * {
     *     "temperature": 30.5
     * }
     **/
	char *data;
};

/************************************************************
 * name:enos_dev_point_set_callback
 * desc:callback function used by enos_dev_point_set
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_point_set_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_point_set_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_point_set_callback_data
 * desc:transform data received from json to struct enos_dev_point_set_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_point_set_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_set_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_set_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_set_callback_data **output);

/************************************************************
 * name:parse_enos_dev_point_set_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_point_set_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_set_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_set_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_set_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_point_set_callback_data
 * desc:free output generated by parse_enos_dev_point_set_callback_data
 *
 * para:[in] output            generated by parse_enos_dev_point_set_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_set_callback_data(struct enos_dev_point_set_callback_data *output);

/************************************************************
 * name:free_enos_dev_point_set_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_point_set_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_point_set_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_set_callback_data_info_ex(struct enos_dev_point_set_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_point_set_set_callback
 * desc:set callback of enos_dev_point_set
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_point_set_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_set_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_set_callback cb, void *user);

/************************************************************
 * name:enos_dev_point_set_register
 * desc:register dev_point_set command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_point_set_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_set_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_point_set_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_point_set_reply_syn
 * desc:send dev_point_set_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_point_set_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);
//<--Set Device Measuring Points


//Get Device Measuring Points-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_point_get_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_dev_point_get_callback_data_para
{
    //parameter name
    char para_name[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_point_get_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_point_get_callback_data_info_ex *info;
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_dev_point_get_callback_data_para
	struct enos_dev_point_get_callback_data_para *para;
};

/************************************************************
 * name:enos_dev_point_get_callback
 * desc:callback function used by enos_dev_point_get
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_point_get_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_point_get_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_point_get_callback_data
 * desc:transform data received from json to struct enos_dev_point_get_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_point_get_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_get_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_get_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_get_callback_data **output);

/************************************************************
 * name:parse_enos_dev_point_get_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_point_get_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_point_get_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_point_get_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_point_get_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_point_get_callback_data
 * desc:free output generated by parse_enos_dev_point_get_callback_data
 *
 * para:[in] output            generated by parse_enos_dev_point_get_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_get_callback_data(struct enos_dev_point_get_callback_data *output);

/************************************************************
 * name:free_enos_dev_point_get_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_point_get_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_point_get_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_point_get_callback_data_info_ex(struct enos_dev_point_get_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_point_get_set_callback
 * desc:set callback of enos_dev_point_get
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_point_get_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_get_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_point_get_callback cb, void *user);

/************************************************************
 * name:enos_dev_point_get_register
 * desc:register dev_point_get command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_point_get_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_point_get_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_point_get_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_point_get_reply_syn
 * desc:send dev_point_get_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_point_get_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);

//<--Get Device Measuring Points


//Invoke Device Services (Non-Passthrough)-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_service_invoke_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
	
	//service id
	char event_id[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_service_invoke_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_service_invoke_callback_data_info_ex *info;
    
    /**
     * json string of dev_service_invoke
     * for example:
     * {
     *     "Power": "on",
     *     "WindState": 2
     * }
     **/
	char *data;
};

/************************************************************
 * name:enos_dev_service_invoke_callback
 * desc:callback function used by enos_dev_service_invoke
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_service_invoke_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_service_invoke_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_service_invoke_callback_data
 * desc:transform data received from json to struct enos_dev_service_invoke_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_service_invoke_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_service_invoke_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_service_invoke_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_callback_data **output);

/************************************************************
 * name:parse_enos_dev_service_invoke_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_service_invoke_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_service_invoke_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_service_invoke_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_service_invoke_callback_data
 * desc:free output generated by parse_enos_dev_service_invoke_callback_data
 *
 * para:[in] output            generated by parse_enos_service_invoke_set_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_service_invoke_callback_data(struct enos_dev_service_invoke_callback_data *output);

/************************************************************
 * name:free_enos_dev_service_invoke_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_service_invoke_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_service_invoke_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_service_invoke_callback_data_info_ex(struct enos_dev_service_invoke_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_service_invoke_set_callback
 * desc:set callback of enos_dev_service_invoke
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_service_invoke_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_service_invoke_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_service_invoke_callback cb, void *user);

/************************************************************
 * name:enos_dev_service_invoke_register
 * desc:register dev_service_invoke command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] event_id                service id specifying the service which the function will operate
 *      [in] cb                      enos_dev_service_invoke_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_service_invoke_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, enos_dev_service_invoke_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_service_invoke_reply_syn
 * desc:send dev_service_invoke_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] event_id                service id specifying the service which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_service_invoke_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *event_id, char *input, int input_len, int timeout_ms);
//<--Invoke Device Services (Non-Passthrough)

//Invoke Device Services (Passthrough)-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_service_invoke_raw_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

/************************************************************
 * name:enos_dev_service_invoke_raw_callback
 * desc:callback function used by enos_service_invoke_raw
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_service_invoke_raw_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_service_invoke_raw_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_service_invoke_raw_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_service_invoke_raw_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_service_invoke_raw_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_service_invoke_raw_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_service_invoke_raw_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_service_invoke_raw_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_service_invoke_raw_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_service_invoke_raw_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_service_invoke_raw_callback_data_info_ex(struct enos_dev_service_invoke_raw_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_service_invoke_raw_set_callback
 * desc:set callback of enos_dev_service_invoke_raw
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_service_invoke_raw_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_service_invoke_raw_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_service_invoke_raw_callback cb, void *user);

/************************************************************
 * name:enos_dev_service_invoke_raw_register
 * desc:register dev_service_invoke_raw command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_service_invoke_raw_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_service_invoke_raw_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_service_invoke_raw_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_service_invoke_raw_reply_syn
 * desc:send dev_service_invoke_raw_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_service_invoke_raw_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);
//<--Invoke Device Services (Passthrough)


//Disable Devices-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_disable_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_disable_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_disable_callback_data_info_ex *info;
    
    //data
	char *data;
};

/************************************************************
 * name:enos_dev_disable_callback
 * desc:callback function used by enos_dev_disable
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_disable_set_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_disable_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_disable_callback_data
 * desc:transform data received from json to struct enos_dev_disable_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_disable_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_disable_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_disable_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_disable_callback_data **output);

/************************************************************
 * name:parse_enos_dev_disable_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_disable_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_disable_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_disable_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_disable_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_disable_callback_data
 * desc:free output generated by parse_enos_dev_disable_callback_data
 *
 * para:[in] output            generated by parse_enos_dev_disable_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_disable_callback_data(struct enos_dev_disable_callback_data *output);

/************************************************************
 * name:free_enos_dev_disable_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_disable_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_disable_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_disable_callback_data_info_ex(struct enos_dev_disable_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_disable_set_callback
 * desc:set callback of enos_dev_disable
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_disable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_disable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_disable_callback cb, void *user);

/************************************************************
 * name:enos_dev_disable_register
 * desc:register dev_disable command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_disable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_disable_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_disable_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_disable_reply_syn
 * desc:send dev_disable_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_disable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);
//<--Disable Devices

//Enable Devices-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_enable_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_enable_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_enable_callback_data_info_ex *info;
    
    //data
	char *data;
};

/************************************************************
 * name:enos_dev_enable_callback
 * desc:callback function used by enos_dev_enable
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_enable_set_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_enable_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_enable_callback_data
 * desc:transform data received from json to struct enos_dev_enable_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_enable_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_enable_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_enable_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_enable_callback_data **output);

/************************************************************
 * name:parse_enos_dev_enable_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_enable_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_enable_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_enable_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_enable_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_enable_callback_data
 * desc:free output generated by parse_enos_dev_enable_callback_data
 *
 * para:[in] output            generated by parse_enos_dev_enable_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_enable_callback_data(struct enos_dev_enable_callback_data *output);

/************************************************************
 * name:free_enos_dev_enable_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_enable_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_enable_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_enable_callback_data_info_ex(struct enos_dev_enable_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_enable_set_callback
 * desc:set callback of enos_dev_enable
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_enable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_enable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_enable_callback cb, void *user);

/************************************************************
 * name:enos_dev_enable_register
 * desc:register dev_enable command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_enable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_enable_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_enable_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_enable_reply_syn
 * desc:send dev_enable_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_enable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);
//<--Enable Devices


//Delete Devices-->

//callback data struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_delete_callback_data_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_dev_delete_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_delete_callback_data_info_ex *info;
    
    //data
	char *data;
};

/************************************************************
 * name:enos_dev_delete_callback
 * desc:callback function used by enos_dev_delete
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_delete_set_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_delete_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_dev_delete_callback_data
 * desc:transform data received from json to struct enos_dev_delete_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_delete_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_delete_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_delete_callback_data(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_delete_callback_data **output);

/************************************************************
 * name:parse_enos_dev_delete_callback_data_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_delete_callback_data_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_delete_callback_data_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_delete_callback_data_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_delete_callback_data_info_ex **info);

/************************************************************
 * name:free_enos_dev_delete_callback_data
 * desc:free output generated by parse_enos_dev_delete_callback_data
 *
 * para:[in] output            generated by parse_enos_dev_delete_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_delete_callback_data(struct enos_dev_delete_callback_data *output);

/************************************************************
 * name:free_enos_dev_delete_callback_data_info_ex
 * desc:free info generated by parse_enos_dev_delete_callback_data_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_delete_callback_data_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_delete_callback_data_info_ex(struct enos_dev_delete_callback_data_info_ex *info);

/************************************************************
 * name:enos_dev_delete_set_callback
 * desc:set callback of enos_dev_delete
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_delete_callback cb, void *user);

/************************************************************
 * name:enos_dev_delete_register
 * desc:register dev_delete command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] cb                      enos_dev_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_delete_register(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, enos_dev_delete_callback cb, void *user);

/************************************************************
 * name:enos_send_dev_delete_reply_syn
 * desc:send dev_delete_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_dev_delete_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, int timeout_ms);
//<--Delete Devices

//Disable Sub-devices-->

//callback data struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_disable_callback_data_para
{
    //sub-device product_key
    char sub_dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub-device device_key
    char sub_dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_disable_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_sub_dev_disable_callback_data_para
	struct enos_sub_dev_disable_callback_data_para *para;
};

/************************************************************
 * name:enos_sub_dev_disable_callback
 * desc:callback function used by enos_sub_dev_disable
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_disable_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_disable_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_sub_dev_disable_callback_data
 * desc:transform data received from json to struct enos_sub_dev_disable_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_disable_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_disable_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_disable_callback_data(char *data, int data_len, struct enos_sub_dev_disable_callback_data **output);

/************************************************************
 * name:free_enos_sub_dev_disable_callback_data
 * desc:free output generated by parse_enos_sub_dev_disable_callback_data
 *
 * para:[in] output            generated by parse_enos_sub_dev_disable_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_disable_callback_data(struct enos_sub_dev_disable_callback_data *output);

/************************************************************
 * name:enos_sub_dev_disable_set_callback
 * desc:set callback of enos_sub_dev_disable
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_disable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_disable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_disable_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_disable_register
 * desc:register sub_dev_disable command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_disable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_disable_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_disable_callback cb, void *user);

/************************************************************
 * name:enos_send_sub_dev_disable_reply_syn
 * desc:send sub_dev_disable_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_sub_dev_disable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms);
//<--Disable Sub-devices


//Enable Sub-devices-->

//callback data struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_enable_callback_data_para
{
    //sub-device product_key
    char sub_dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub-device device_key
    char sub_dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_enable_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_sub_dev_enable_callback_data_para
	struct enos_sub_dev_enable_callback_data_para *para;
};

/************************************************************
 * name:enos_sub_dev_enable_callback
 * desc:callback function used by enos_sub_dev_enable
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_enable_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_enable_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_sub_dev_enable_callback_data
 * desc:transform data received from json to struct enos_sub_dev_enable_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_enable_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_enable_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_enable_callback_data(char *data, int data_len, struct enos_sub_dev_enable_callback_data **output);

/************************************************************
 * name:free_enos_sub_dev_enable_callback_data
 * desc:free output generated by parse_enos_sub_dev_enable_callback_data
 *
 * para:[in] output            generated by parse_enos_sub_dev_enable_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_enable_callback_data(struct enos_sub_dev_enable_callback_data *output);

/************************************************************
 * name:enos_sub_dev_enable_set_callback
 * desc:set callback of enos_sub_dev_enable
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_enable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_enable_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_enable_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_enable_register
 * desc:register sub_dev_enable command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_enable_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_enable_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_enable_callback cb, void *user);

/************************************************************
 * name:enos_send_sub_dev_enable_reply_syn
 * desc:send sub_dev_enable_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_sub_dev_enable_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms);
//<--Enable Sub-devices


//Delete Sub-devices-->

//callback data struct--parameters
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_delete_callback_data_para
{
    //sub-device product_key
    char sub_dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //sub-device device_key
    char sub_dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//callback data struct
struct ENOS_C_API_DLL_EXPORT enos_sub_dev_delete_callback_data
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //the number of entries in the para array
	int para_count;
	
	//an array of struct enos_sub_dev_delete_callback_data_para
	struct enos_sub_dev_delete_callback_data_para *para;
};

/************************************************************
 * name:enos_sub_dev_delete_callback
 * desc:callback function used by enos_sub_dev_delete
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_sub_dev_delete_callback_data to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_sub_dev_delete_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:parse_enos_sub_dev_delete_callback_data
 * desc:transform data received from json to struct enos_sub_dev_delete_callback_data
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [out] output            struct enos_sub_dev_delete_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_sub_dev_delete_callback_data(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_sub_dev_delete_callback_data(char *data, int data_len, struct enos_sub_dev_delete_callback_data **output);

/************************************************************
 * name:free_enos_sub_dev_delete_callback_data
 * desc:free output generated by parse_enos_sub_dev_delete_callback_data
 *
 * para:[in] output            generated by parse_enos_sub_dev_delete_callback_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_sub_dev_delete_callback_data(struct enos_sub_dev_delete_callback_data *output);

/************************************************************
 * name:enos_sub_dev_delete_set_callback
 * desc:set callback of enos_sub_dev_delete
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_delete_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_delete_callback cb, void *user);

/************************************************************
 * name:enos_sub_dev_delete_register
 * desc:register sub_dev_delete command
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_sub_dev_delete_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:mqtt server will register it for client automatically, you don't have to call this function manually
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sub_dev_delete_register(struct enos_mqtt_api_struct *enos_mas_p, enos_sub_dev_delete_callback cb, void *user);

/************************************************************
 * name:enos_send_sub_dev_delete_reply_syn
 * desc:send sub_dev_delete_reply data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] timeout_ms              timeout(ms)
 * return:0         success
 *        <0        fail
 * tips:1. you can use generate_enos_general_reply_data/generate_enos_general_reply_data_var to generate input, or you can generate it yourself
 *      2. if timeout_ms < 0 or timeout_ms > ENOS_DEFAULT_MAX_SYN_WAIT_TIME, timeout_ms will be set to ENOS_DEFAULT_MAX_SYN_WAIT_TIME
 *      3. if timeout_ms == 0, it will send data use qos 1 and return immediately(not waiting for server confirmation)
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_send_sub_dev_delete_reply_syn(struct enos_mqtt_api_struct *enos_mas_p, char *input, int input_len, int timeout_ms);
//<--Delete Sub-devices


//get tsl model-->

//input struct
struct ENOS_C_API_DLL_EXPORT enos_dev_get_tsl_input
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
};

//output struct--additional information
struct ENOS_C_API_DLL_EXPORT enos_dev_get_tsl_output_info_ex
{
    //device product_key
    char dev_product_key[ENOS_GENERAL_BUF_MAX];
    
    //device device_key
	char dev_device_key[ENOS_GENERAL_BUF_MAX];
};

//output struct
struct ENOS_C_API_DLL_EXPORT enos_dev_get_tsl_output
{
    //message id
    char id[ENOS_GENERAL_BUF_MAX];
    
    //additional information
    struct enos_dev_get_tsl_output_info_ex *info;
    
    //return code
	int code;
	
	//return message
	char message[ENOS_GENERAL_BUF_MAX];
	
	//json string of tsl model
	char *data;
};

/************************************************************
 * name:enos_dev_get_tsl_callback
 * desc:callback function used by enos_dev_get_tsl
 *
 * para:[in] user              user pointer
 *      [in] data              data received
 *      [in] data_len          length of data
 *      [in] info_ex           additional information, for example topic and topic_len
 * return:0         success
 *        <0        fail
 * tips:you can use parse_enos_dev_get_tsl_data_response to analyse data, or you can analyse it yourself
 ************************************************************/
typedef void (*enos_dev_get_tsl_callback)(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex);

/************************************************************
 * name:generate_enos_dev_get_tsl_data
 * desc:generate json data of enos_dev_get_tsl
 *
 * para:[in]  input             input information
 *      [out] data              generated json data
 *      [out] data_len          actual length of data
 * return:0         success
 *        <0        fail
 * tips:call free(data) if you don't use data
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int generate_enos_dev_get_tsl_data(struct enos_dev_get_tsl_input *input, char **data, int *data_len);

/************************************************************
 * name:parse_enos_dev_get_tsl_data_response
 * desc:transform data received from json to struct enos_dev_get_tsl_output
 *
 * para:[in]  data              data received
 *      [in]  data_len          length of data
 *      [in]  info_ex           additional information
 *      [out] output            struct enos_dev_get_tsl_output
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_get_tsl_output(output) if you don't use output
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_get_tsl_data_response(char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_get_tsl_output **output);

/************************************************************
 * name:parse_enos_dev_get_tsl_data_response_info_ex
 * desc:transform additional information received from struct enos_mqtt_api_info_ex to struct enos_dev_get_tsl_output_info_ex
 *
 * para:[in]  info_ex         additional information received
 *      [out] info            additional information out
 * return:0         success
 *        <0        fail
 * tips:call free_enos_dev_get_tsl_output_info_ex(info) if you don't use info
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int parse_enos_dev_get_tsl_data_response_info_ex(struct enos_mqtt_api_info_ex *info_ex, struct enos_dev_get_tsl_output_info_ex **info);

/************************************************************
 * name:free_enos_dev_get_tsl_output
 * desc:free output generated by parse_enos_dev_get_tsl_data_response
 *
 * para:[in] output            generated by parse_enos_dev_get_tsl_data_response
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_get_tsl_output(struct enos_dev_get_tsl_output *output);

/************************************************************
 * name:free_enos_dev_get_tsl_output_info_ex
 * desc:free info generated by parse_enos_dev_get_tsl_data_response_info_ex
 *
 * para:[in] info            generated by parse_enos_dev_get_tsl_data_response_info_ex
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int free_enos_dev_get_tsl_output_info_ex(struct enos_dev_get_tsl_output_info_ex *info);

/************************************************************
 * name:enos_dev_get_tsl_set_callback
 * desc:set callback of enos_dev_get_tsl
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] cb                      enos_dev_get_tsl_callback cb
 *      [in] user                    user pointer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_get_tsl_set_callback(struct enos_mqtt_api_struct *enos_mas_p, enos_dev_get_tsl_callback cb, void *user);

/************************************************************
 * name:enos_dev_get_tsl_asyn
 * desc:send enos_dev_get_tsl data to server
 *
 * para:[in] enos_mas_p              pointer of struct enos_mqtt_api_struct initialized by enos_mqtt_api_init
 *      [in] dev_product_key         device product_key specifying the device which the function will operate
 *      [in] dev_device_key          device device_key specifying the device which the function will operate
 *      [in] input                   data to be sent
 *      [in] input_len               length of input
 *      [in] cb                      cb!=NULL:override operation of enos_dev_get_tsl_set_callback, cb==NULL:not override(recommended)
 *      [in] user                    user!=NULL:override operation of enos_dev_get_tsl_set_callback, user==NULL:not override(recommended)
 * return:0         success
 *        <0        fail
 * tips:you can use generate_enos_dev_get_tsl_data to generate input, or you can generate it yourself
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_dev_get_tsl_asyn(struct enos_mqtt_api_struct *enos_mas_p, char *dev_product_key, char *dev_device_key, char *input, int input_len, enos_dev_get_tsl_callback cb, void *user);

//<--get tsl model

#ifdef __cplusplus
}
#endif

#endif
