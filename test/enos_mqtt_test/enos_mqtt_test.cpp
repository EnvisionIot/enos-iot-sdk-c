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

#include "enos_mqtt_test.h"

static int myprintf(void *print_arg, int print_level, const char *format, ...);

static int myprintf(void *print_arg, int print_level, const char *format, ...)
{
    if(print_level > ENOS_MQTT_TEST_CURRENT_LOG_LEVEL)
    {
        return -1;
    }

    char sprint_buf[8192];
    va_list args;
    int n;
    va_start(args, format);
    n = vsnprintf(sprint_buf, sizeof(sprint_buf), format, args);
    va_end(args);
    
    if(n < 0)
    {
        return -1;
    }
    else if(n > (int)(sizeof(sprint_buf)))
    {
        int buf_max = n;
        char *sprint_buf_temp = (char *)malloc(buf_max);
        if(sprint_buf_temp == NULL)
        {
            return -1;
        }
        va_start(args, format);
        n = vsnprintf(sprint_buf_temp, buf_max, format, args);
        va_end(args);
        
        if((n < 0) || (n > buf_max))
        {
            free(sprint_buf_temp);
            return -1;
        }
        
        #if defined(_WIN32) || defined(WIN32)
            _write(_fileno(stdout), sprint_buf_temp, n);
        #else
            write(STDOUT_FILENO, sprint_buf_temp, n);
        #endif
        free(sprint_buf_temp);
        return n;
    }
    else
    {
        #if defined(_WIN32) || defined(WIN32)
            _write(_fileno(stdout), sprint_buf, n);
        #else
            write(STDOUT_FILENO, sprint_buf, n);
        #endif
        return n;
    }
}

//static int show_help(char *program_name)
//{
//    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]usage:<%s> <mode> <rsa_key_path> <file_src_path> <file_dst_path>\n", program_name);
//    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]mode=0 encrypt, mode=1 decrypt\n");
//    
//    return 0;
//}

//enos_calc_sign calc mqtt sign
int enos_calc_sign_test()
{
    struct enos_para_value *para_value_p = NULL;
    int count = 10;
    para_value_p = (struct enos_para_value *)malloc(sizeof(struct enos_para_value) * count);
    if(para_value_p == NULL)
    {
        printf("para_value_p malloc error\n");
        return -1;
    }
    memset(para_value_p, 0, sizeof(struct enos_para_value) * count);
    
    char random_ptr[5];
    unsigned char random_value = 0;
    
    int ii = 0;
    int jj = 0;
    int ret = 0;
    printf("src:\n");
    for(ii = 0; ii < count; ii++)
    {
        for(jj = 0; jj < 10; jj++)
        {
            enos_generate_random_value(random_ptr, 1);
            random_value = (unsigned char)(random_ptr[0]);
            random_value = random_value % 26;
            para_value_p[ii].para_name[jj] = 'a' + random_value;
            
            enos_generate_random_value(random_ptr, 1);
            random_value = (unsigned char)(random_ptr[0]);
            random_value = random_value % 26;
            para_value_p[ii].para_value[jj] = 'a' + random_value;
        }
        printf("ii = %03d, para_name=%s, para_value=%s\n", ii, para_value_p[ii].para_name, para_value_p[ii].para_value);
    }
    
    char device_secret[128];
    memset(device_secret, 0, sizeof(device_secret));
    for(jj = 0; jj < 10; jj++)
    {
        enos_generate_random_value(random_ptr, 1);
        random_value = (unsigned char)(random_ptr[0]);
        random_value = random_value % 26;
        device_secret[jj] = 'a' + random_value;
    }
    printf("device_secret=%s\n", device_secret);
    
    char sign[256];
    int sign_max = (int)(sizeof(sign));
    int sign_len = 0;
    memset(sign, 0, sign_max);
    ret = enos_calc_sign(para_value_p, count, device_secret, sign, sign_max, &sign_len);
    if(ret < 0)
    {
        printf("enos_calc_sign error\n");
        free(para_value_p);
        return -1;
    }
    
    printf("sign=%s\n", sign);
    
    free(para_value_p);
    return 0;
}

//enos_calc_sign_var calc mqtt sign
int enos_calc_sign_var_test()
{
    int ret = 0;
    char *device_secret = "deviceSecret";
    char sign[256];
    int sign_max = (int)(sizeof(sign));
    int sign_len = 0;
    memset(sign, 0, sign_max);
    ret = enos_calc_sign_var(device_secret, sign, sign_max, &sign_len, 8, \
    "timestamp", "1524448722000", \
    "deviceKey", "test", \
    "clientId", "123", \
    "productKey", "123");
    if(ret < 0)
    {
        printf("enos_calc_sign_var error\n");
        return -1;
    }
    
    printf("enos_calc_sign_var sign=%s\n", sign);
    
    return 0;
}

//mqtt connect
void enos_mqtt_api_connect_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

int enos_mqtt_api_connect_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_mqtt_api_connect_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_sleep(5000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Device Registration
void enos_sub_dev_register_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_register_asyn_test_reg_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_register_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_register_output *output = NULL;
    int ret = parse_enos_sub_dev_register_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_register_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, sub_dev_count=%d\n", output->id, output->code, output->message, output->sub_dev_count);
    int ii = 0;
    for(ii = 0; ii < output->sub_dev_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "dev device(%d):sub_iot_id=%s, sub_product_key=%s, sub_device_key=%s, sub_device_secret=%s\n", \
            ii, \
            output->sub_dev[ii].sub_iot_id, \
            output->sub_dev[ii].sub_product_key, \
            output->sub_dev[ii].sub_device_key, \
            output->sub_dev[ii].sub_device_secret);
    }
    
    free_enos_sub_dev_register_output(output);
}

int enos_sub_dev_register_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_register_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_register_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_register_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 2;
    input.sub_dev = (struct enos_sub_dev_register_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_register_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_register_input_sub_dev) * input.sub_dev_count);
    int ii = 0;
    for(ii = 0; ii < input.sub_dev_count; ii++)
    {
        snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
        snprintf(input.sub_dev[ii].sub_device_name, sizeof(input.sub_dev[ii].sub_device_name), "zhangyang_dev_device_name%d", ii);
        snprintf(input.sub_dev[ii].sub_device_desc, sizeof(input.sub_dev[ii].sub_device_desc), "zhangyang_dev_device_desc%d", ii);
    }
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_register_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_register_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *reg_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_register_set_callback(enos_mas_p, enos_sub_dev_register_asyn_test_reg_cb, reg_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_register_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    ret = enos_sub_dev_register_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_register_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    free(input.sub_dev);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Add Topological Relationships of Sub-devices
void enos_sub_dev_add_topo_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_add_topo_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_add_topo_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_add_topo_output *output = NULL;
    int ret = parse_enos_sub_dev_add_topo_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_add_topo_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, data=%s\n", output->id, output->code, output->message, output->data);
    
    free_enos_sub_dev_add_topo_output(output);
}

int enos_sub_dev_add_topo_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_add_topo_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_add_topo_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_add_topo_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 2;
    input.sub_dev = (struct enos_sub_dev_add_topo_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_add_topo_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_add_topo_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "NR27DP8uI1MT8vWKunWQ");
    ii++;
    
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "q5XEil6Ndy");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "HWmhQzMSPuXGfrwTyzdS");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_add_topo_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_add_topo_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_add_topo_set_callback(enos_mas_p, enos_sub_dev_add_topo_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_add_topo_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    ret = enos_sub_dev_add_topo_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_add_topo_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.sub_dev);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//enos_mqtt_api_send_normal_syn send data
void enos_mqtt_api_send_normal_syn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_mqtt_api_send_normal_syn_test_asyn_cb(void *user, char *topic_name, int topic_len, MQTTClient_message *message)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = message->payloadlen + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, message->payload, message->payloadlen);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", message->payloadlen, data_str);
    free(data_str);
}

int enos_mqtt_api_send_normal_syn_test_get_data(char **data, int *data_len)
{
    cJSON *root = cJSON_CreateObject();
//    cJSON *params_array = cJSON_CreateArray();
    cJSON *array_obj = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "id", cJSON_CreateString("1234"));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString("1.0"));
    cJSON_AddItemToObject(root, "params", array_obj);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString("combine.login"));
    
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
    for(ii = 0; ii < 1; ii++)
    {
        
//        array_obj = cJSON_CreateObject();
//        cJSON_AddItemToArray(params_array, array_obj);

        cJSON_AddItemToObject(array_obj, "clientId", cJSON_CreateString("LSbyVxjFX9"));
        cJSON_AddItemToObject(array_obj, "productKey", cJSON_CreateString("BP3X1BTv"));
        cJSON_AddItemToObject(array_obj, "deviceKey", cJSON_CreateString("LSbyVxjFX9"));
        cJSON_AddItemToObject(array_obj, "timestamp", cJSON_CreateString(time_sec_str));
        cJSON_AddItemToObject(array_obj, "signMethod", cJSON_CreateString("hmacsha1"));
        cJSON_AddItemToObject(array_obj, "cleanSession", cJSON_CreateString("true"));
        
        sign_len = 0;
        memset(sign, 0, sizeof(sign));
        ret = enos_calc_sign_var("NR27DP8uI1MT8vWKunWQ", sign, (int)(sizeof(sign)), &sign_len, 8, \
            "clientId", "LSbyVxjFX9", \
            "productKey", "BP3X1BTv", \
            "deviceKey", "LSbyVxjFX9", \
            "timestamp", time_sec_str);
        if(ret < 0)
        {
            myprintf(NULL, ENOS_LOG_ERROR, "[ENOS_MQTT_TEST]enos_calc_sign_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
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

int enos_mqtt_api_send_normal_syn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_mqtt_api_send_normal_syn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    ret = enos_mqtt_api_set_message_arrived_callback(enos_mas_p, enos_mqtt_api_send_normal_syn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_set_message_arrived_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    char *topic = "/ext/session/rZWlw2DK/FR3V3sucNW/combine/login";
    char topic_reply[512];
    memset(topic_reply, 0, sizeof(topic_reply));
    
    snprintf(topic_reply, sizeof(topic_reply), "%s%s", topic, "_reply");
    
    ret = MQTTClient_subscribe(enos_mas_p->client, topic_reply, 1);
    if(ret != MQTTCLIENT_SUCCESS)
    {
        myprintf(NULL, ENOS_LOG_ERROR, "[ENOS_MQTT_TEST]MQTTClient_subscribe %s error(file=%s, function=%s, line=%d)\n", topic_reply, __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *data = NULL;
    int data_len = 0;
    ret = enos_mqtt_api_send_normal_syn_test_get_data(&data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_send_normal_syn_test_get_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_send_normal_syn(enos_mas_p, topic, data, data_len, 0, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_send_normal_syn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Connect Sub-devices to EnOS Cloud
void enos_sub_dev_login_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_login_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_login_output *output = NULL;
    int ret = parse_enos_sub_dev_login_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_login_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, sub_dev_count=%d\n", output->id, output->code, output->message, output->sub_dev_count);
    int ii = 0;
    for(ii = 0; ii < output->sub_dev_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "dev device(%d):sub_product_key=%s, sub_device_key=%s\n", \
            ii, \
            output->sub_dev[ii].sub_product_key, \
            output->sub_dev[ii].sub_device_key);
    }
    
    free_enos_sub_dev_login_output(output);
}

int enos_sub_dev_login_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_login_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_login_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_login_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 1;
    input.sub_dev = (struct enos_sub_dev_login_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "NR27DP8uI1MT8vWKunWQ");
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_login_set_callback(enos_mas_p, enos_sub_dev_login_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    //sub_dev1 login
    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    free(data);
    
    snprintf(input.id, sizeof(input.id), "%s", "2345");
    ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "q5XEil6Ndy");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "HWmhQzMSPuXGfrwTyzdS");
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    //sub_dev2 login
    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.sub_dev);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Disconnect Sub-devices from EnOS Cloud
void enos_sub_dev_logout_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_logout_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_logout_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_logout_output *output = NULL;
    int ret = parse_enos_sub_dev_logout_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_logout_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, sub_dev_count=%d\n", output->id, output->code, output->message, output->sub_dev_count);
    int ii = 0;
    for(ii = 0; ii < output->sub_dev_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "dev device(%d):sub_product_key=%s, sub_device_key=%s\n", \
            ii, \
            output->sub_dev[ii].sub_product_key, \
            output->sub_dev[ii].sub_device_key);
    }
    
    free_enos_sub_dev_logout_output(output);
}

int enos_sub_dev_logout_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_logout_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_login_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_login_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 1;
    input.sub_dev = (struct enos_sub_dev_login_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "NR27DP8uI1MT8vWKunWQ");
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_login_set_callback(enos_mas_p, enos_sub_dev_login_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    //login
    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    free(input.sub_dev);
    free(data);
    
    enos_sleep(10000);
    
    //logout
    struct enos_sub_dev_logout_input input2;
    memset(&input2, 0, sizeof(struct enos_sub_dev_logout_input));
    snprintf(input2.id, sizeof(input2.id), "%s", "4567");
    input2.sub_dev_count = 1;
    input2.sub_dev = (struct enos_sub_dev_logout_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_logout_input_sub_dev) * input2.sub_dev_count);
    if(input2.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input2.sub_dev, 0, sizeof(struct enos_sub_dev_logout_input_sub_dev) * input2.sub_dev_count);
    
    ii = 0;
    snprintf(input2.sub_dev[ii].sub_product_key, sizeof(input2.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input2.sub_dev[ii].sub_device_key, sizeof(input2.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    ii++;
    
    char *data2 = NULL;
    int data2_len = 0;
    ret = generate_enos_sub_dev_logout_data(&input2, &data2, &data2_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_logout_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input2.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user2 = "112233";
    
    ret = enos_sub_dev_logout_set_callback(enos_mas_p, enos_sub_dev_logout_asyn_test_asyn_cb, asyn_cb_user2);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_logout_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input2.sub_dev);
        free(data2);
        return -1;
    }

    ret = enos_sub_dev_logout_asyn(enos_mas_p, data2, data2_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_logout_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input2.sub_dev);
        free(data2);
        return -1;
    }
    
    free(input2.sub_dev);
    free(data2);
    
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Query Tags
void enos_dev_tag_query_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_tag_query_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_query_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_tag_query_output *output = NULL;
    int ret = parse_enos_dev_tag_query_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_tag_query_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, para_count=%d\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->para_count);
    int ii = 0;
    for(ii = 0; ii < output->para_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "name=%s, value=%s\n", output->para[ii].para_name, output->para[ii].para_value);
    }
    
    free_enos_dev_tag_query_output(output);
}

int sub_dev_online(struct enos_mqtt_api_struct *enos_mas_p, char *id, char *sub_product_key, char *sub_device_key, char *sub_device_secret)
{
    int ret = 0;

    struct enos_sub_dev_login_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_login_input));
    snprintf(input.id, sizeof(input.id), "%s", id);
    input.sub_dev_count = 1;
    input.sub_dev = (struct enos_sub_dev_login_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", sub_product_key);
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), sub_device_key);
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), sub_device_secret);
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "rtrtrtrt";
    
    ret = enos_sub_dev_login_set_callback(enos_mas_p, enos_sub_dev_login_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(input.sub_dev);
        free(data);
        return -1;
    }

    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    free(input.sub_dev);
    free(data);
    
    return 0;
}

int enos_dev_tag_query_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_tag_query_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_tag_query_input input;
    memset(&input, 0, sizeof(struct enos_dev_tag_query_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.para_count = 2;
    input.para = (struct enos_dev_tag_query_input_para *)malloc(sizeof(struct enos_dev_tag_query_input_para) * input.para_count);
    if(input.para == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.para, 0, sizeof(struct enos_dev_tag_query_input_para) * input.para_count);
    
    int ii = 0;
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "area");
    ii++;
    
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "owner");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_tag_query_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_tag_query_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_tag_query_set_callback(enos_mas_p, enos_dev_tag_query_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_query_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    ret = enos_dev_tag_query_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_query_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.para);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Report Tags
void enos_dev_tag_update_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_tag_update_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_update_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_tag_update_output *output = NULL;
    int ret = parse_enos_dev_tag_update_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_tag_update_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_tag_update_output(output);
}

int enos_dev_tag_update_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_tag_update_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_tag_update_input input;
    memset(&input, 0, sizeof(struct enos_dev_tag_update_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.para_count = 2;
    input.para = (struct enos_dev_tag_update_input_para *)malloc(sizeof(struct enos_dev_tag_update_input_para) * input.para_count);
    if(input.para == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.para, 0, sizeof(struct enos_dev_tag_update_input_para) * input.para_count);
    
    int ii = 0;
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "area");
    snprintf(input.para[ii].para_value, sizeof(input.para[ii].para_value), "%s", "aaa");
    ii++;
    
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "owner");
    snprintf(input.para[ii].para_value, sizeof(input.para[ii].para_value), "%s", "bbb");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_tag_update_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_tag_update_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_tag_update_set_callback(enos_mas_p, enos_dev_tag_update_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_update_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    ret = enos_dev_tag_update_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_update_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.para);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Delete Tags
void enos_dev_tag_delete_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_tag_delete_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_delete_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_tag_delete_output *output = NULL;
    int ret = parse_enos_dev_tag_delete_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_tag_delete_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_tag_delete_output(output);
}

int enos_dev_tag_delete_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_tag_delete_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_tag_delete_input input;
    memset(&input, 0, sizeof(struct enos_dev_tag_delete_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.para_count = 2;
    input.para = (struct enos_dev_tag_delete_input_para *)malloc(sizeof(struct enos_dev_tag_delete_input_para) * input.para_count);
    if(input.para == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.para, 0, sizeof(struct enos_dev_tag_delete_input_para) * input.para_count);
    
    int ii = 0;
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "delete_test1");
    ii++;
    
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "delete_test2");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_tag_delete_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_tag_delete_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_tag_delete_set_callback(enos_mas_p, enos_dev_tag_delete_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_delete_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    ret = enos_dev_tag_delete_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_tag_delete_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.para);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//get tsl model
void enos_dev_get_tsl_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_get_tsl_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_tsl_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_get_tsl_output *output = NULL;
    int ret = parse_enos_dev_get_tsl_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_get_tsl_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_get_tsl_output(output);
}

int enos_dev_get_tsl_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_get_tsl_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_get_tsl_input input;
    memset(&input, 0, sizeof(struct enos_dev_get_tsl_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_get_tsl_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_get_tsl_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_get_tsl_set_callback(enos_mas_p, enos_dev_get_tsl_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_tsl_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    ret = enos_dev_get_tsl_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_tsl_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}


//Get Attributes
void enos_dev_attr_query_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_attr_query_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_attr_query_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_attr_query_output *output = NULL;
    int ret = parse_enos_dev_attr_query_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_attr_query_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_attr_query_output(output);
}

int enos_dev_attr_query_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_attr_query_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_attr_query_input input;
    memset(&input, 0, sizeof(struct enos_dev_attr_query_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.para_count = 2;
    input.para = (struct enos_dev_attr_query_input_para *)malloc(sizeof(struct enos_dev_attr_query_input_para) * input.para_count);
    if(input.para == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.para, 0, sizeof(struct enos_dev_attr_query_input_para) * input.para_count);
    
    int ii = 0;
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "color");
    ii++;
    
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "weight");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_attr_query_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_attr_query_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_attr_query_set_callback(enos_mas_p, enos_dev_attr_query_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_query_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    ret = enos_dev_attr_query_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_query_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.para);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Report Attributes
void enos_dev_attr_update_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_attr_update_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_attr_update_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_attr_update_output *output = NULL;
    int ret = parse_enos_dev_attr_update_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_attr_update_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_attr_update_output(output);
}

int enos_dev_attr_update_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_attr_update_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_attr_update_input input;
    memset(&input, 0, sizeof(struct enos_dev_attr_update_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.data = "{\
         \"color\": \"green\",\
         \"weight\": 1.02\
         }";
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_attr_update_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_attr_update_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_attr_update_set_callback(enos_mas_p, enos_dev_attr_update_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_update_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    ret = enos_dev_attr_update_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_update_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);

    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Delete Attributes
int enos_dev_attr_delete_asyn_test_update(struct enos_mqtt_api_struct *enos_mas_p)
{
    struct enos_dev_attr_update_input input;
    memset(&input, 0, sizeof(struct enos_dev_attr_update_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.data = "{\
         \"attr_delete1\": 100,\
         \"attr_delete2\": 3.33\
         }";
    
    int ret = 0;
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_attr_update_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_attr_update_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_dev_attr_update_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_update_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(data);
        return -1;
    }
    free(data);
    
    return 0;
}

void enos_dev_attr_delete_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_attr_delete_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_attr_delete_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_attr_delete_output *output = NULL;
    int ret = parse_enos_dev_attr_delete_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_attr_delete_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_attr_delete_output(output);
}

int enos_dev_attr_delete_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_attr_delete_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    //update first
    enos_dev_attr_delete_asyn_test_update(enos_mas_p);
    enos_sleep(5000);
    
    struct enos_dev_attr_delete_input input;
    memset(&input, 0, sizeof(struct enos_dev_attr_delete_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.para_count = 2;
    input.para = (struct enos_dev_attr_delete_input_para *)malloc(sizeof(struct enos_dev_attr_delete_input_para) * input.para_count);
    if(input.para == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]para malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.para, 0, sizeof(struct enos_dev_attr_delete_input_para) * input.para_count);
    
    int ii = 0;
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "attr_delete1");
    ii++;
    
    snprintf(input.para[ii].para_name, sizeof(input.para[ii].para_name), "%s", "attr_delete2");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_attr_delete_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_attr_delete_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_attr_delete_set_callback(enos_mas_p, enos_dev_attr_delete_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_delete_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    ret = enos_dev_attr_delete_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_attr_delete_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.para);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.para);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Report Device Measuring Points
void enos_dev_point_post_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_point_post_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_point_post_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_point_post_output *output = NULL;
    int ret = parse_enos_dev_point_post_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_point_post_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_point_post_output(output);
}

int enos_dev_point_post_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_point_post_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_point_post_input input;
    memset(&input, 0, sizeof(struct enos_dev_point_post_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.data = "{\
         \"enos_ai.0\": 5.55,\
         \"enos_ai.1\": 500\
         }";
    input.time_ms_is_valid = 0;
    //input.time_ms_is_valid = 1;
    //input.time_ms = 2000;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_point_post_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_point_post_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_point_post_set_callback(enos_mas_p, enos_dev_point_post_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_post_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    ret = enos_dev_point_post_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_post_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);

    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Report Device Events(Non-Passthrough)
void enos_dev_event_post_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_event_post_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_event_post_output *output = NULL;
    int ret = parse_enos_dev_event_post_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_event_post_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_event_post_output(output);
}

int enos_dev_event_post_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_event_post_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_event_post_input input;
    memset(&input, 0, sizeof(struct enos_dev_event_post_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    snprintf(input.event_id, sizeof(input.event_id), "%s", "enos_soe.0");
    input.data = "{\
                        \"warn_msg\": \"aaaaaa\"\
                }";
//    input.data = "{\
//                        \"enos_soe.0\": {\
//                            \"warn_msg\": \"aaaaaa\"\
//                        }\
//                }";
    input.time_ms_is_valid = 0;
//    input.time_ms_is_valid = 1;
//    input.time_ms = 2000;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_event_post_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_event_post_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_event_post_set_callback(enos_mas_p, enos_dev_event_post_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_event_post_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    ret = enos_dev_event_post_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", "enos_soe.0", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_event_post_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);

    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Report Device Events(Passthrough)
void enos_dev_event_post_raw_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_event_post_raw_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_raw_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_event_post_raw_output_info_ex *info = NULL;
    int ret = parse_enos_dev_event_post_raw_data_response_info_ex(info_ex, &info);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_event_post_raw_data_response_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "product_key=%s, device_key=%s\n", info->dev_product_key, info->dev_device_key);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_event_post_raw_output_info_ex(info);
}

int enos_dev_event_post_raw_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_event_post_raw_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    char *data = "enos_dev_event_post_raw tttttttt";
    int data_len = strlen(data);
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_event_post_raw_set_callback(enos_mas_p, enos_dev_event_post_raw_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_event_post_raw_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_event_post_raw_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_event_post_raw_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Set Device Measuring Points(downstream)
void enos_dev_point_set_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_point_set_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_dev_point_set_callback_data *callback_data = NULL;
    int ret = parse_enos_dev_point_set_callback_data(data, data_len, info_ex, &callback_data);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_point_set_callback_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data:id=%s, product_key=%s, device_key=%s, data=%s\n", callback_data->id, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = NULL;
    int reply_data_len = 0;
    ret = generate_enos_general_reply_data_var(callback_data->id, 200, "hello", NULL, &reply_data, &reply_data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_set_callback_data(callback_data);
        return;
    }
    ret = enos_send_dev_point_set_reply_syn(enos_mas_p, callback_data->info->dev_product_key, callback_data->info->dev_device_key, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_set_callback_data(callback_data);
        free(reply_data);
        return;
    }
    free_enos_dev_point_set_callback_data(callback_data);
    free(reply_data);
    
    return;
}

int enos_dev_point_set_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_point_set_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_dev_point_set_set_callback(enos_mas_p, enos_dev_point_set_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_set_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_point_set_register(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_set_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//Get Device Measuring Points(downstream)
void enos_dev_point_get_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_point_get_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_dev_point_get_callback_data *callback_data = NULL;
    int ret = parse_enos_dev_point_get_callback_data(data, data_len, info_ex, &callback_data);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_point_get_callback_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data:id=%s, product_key=%s, device_key=%s, para_count=%d\n", callback_data->id, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->para_count);
    int ii = 0;
    for(ii = 0; ii < callback_data->para_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "para_name=%s\n", callback_data->para[ii]);
        return;
    }
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = NULL;
    int reply_data_len = 0;
    char *reply_data_data = "{\
                                    \"enos_ai.0\": 8.88\
                            }";
    ret = generate_enos_general_reply_data_var(callback_data->id, 200, "hello", reply_data_data, &reply_data, &reply_data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_get_callback_data(callback_data);
        return;
    }
    ret = enos_send_dev_point_get_reply_syn(enos_mas_p, callback_data->info->dev_product_key, callback_data->info->dev_device_key, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_point_get_callback_data(callback_data);
        free(reply_data);
        return;
    }
    free_enos_dev_point_get_callback_data(callback_data);
    free(reply_data);
    
    return;
}

int enos_dev_point_get_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_point_get_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_dev_point_get_set_callback(enos_mas_p, enos_dev_point_get_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_get_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_point_get_register(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_get_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}


//Invoke Device Services(Non-Passthrough)(downstream)
void enos_dev_service_invoke_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_service_invoke_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_dev_service_invoke_callback_data *callback_data = NULL;
    int ret = parse_enos_dev_service_invoke_callback_data(data, data_len, info_ex, &callback_data);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_service_invoke_callback_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data:id=%s, product_key=%s, device_key=%s, event_id=%s, data=%s\n", callback_data->id, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->info->event_id, callback_data->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = NULL;
    int reply_data_len = 0;
    char *reply_data_data = "{\
                                    \"result\": 5\
                            }";
    ret = generate_enos_general_reply_data_var(callback_data->id, 200, "hello", reply_data_data, &reply_data, &reply_data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(callback_data);
        return;
    }
    ret = enos_send_dev_service_invoke_reply_syn(enos_mas_p, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->info->event_id, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(callback_data);
        free(reply_data);
        return;
    }
    free_enos_dev_service_invoke_callback_data(callback_data);
    free(reply_data);
    
    return;
}

int enos_dev_service_invoke_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_service_invoke_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_dev_service_invoke_set_callback(enos_mas_p, enos_dev_service_invoke_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_service_invoke_register(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", "enos_di.0", NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}


//Invoke Device Services(Passthrough)(downstream)
void enos_dev_service_invoke_raw_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_service_invoke_raw_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_dev_service_invoke_raw_callback_data_info_ex *callback_data_info = NULL;
    int ret = parse_enos_dev_service_invoke_raw_callback_data_info_ex(info_ex, &callback_data_info);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_service_invoke_raw_callback_data_info_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data_info:product_key=%s, device_key=%s\n", callback_data_info->dev_product_key, callback_data_info->dev_device_key);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = "ggggggggggggggggg";
    int reply_data_len = strlen(reply_data);
    
    ret = enos_send_dev_service_invoke_raw_reply_syn(enos_mas_p, callback_data_info->dev_product_key, callback_data_info->dev_device_key, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_raw_callback_data_info_ex(callback_data_info);
        return;
    }
    free_enos_dev_service_invoke_raw_callback_data_info_ex(callback_data_info);
    
    return;
}

int enos_dev_service_invoke_raw_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_service_invoke_raw_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_dev_service_invoke_raw_set_callback(enos_mas_p, enos_dev_service_invoke_raw_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_raw_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_service_invoke_raw_register(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_raw_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}


//Disable Sub-devices(downstream)(Enable Sub-devices,Delete Sub-devices,Disable/Enable/Delete devices is similar)
void enos_sub_dev_disable_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_disable_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_sub_dev_disable_callback_data *callback_data = NULL;
    int ret = parse_enos_sub_dev_disable_callback_data(data, data_len, &callback_data);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_disable_callback_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data:id=%s, para_count=%d\n", callback_data->id, callback_data->para_count);
    int ii = 0;
    for(ii = 0; ii < callback_data->para_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "sub_dev_product_key=%s, sub_dev_device_key=%s\n", callback_data->para[ii].sub_dev_product_key, callback_data->para[ii].sub_dev_device_key);
        free_enos_sub_dev_disable_callback_data(callback_data);
        return;
    }
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = NULL;
    int reply_data_len = 0;
    ret = generate_enos_general_reply_data_var(callback_data->id, 200, "hello", NULL, &reply_data, &reply_data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_disable_callback_data(callback_data);
        return;
    }
    ret = enos_send_sub_dev_disable_reply_syn(enos_mas_p, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_sub_dev_disable_callback_data(callback_data);
        free(reply_data);
        return;
    }
    free_enos_sub_dev_disable_callback_data(callback_data);
    free(reply_data);
    
    return;
}

int enos_sub_dev_disable_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_disable_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_sub_dev_disable_set_callback(enos_mas_p, enos_sub_dev_disable_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_disable_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_sub_dev_disable_register(enos_mas_p, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_disable_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

//generate csr
int enos_generate_csr()
{
    int ret = 0;
    
    char pub_key_path[1024];
    memset(pub_key_path, 0, sizeof(pub_key_path));
    snprintf(pub_key_path, sizeof(pub_key_path), "%s", "./enos_mqtt_test_tmp/csr_pub_key.pem");
    
    char pri_key_path[1024];
    memset(pri_key_path, 0, sizeof(pri_key_path));
    snprintf(pri_key_path, sizeof(pri_key_path), "%s", "./enos_mqtt_test_tmp/csr_pri_key.pem");
    
    char req_path[1024];
    memset(req_path, 0, sizeof(req_path));
    snprintf(req_path, sizeof(req_path), "%s", "./enos_mqtt_test_tmp/req.csr");
    
//    char *cipher_fun = NULL;
//    
//    char *key_passwd = NULL;
    
    char cipher_fun[1024];
    memset(cipher_fun, 0, sizeof(cipher_fun));
    snprintf(cipher_fun, sizeof(cipher_fun), "%s", "aes256");
    
    char key_passwd[1024];
    memset(key_passwd, 0, sizeof(key_passwd));
    snprintf(key_passwd, sizeof(key_passwd), "%s", "123456");
    
    int key_size = 2048;
    EVP_PKEY *pkey = NULL;
    
    ret = enos_generate_rsa_key(pub_key_path, pri_key_path, cipher_fun, key_passwd, key_size, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_generate_rsa_key error\n");
        return -1;
    }
    
    struct enos_cert_info input_info;
    memset(&input_info, 0, sizeof(struct enos_cert_info));
    input_info.cert_version = ENOS_CERT_VERSION_3;
    input_info.sig_alg = ENOS_DEFAULT_SIG_ALG;
    snprintf(input_info.C_country, sizeof(input_info.C_country), "%s", ENOS_DEFAULT_REQ_C);
    snprintf(input_info.ST_province, sizeof(input_info.ST_province), "%s", ENOS_DEFAULT_REQ_ST);
    snprintf(input_info.L_city, sizeof(input_info.L_city), "%s", ENOS_DEFAULT_REQ_L);
    snprintf(input_info.O_organization, sizeof(input_info.O_organization), "%s", ENOS_DEFAULT_REQ_O);
    snprintf(input_info.OU_organizationunit, sizeof(input_info.OU_organizationunit), "%s", ENOS_DEFAULT_REQ_OU);
    snprintf(input_info.CN_commonname, sizeof(input_info.CN_commonname), "%s", "fjdklfhdsfhkjds");
    
    ret = enos_generate_cert_req(req_path, &input_info, pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_generate_cert_req error\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    return 0;
}

//csr apply
int get_file_size(char *path)
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


int read_file_content(char *path, char *buf, int len)
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

int write_file_content(char *path, char *buf, int len)
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

int replace_all(char *str_in, char *str_out, int str_out_max, char *old_value, char *new_value)
{
	string string_str_in(str_in);
	string string_old_value(old_value);
	string string_new_value(new_value);

	string::size_type pos(0);
	string::size_type pos_temp(0);

	while(pos != string::npos)
	{
		pos_temp = string_str_in.find(string_old_value, pos);
		if(pos_temp != string::npos)
		{
			string_str_in.replace(pos_temp, string_old_value.length(), string_new_value);
		}
		else
		{
			break;
		}

		pos = pos_temp + string_new_value.length();
	}

	snprintf(str_out, str_out_max, "%s", string_str_in.c_str());

	return 0;
}

int enos_mqtt_api_csr_test()
{
    struct enos_restful_api_struct *my_enos_restful_api;
	char request_url[]="https://beta-portal-cn4.eniot.io:8081/enosapi";
	char access_key[]="635986bc-f6cb-4812552310a8-da8c-462a";
	char access_secret[]="552310a8-da8c-462a-a8ca-f60dbdd3a97a";
	char org_id[]="o15427722038191";
	char *csr_path = "./enos_mqtt_test_tmp/req.csr";
	char csr1[8192];
	memset(csr1, 0, sizeof(csr1));
	char csr2[8192];
	memset(csr2, 0, sizeof(csr2));
	int len1 = get_file_size(csr_path);
	if(len1 <= 0)
	{
	    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]get_file_size %s error(file=%s, function=%s, line=%d)\n", csr_path, __FILE__, __FUNCTION__, __LINE__);
	    return -1;
	}
	
	int ret = read_file_content(csr_path, csr1, len1);
	if(ret < 0)
	{
	    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]read_file_content %s error(file=%s, function=%s, line=%d)\n", csr_path, __FILE__, __FUNCTION__, __LINE__);
	    return -1;
	}
    
    replace_all(csr1, csr2, (int)(sizeof(csr2)), "\n", "\\n");
	
	enos_restful_api_init(&my_enos_restful_api);
	
	enos_restful_api_set_requestURL(my_enos_restful_api,request_url);
	
	enos_restful_api_set_accessKey(my_enos_restful_api,access_key);
	
	enos_restful_api_set_secretKey(my_enos_restful_api,access_secret);
	
	enos_restful_api_set_orgId(my_enos_restful_api,org_id);
	
	struct enos_applyCertificateByDeviceKey_input input;
	sprintf(input.productKey,"bQBK85tM");
	sprintf(input.deviceKey,"1EtyL9EZ7J");
	//input.csr = csr2;
	input.csr = csr1;
	struct enos_restful_api_output *output = NULL;
	struct enos_applyCertificate_data *data = NULL;
	
	ret = enos_restful_api_syn_applyCertificateByDeviceKey(my_enos_restful_api, &input, &output, 30);
	if(ret < 0)
	{
	    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_restful_api_syn_applyCertificateByDeviceKey error1(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    return -1;
	}
	
	if(output == NULL)
	{
	    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_restful_api_syn_applyCertificateByDeviceKey error2(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    return -1;
	}
	
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output = %s\n", output->data);
	
	enos_restful_api_analysis_applyCertificate(output, &data);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "status = %d\n", data->head.status);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "requestId = %s\n", data->head.requestId);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "msg = %s\n", data->head.msg);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "submsg = %s\n", data->head.submsg);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "certChainURL = %s\n", data->certChainURL);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "cert = %s\n", data->cert);
	myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "certSN = %s\n", data->certSN);
	
	if(data->cert == NULL)
	{
	    myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data->cert==NULL, error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    enos_restful_api_free_applyCertificate_data(data);
	    enos_restful_api_free_output(output);
	    return -1;
	}
	write_file_content("./enos_mqtt_test_tmp/cert.crt", data->cert, strlen(data->cert));
	
	//ÏÂÔØCAÖ¤Êé
	FILE *fp = fopen("./enos_mqtt_test_tmp/ca.crt","w+b");
	enos_restful_api_syn_downloadFile(data->certChainURL, fp, 30);
	fclose(fp);
	enos_restful_api_free_applyCertificate_data(data);
	enos_restful_api_free_output(output);
	
	return 0;
}

//mqtt tls connect
void enos_mqtt_api_tls_connect_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]tls connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

int enos_mqtt_api_tls_connect_test()
{
    int ret = 0;
    char *address = "ssl://10.27.20.142:18883";
    //char *address = "beta-iot-as-mqtt-cn4.eniot.io:18883";
    char *gw_product_key = "bQBK85tM";
    char *gw_device_key = "1EtyL9EZ7J";
    char *gw_device_secret = "chG0XIyXyduG7GbHA29j";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 15000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_mqtt_api_tls_connect_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_set_ssl(enos_mas_p, "./enos_mqtt_test_tmp/ca.crt", "./enos_mqtt_test_tmp/cert.crt", "./enos_mqtt_test_tmp/csr_pri_key.pem", "123456");
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_ssl error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_sleep(10000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}

int main(int argc, char **argv)
{
    char *tmp_dir = "enos_mqtt_test_tmp";
    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
        #if _MSC_VER
            _mkdir(tmp_dir);
        #else
            mkdir(tmp_dir);
        #endif
    #else
        mkdir(tmp_dir, 0777);
    #endif
    
//    enos_generate_csr();
//    enos_mqtt_api_csr_test();

      enos_mqtt_api_tls_connect_test();
//    enos_dev_event_post_asyn_test();
//    enos_sub_dev_disable_asyn_test();
//    enos_dev_event_post_raw_asyn_test();
//    enos_mqtt_api_send_normal_syn_test();

    return 0;
}