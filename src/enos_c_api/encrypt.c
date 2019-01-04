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
#include "common_tool.h"

#if defined(ENABLE_TPM)
    #include "tpm2-tss-engine/tpm2-tss-engine.h"
#endif

static int sig_alg_int_to_EVP_MD(int sig_alg_int, EVP_MD **evp_md_pp);

static int rsa_padding_int_to_use(int rsa_padding_int, int *int_out);

static int custom_pem_passwd_cb(char* buf, int size, int rwflag, void* userdata);


static int sig_alg_int_to_EVP_MD(int sig_alg_int, EVP_MD **evp_md_pp)
{
    switch(sig_alg_int)
    {
        case ENOS_CERT_SIG_ALG_sha1:
            *evp_md_pp = (EVP_MD *)EVP_sha1();
            break;
        case ENOS_CERT_SIG_ALG_sha224:
            *evp_md_pp = (EVP_MD *)EVP_sha224();
            break;
        case ENOS_CERT_SIG_ALG_sha256:
            *evp_md_pp = (EVP_MD *)EVP_sha256();
            break;
        case ENOS_CERT_SIG_ALG_sha512_224:
            *evp_md_pp = (EVP_MD *)EVP_sha512_224();
            break;
        case ENOS_CERT_SIG_ALG_sha512_256:
            *evp_md_pp = (EVP_MD *)EVP_sha512_256();
            break;
        case ENOS_CERT_SIG_ALG_sha384:
            *evp_md_pp = (EVP_MD *)EVP_sha384();
            break;
        case ENOS_CERT_SIG_ALG_sha512:
            *evp_md_pp = (EVP_MD *)EVP_sha512();
            break;
        case ENOS_CERT_SIG_ALG_md5:
            *evp_md_pp = (EVP_MD *)EVP_md5();
            break;
        default:
            *evp_md_pp = (EVP_MD *)EVP_sha256();
            break;
    }
    
    return 0;
}


static int rsa_padding_int_to_use(int rsa_padding_int, int *int_out)
{
    switch(rsa_padding_int)
    {
        case ENOS_RSA_PADDING_PKCS1_PADDING:
            *int_out = RSA_PKCS1_PADDING;
            break;
        case ENOS_RSA_PADDING_SSLV23_PADDING:
            *int_out = RSA_SSLV23_PADDING;
            break;
        case ENOS_RSA_PADDING_NO_PADDING:
            *int_out = RSA_NO_PADDING;
            break;
        case ENOS_RSA_PADDING_PKCS1_OAEP_PADDING:
            *int_out = RSA_PKCS1_OAEP_PADDING;
            break;
        case ENOS_RSA_PADDING_X931_PADDING:
            *int_out = RSA_X931_PADDING;
            break;
        case ENOS_RSA_PADDING_PSS_PADDING:
            *int_out = RSA_PKCS1_PSS_PADDING;
            break;
        default:
            *int_out = RSA_PKCS1_PADDING;
            break;
    }
    
    return 0;
}


static int custom_pem_passwd_cb(char* buf, int size, int rwflag, void* userdata)
{
    enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]size = %d, rwflag = %d(file=%s, function=%s, line=%d)\n", size, rwflag, __FILE__, __FUNCTION__, __LINE__);
    if(userdata != NULL)
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]userdata=%s(file=%s, function=%s, line=%d)\n", (char *)userdata, __FILE__, __FUNCTION__, __LINE__);
    }
    else
    {
        enos_printf(NULL, ENOS_LOG_DEBUG, "[ENOS_ACCESS_API]userdata=NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    }
    if(size <= 0)
    {
        return 0;
    }
    
    if(rwflag)
    {
        return 0;
    }
    
    if(userdata == NULL)
    {
        memset(buf, 0, size);
        return 0;
    }
    
    char *userdata_str = (char *)userdata;
    int len = (int)(strlen(userdata_str));
    if(len > size)
    {
        len = size;
    }
    
    memset(buf, 0, size);
    memcpy(buf, userdata_str, len);
    buf[size - 1] = '\0';
    
    int ret = (int)strlen(buf);
    return ret;
}


extern int enos_generate_random_value(char *random_ptr, int len)
{
    int ret = 0;
    struct timeval tv_temp;
    enos_gettimeofday(&tv_temp, NULL);
    RAND_seed((void *)&tv_temp, sizeof(tv_temp));
    ret = RAND_bytes((unsigned char *)(random_ptr), len);
    if(ret != 1)
    {
        return -1;
    }
    
    return 0;
}


extern int enos_generate_random_value_with_seed(char *seed_ptr, int seed_len, char *random_ptr, int len)
{
    int ret = 0;
    RAND_seed((void *)seed_ptr, seed_len);
    ret = RAND_bytes((unsigned char *)(random_ptr), len);
    if(ret != 1)
    {
        return -1;
    }
    
    return 0;
}


#if defined(ENABLE_TPM)

static int ui_open(UI *ui);

static int ui_read(UI *ui, UI_STRING *uis);

static int ui_write(UI *ui, UI_STRING *uis);

static int ui_close(UI *ui);

static int enos_init_tpm2_engine(char *owner_hierarchy_passwd, ENGINE **tpm_engine);

static int enos_uninit_tpm2_engine(ENGINE *tpm_engine);


static int ui_open(UI *ui)
{
    return 1;
}


static int ui_read(UI *ui, UI_STRING *uis)
{
    switch(UI_get_string_type(uis))
    {
        case UIT_PROMPT:
        {
            int maxsize = UI_get_result_maxsize(uis);
            char *user_data = (char *)UI_get0_user_data(ui);
            int ret = 0;
            if(user_data != NULL)
            {
                int user_data_len = strlen(user_data);
                int copy_len = user_data_len;
                if(copy_len > maxsize)
                {
                    copy_len = maxsize;
                }
                ret = UI_set_result_ex(ui, uis, user_data, copy_len);
                if(ret < 0)
                {
                    return 0;
                }
            }
            else
            {
                char result_temp[1];
                memset(result_temp, 0, sizeof(result_temp));
                ret = UI_set_result_ex(ui, uis, result_temp, 0);
                if(ret < 0)
                {
                    return 0;
                }
            }
            return 1;
        }
        case UIT_VERIFY:
        case UIT_NONE:
        case UIT_BOOLEAN:
        case UIT_INFO:
        case UIT_ERROR:
            break;
    }
    return 1;
}


static int ui_write(UI *ui, UI_STRING *uis)
{
    return 1;
}


static int ui_close(UI *ui)
{
    return 1;
}


static int enos_init_tpm2_engine(char *owner_hierarchy_passwd, ENGINE **tpm_engine)
{
    *tpm_engine = NULL;
    ENGINE_load_dynamic();
    ENGINE *tpm_engine_ret = ENGINE_by_id("tpm2tss");
    if(tpm_engine_ret == NULL)
    {
        tpm_engine_ret = ENGINE_by_id("libtpm2tss");
        if(tpm_engine_ret == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ENGINE_by_id error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    
    int ret = ENGINE_init(tpm_engine_ret);
    enos_printf(NULL, ENOS_LOG_INFO, "[ENOS_ACCESS_API]Engine name:%s, Init result:%d(file=%s, function=%s, line=%d)\n", ENGINE_get_name(tpm_engine_ret), ret, __FILE__, __FUNCTION__, __LINE__);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ENGINE_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        ENGINE_free(tpm_engine_ret);
        return -1;
    }

    ret = ENGINE_ctrl(tpm_engine_ret, TPM2TSS_SET_OWNERAUTH, 0, owner_hierarchy_passwd, NULL);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]tpm2 use owner_hierarchy_passwd error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        ENGINE_finish(tpm_engine_ret);
        ENGINE_free(tpm_engine_ret);
        return -1;
    }
    
    *tpm_engine = tpm_engine_ret;
    
    return 0;
}


static int enos_uninit_tpm2_engine(ENGINE *tpm_engine)
{
    ENGINE_finish(tpm_engine);
    ENGINE_free(tpm_engine);
    
    return 0;
}


extern int enos_generate_rsa_key_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, int key_size, EVP_PKEY **pkey)
{
    int ret = 0;
    
    if(pkey != NULL)
    {
        *pkey = NULL;
    }
    
    int key_size_local = ENOS_DEFAULT_RSA_KEY_SIZE;
    if(key_size <= 0)
    {
        key_size_local = ENOS_DEFAULT_RSA_KEY_SIZE;
    }
    else
    {
        key_size_local = key_size;
    }
    
    ENGINE *tpm_engine = NULL;
    ret = enos_init_tpm2_engine(owner_hierarchy_passwd, &tpm_engine);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_init_tpm2_engine error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    RSA *rsa_ret = NULL;
    BIGNUM *big_num = BN_new();
    if (big_num == NULL) 
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BN_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    BN_set_word(big_num, RSA_F4);

    rsa_ret = RSA_new();
    if(rsa_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        BN_free(big_num);
        return -1;
    }
    
    ret = tpm2tss_rsa_genkey(rsa_ret, key_size_local, big_num, key_passwd);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]tpm2tss_rsa_genkey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        BN_free(big_num);
        RSA_free(rsa_ret);
        return -1;
    }
    
    if(key_path != NULL)
    {
        TPM2_DATA *tpm2_data = (TPM2_DATA *)malloc(sizeof(TPM2_DATA));
        if (tpm2_data == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]tpm2_data malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            enos_uninit_tpm2_engine(tpm_engine);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }
        memset(tpm2_data, 0, sizeof(TPM2_DATA));
        char *app_data = (char *)RSA_get_app_data(rsa_ret);
        if(app_data == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_get_app_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            enos_uninit_tpm2_engine(tpm_engine);
            BN_free(big_num);
            RSA_free(rsa_ret);
            free(tpm2_data);
            return -1;
        }
        memcpy(tpm2_data, app_data, sizeof(TPM2_DATA));
        
        ret = tpm2tss_tpm2data_write(tpm2_data, key_path);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]tpm2tss_tpm2data_write error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            enos_uninit_tpm2_engine(tpm_engine);
            BN_free(big_num);
            RSA_free(rsa_ret);
            free(tpm2_data);
            return -1;
        }
        free(tpm2_data);
    }
    
    if(pkey != NULL)
    {
//        EVP_PKEY *pkey_ret = EVP_PKEY_new();
//        if(pkey_ret == NULL)
//        {
//            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
//            enos_uninit_tpm2_engine(tpm_engine);
//            BN_free(big_num);
//            RSA_free(rsa_ret);
//            return -1;
//        }
//        
//        ret = EVP_PKEY_assign_RSA(pkey_ret, rsa_ret);
//        if(ret <= 0)
//        {
//            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_assign_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
//            enos_uninit_tpm2_engine(tpm_engine);
//            BN_free(big_num);
//            RSA_free(rsa_ret);
//            EVP_PKEY_free(pkey_ret);
//            return -1;
//        }
//        
//        *pkey = pkey_ret;
//        rsa_ret = NULL;
//        //rsa_ret will be free when EVP_PKEY_free(pKey)

        TPM2_DATA *tpm2_data = (TPM2_DATA *)RSA_get_app_data(rsa_ret);
        if (tpm2_data == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_get_app_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            enos_uninit_tpm2_engine(tpm_engine);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }

        EVP_PKEY *pkey_ret = tpm2tss_rsa_makekey(tpm2_data);
        if (pkey_ret == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]tpm2tss_rsa_makekey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            enos_uninit_tpm2_engine(tpm_engine);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }
        *pkey = pkey_ret;
        RSA_free(rsa_ret);
    }
    else
    {
        RSA_free(rsa_ret);
    }

    enos_uninit_tpm2_engine(tpm_engine);
    BN_free(big_num);
    return 0;
}


extern int enos_read_rsa_pri_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pri_key)
{
    *pri_key = NULL;
    ENGINE *tpm_engine = NULL;
    int ret = enos_init_tpm2_engine(owner_hierarchy_passwd, &tpm_engine);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_init_tpm2_engine error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    UI_METHOD *ui_method = UI_create_method("OpenSSL tpm2 user interface");
    if(ui_method == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_create_method error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_opener(ui_method, ui_open);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_opener error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_reader(ui_method, ui_read);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_reader error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_writer(ui_method, ui_write);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_writer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_closer(ui_method, ui_close);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_closer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    EVP_PKEY* pri_key_ret = ENGINE_load_private_key(tpm_engine, key_path, ui_method, key_passwd);
    if(pri_key_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ENGINE_load_private_key error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    *pri_key = pri_key_ret;
    
    UI_destroy_method(ui_method);
    enos_uninit_tpm2_engine(tpm_engine);
    
    return 0;
}


extern int enos_read_rsa_pub_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pub_key)
{
    *pub_key = NULL;
    ENGINE *tpm_engine = NULL;
    int ret = enos_init_tpm2_engine(owner_hierarchy_passwd, &tpm_engine);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_init_tpm2_engine error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    UI_METHOD *ui_method = UI_create_method("OpenSSL tpm2 user interface");
    if(ui_method == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_create_method error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_opener(ui_method, ui_open);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_opener error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_reader(ui_method, ui_read);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_reader error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_writer(ui_method, ui_write);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_writer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_closer(ui_method, ui_close);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_closer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    //no ENGINE_load_public_key
    EVP_PKEY* pub_key_ret = ENGINE_load_private_key(tpm_engine, key_path, ui_method, key_passwd);
    if(pub_key_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ENGINE_load_public_key error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    *pub_key = pub_key_ret;
    
    UI_destroy_method(ui_method);
    enos_uninit_tpm2_engine(tpm_engine);
    
    return 0;
}


extern int enos_read_rsa_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pkey)
{
    *pkey = NULL;
    ENGINE *tpm_engine = NULL;
    int ret = enos_init_tpm2_engine(owner_hierarchy_passwd, &tpm_engine);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_init_tpm2_engine error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    UI_METHOD *ui_method = UI_create_method("OpenSSL tpm2 user interface");
    if(ui_method == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_create_method error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_opener(ui_method, ui_open);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_opener error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_reader(ui_method, ui_read);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_reader error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_writer(ui_method, ui_write);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_writer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    ret = UI_method_set_closer(ui_method, ui_close);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]UI_method_set_closer error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    EVP_PKEY* pkey_ret = ENGINE_load_private_key(tpm_engine, key_path, ui_method, key_passwd);
    if(pkey_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]ENGINE_load_private_key error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        UI_destroy_method(ui_method);
        enos_uninit_tpm2_engine(tpm_engine);
        return -1;
    }
    
    *pkey = pkey_ret;
    
    UI_destroy_method(ui_method);
    enos_uninit_tpm2_engine(tpm_engine);
    
    return 0;
}
#endif


extern int enos_generate_rsa_key(char *pub_key_path, char *pri_key_path, char *cipher_fun, char *key_passwd, int key_size, EVP_PKEY **pkey)
{
    int ret = 0;
    
    if(pkey != NULL)
    {
        *pkey = NULL;
    }
    
    int key_size_local = ENOS_DEFAULT_RSA_KEY_SIZE;
    if(key_size <= 0)
    {
        key_size_local = ENOS_DEFAULT_RSA_KEY_SIZE;
    }
    else
    {
        key_size_local = key_size;
    }
    
    RSA *rsa_ret = NULL;
    BIGNUM *big_num = BN_new();
    if (big_num == NULL) 
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BN_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    BN_set_word(big_num, RSA_F4);

    rsa_ret = RSA_new();
    if(rsa_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BN_free(big_num);
        return -1;
    }
    
    ret = RSA_generate_key_ex(rsa_ret, key_size_local, big_num, NULL);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_generate_key_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BN_free(big_num);
        RSA_free(rsa_ret);
        return -1;
    }
    
    if(pub_key_path != NULL)
    {
        BIO *public_out = BIO_new_file(pub_key_path, "w");
        if(public_out == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", pub_key_path, __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }
        
        ret = PEM_write_bio_RSAPublicKey(public_out, rsa_ret);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_write_bio_RSAPublicKey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            BIO_free_all(public_out);
            return -1;
        }
        BIO_free_all(public_out);
    }
    
    if(pri_key_path != NULL)
    {
        BIO *private_out = BIO_new_file(pri_key_path, "w");
        if(private_out == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", pri_key_path, __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }
        
        EVP_CIPHER *evp_chipher = NULL;
        if(cipher_fun != NULL)
        {
            evp_chipher = (EVP_CIPHER *)EVP_get_cipherbyname(cipher_fun);
            if(evp_chipher == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cipher_fun=%s, not support, error(file=%s, function=%s, line=%d)\n", cipher_fun, __FILE__, __FUNCTION__, __LINE__);
                BN_free(big_num);
                RSA_free(rsa_ret);
                BIO_free_all(private_out);
                return -1;
            }
        }
        
        if(evp_chipher != NULL)
        {
            if(key_passwd == NULL)
            {
                enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]cipher_fun=%s, but key_passwd is NULL, error(file=%s, function=%s, line=%d)\n", cipher_fun, __FILE__, __FUNCTION__, __LINE__);
                BN_free(big_num);
                RSA_free(rsa_ret);
                BIO_free_all(private_out);
                return -1;
            }
        }
        
        ret = PEM_write_bio_RSAPrivateKey(private_out, rsa_ret, evp_chipher, NULL, 0, NULL, key_passwd);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_write_bio_RSAPrivateKey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            BIO_free_all(private_out);
            return -1;
        }
        BIO_free_all(private_out);
    }
    
    if(pkey != NULL)
    {
        EVP_PKEY *pkey_ret = EVP_PKEY_new();
        if(pkey_ret == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            return -1;
        }
        
        ret = EVP_PKEY_assign_RSA(pkey_ret, rsa_ret);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_assign_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            BN_free(big_num);
            RSA_free(rsa_ret);
            EVP_PKEY_free(pkey_ret);
            return -1;
        }
        
        *pkey = pkey_ret;
        rsa_ret = NULL;
        //rsa_ret will be free when EVP_PKEY_free(pKey)
    }
    else
    {
        RSA_free(rsa_ret);
    }

    BN_free(big_num);
    return 0;
}


extern int enos_read_rsa_pri_key_from_file(char *key_path, char *key_passwd, EVP_PKEY **pri_key)
{
    int ret = 0;
    
    if(key_path == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]key_path==NULL, error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(pri_key == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY **pri_key==NULL, error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    *pri_key = NULL;
    
    BIO *bio_in = NULL;
    RSA *rsa_ret = NULL;

    bio_in = BIO_new_file(key_path, "r");
    if(bio_in == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", key_path, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    rsa_ret = PEM_read_bio_RSAPrivateKey(bio_in, NULL, custom_pem_passwd_cb, key_passwd);
    if(rsa_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_RSAPrivateKey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free_all(bio_in);
        return -1;
    }
    
    EVP_PKEY *pkey_ret = EVP_PKEY_new();
    if(pkey_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        RSA_free(rsa_ret);
        BIO_free_all(bio_in);
        return -1;
    }
    
    ret = EVP_PKEY_assign_RSA(pkey_ret, rsa_ret);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_assign_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        RSA_free(rsa_ret);
        BIO_free_all(bio_in);
        EVP_PKEY_free(pkey_ret);
        return -1;
    }
    
    *pri_key = pkey_ret;
    rsa_ret = NULL;
    //rsa_ret will be free when EVP_PKEY_free(pkey_ret)
    
    BIO_free_all(bio_in);
    return 0;
}


extern int enos_read_rsa_pub_key_from_file(char *key_path, EVP_PKEY **pub_key)
{
    int ret = 0;
    
    if(key_path == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]key_path==NULL, error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(pub_key == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY **pub_key==NULL, error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    *pub_key = NULL;
    
    BIO *bio_in = NULL;
    RSA *rsa_ret = NULL;

    bio_in = BIO_new_file(key_path, "r");
    if(bio_in == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", key_path, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    rsa_ret = PEM_read_bio_RSAPublicKey(bio_in, NULL, NULL, NULL);
    if(rsa_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_RSAPublicKey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free_all(bio_in);
        return -1;
    }
    
    EVP_PKEY *pkey_ret = EVP_PKEY_new();
    if(pkey_ret == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        RSA_free(rsa_ret);
        BIO_free_all(bio_in);
        return -1;
    }
    
    ret = EVP_PKEY_assign_RSA(pkey_ret, rsa_ret);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_assign_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        RSA_free(rsa_ret);
        BIO_free_all(bio_in);
        EVP_PKEY_free(pkey_ret);
        return -1;
    }
    
    *pub_key = pkey_ret;
    rsa_ret = NULL;
    //rsa_ret will be free when EVP_PKEY_free(pkey_ret)
    
    BIO_free_all(bio_in);
    return 0;
}

extern int enos_read_rsa_pub_key_from_csr(char *csr_path, EVP_PKEY **pub_key)
{
    if((csr_path == NULL) || (pub_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    BIO *bio = NULL;
    X509_REQ *req = NULL;
    EVP_PKEY *pkey = NULL;
    *pub_key = NULL;

    bio = BIO_new_file(csr_path, "r");
    if(bio == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", csr_path, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    req = PEM_read_bio_X509_REQ(bio, NULL, NULL, NULL);
    if(req == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509_REQ error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        return -1;
    }

    pkey = X509_REQ_get_pubkey(req);
    if(pkey == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_REQ_get_pubkey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        X509_REQ_free(req);
        return -1;
    }
    
    *pub_key = pkey;

    BIO_free(bio);
    X509_REQ_free(req);
    return 0;
}


extern int enos_read_rsa_pub_key_from_crt_str(char *crt_str, char *passwd, EVP_PKEY **pub_key)
{
    if((crt_str == NULL) || (pub_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    BIO *bio = NULL;
    X509 *x509_cert = NULL;
    EVP_PKEY *pkey = NULL;
    *pub_key = NULL;

    bio = BIO_new_mem_buf(crt_str, strlen(crt_str));
    if(bio == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_mem_buf error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    x509_cert = PEM_read_bio_X509(bio, NULL, custom_pem_passwd_cb, passwd);
    if(x509_cert == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509 error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        return -1;
    }

    pkey = X509_get_pubkey(x509_cert);
    if(pkey == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_get_pubkey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        X509_free(x509_cert);
        return -1;
    }
    
    *pub_key = pkey;

    BIO_free(bio);
    X509_free(x509_cert);
    return 0;
}


extern int enos_read_rsa_pub_key_from_crt(char *crt_path, char *passwd, EVP_PKEY **pub_key)
{
    if((crt_path == NULL) || (pub_key == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    BIO *bio = NULL;
    X509 *x509_cert = NULL;
    EVP_PKEY *pkey = NULL;
    *pub_key = NULL;

    bio = BIO_new_file(crt_path, "r");
    if(bio == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", crt_path, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    x509_cert = PEM_read_bio_X509(bio, NULL, custom_pem_passwd_cb, passwd);
    if(x509_cert == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509 error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        return -1;
    }

    pkey = X509_get_pubkey(x509_cert);
    if(pkey == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_get_pubkey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        X509_free(x509_cert);
        return -1;
    }
    
    *pub_key = pkey;

    BIO_free(bio);
    X509_free(x509_cert);
    return 0;
}


extern int enos_get_req_CN(char *common_name, int common_name_max_size)
{
    if((common_name == NULL) || (common_name_max_size <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    memset(common_name, 0, common_name_max_size);
    char random_data[10];
    int random_data_len = (int)(sizeof(random_data));
    memset(random_data, 0, sizeof(random_data));
    int ret = enos_generate_random_value(random_data, random_data_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_generate_random_value error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    char random_data_str[64];
    char *random_data_str_ptr = random_data_str;
    int max_len = (int)(sizeof(random_data_str) - (random_data_str_ptr - random_data_str));
    memset(random_data_str, 0, sizeof(random_data_str));
    int ii = 0;
    for(ii = 0; ii < random_data_len; ii++)
    {
        max_len = (int)(sizeof(random_data_str) - (random_data_str_ptr - random_data_str));
        snprintf(random_data_str_ptr, max_len, "%02hhx", (unsigned char)(random_data[ii]));
        random_data_str_ptr = random_data_str + strlen(random_data_str);
    }
    
    snprintf(common_name, common_name_max_size, "%s", random_data_str);
    return 0;
}


extern int enos_generate_cert_req(char *req_path, struct enos_cert_info *input_info, EVP_PKEY *pkey)
{
    int ret = 0;
    
    if((req_path == NULL) || (pkey == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    struct enos_cert_info input_info_local;
    memset(&input_info_local, 0, sizeof(struct enos_cert_info));
    
    if(input_info == NULL)
    {
        input_info_local.cert_version = ENOS_CERT_VERSION_3;
        input_info_local.sig_alg = ENOS_DEFAULT_SIG_ALG;
        snprintf(input_info_local.C_country, sizeof(input_info_local.C_country), "%s", ENOS_DEFAULT_REQ_C);
        snprintf(input_info_local.ST_province, sizeof(input_info_local.ST_province), "%s", ENOS_DEFAULT_REQ_ST);
        snprintf(input_info_local.L_city, sizeof(input_info_local.L_city), "%s", ENOS_DEFAULT_REQ_L);
        snprintf(input_info_local.O_organization, sizeof(input_info_local.O_organization), "%s", ENOS_DEFAULT_REQ_O);
        snprintf(input_info_local.OU_organizationunit, sizeof(input_info_local.OU_organizationunit), "%s", ENOS_DEFAULT_REQ_OU);
        ret = enos_get_req_CN(input_info_local.CN_commonname, (int)(sizeof(input_info_local.CN_commonname)));
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_get_req_CN error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    else
    {
        memcpy(&input_info_local, input_info, sizeof(struct enos_cert_info));
    }

    X509_REQ *x509_req = NULL;
    X509_NAME *x509_name = NULL;
    BIO *out = NULL;

    int cert_version = input_info_local.cert_version;
    int sig_alg = input_info_local.sig_alg;
    const char *C_country = input_info_local.C_country;
    const char *ST_province = input_info_local.ST_province;
    const char *L_city = input_info_local.L_city;
    const char *O_organization = input_info_local.O_organization;
    const char *OU_organizationunit = input_info_local.OU_organizationunit;
    const char *CN_commonname = input_info_local.CN_commonname;

    x509_req = X509_REQ_new();
    if(x509_req == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_REQ_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = X509_REQ_set_version(x509_req, cert_version);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_REQ_set_version cert_version=%d error(file=%s, function=%s, line=%d)\n", cert_version, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    x509_name = X509_REQ_get_subject_name(x509_req);

    ret = X509_NAME_add_entry_by_txt(x509_name, "C", MBSTRING_ASC, (const unsigned char*)C_country, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt C_country=%s error(file=%s, function=%s, line=%d)\n", C_country, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_NAME_add_entry_by_txt(x509_name,"ST", MBSTRING_ASC, (const unsigned char*)ST_province, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt ST_province=%s error(file=%s, function=%s, line=%d)\n", ST_province, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_NAME_add_entry_by_txt(x509_name,"L", MBSTRING_ASC, (const unsigned char*)L_city, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt L_city=%s error(file=%s, function=%s, line=%d)\n", L_city, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_NAME_add_entry_by_txt(x509_name,"O", MBSTRING_ASC, (const unsigned char*)O_organization, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt O_organization=%s error(file=%s, function=%s, line=%d)\n", O_organization, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_NAME_add_entry_by_txt(x509_name,"OU", MBSTRING_ASC, (const unsigned char*)OU_organizationunit, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt OU_organizationunit=%s error(file=%s, function=%s, line=%d)\n", OU_organizationunit, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_NAME_add_entry_by_txt(x509_name,"CN", MBSTRING_ASC, (const unsigned char*)CN_commonname, -1, -1, 0);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_NAME_add_entry_by_txt CN_commonname=%s error(file=%s, function=%s, line=%d)\n", CN_commonname, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    ret = X509_REQ_set_pubkey(x509_req, pkey);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_REQ_set_pubkey error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }
    
    EVP_MD *evp_md_p = NULL;
    sig_alg_int_to_EVP_MD(sig_alg, &evp_md_p);
    ret = X509_REQ_sign(x509_req, pkey, evp_md_p);    // return x509_req->signature->length
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_REQ_sign error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }

    out = BIO_new_file(req_path, "w");
    if(out == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", req_path, __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        return -1;
    }
    
    ret = PEM_write_bio_X509_REQ(out, x509_req);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_write_bio_X509_REQ error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_REQ_free(x509_req);
        BIO_free_all(out);
        return -1;
    }

    X509_REQ_free(x509_req);
    BIO_free_all(out);
    
    return 0;
}


extern int enos_crt_sign_verify(char *signed_crt_path, char *signed_crt_passwd, char *root_crt_path, char *root_crt_passwd, char *crl_path, char *crl_passwd)
{
    if((signed_crt_path == NULL) || (root_crt_path == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    OpenSSL_add_all_algorithms();
    
    int ret = 0;
    BIO *bio = NULL;
    X509 *signed_crt_x509 = NULL;
    X509 *root_crt_x509 = NULL;
    X509_STORE *ca_cert_store = NULL;
    X509_CRL *crl_x509 = NULL;
    X509_STORE_CTX *cert_ctx = NULL;
    STACK_OF(X509) *cert_stack = NULL;

    bio = BIO_new_file(signed_crt_path, "r");
    if(bio == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", signed_crt_path, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    signed_crt_x509 = PEM_read_bio_X509(bio, NULL, custom_pem_passwd_cb, signed_crt_passwd);
    if(signed_crt_x509 == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509 error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        return -1;
    }
    BIO_free(bio);
    bio = NULL;
    
    bio = BIO_new_file(root_crt_path, "r");
    if(bio == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", root_crt_path, __FILE__, __FUNCTION__, __LINE__);
        X509_free(signed_crt_x509);
        return -1;
    }
    
    root_crt_x509 = PEM_read_bio_X509(bio, NULL, custom_pem_passwd_cb, root_crt_passwd);
    if(root_crt_x509 == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509 error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        BIO_free(bio);
        X509_free(signed_crt_x509);
        return -1;
    }
    BIO_free(bio);
    bio = NULL;
    
    ca_cert_store = X509_STORE_new();
    if(ca_cert_store == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_free(signed_crt_x509);
        X509_free(root_crt_x509);
        return -1;
    }

    ret = X509_STORE_add_cert(ca_cert_store, root_crt_x509);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_add_cert error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_free(signed_crt_x509);
        X509_free(root_crt_x509);
        X509_STORE_free(ca_cert_store);
        return -1;
    }
    
    if(crl_path != NULL)
    {
        bio = BIO_new_file(crl_path, "r");
        if(bio == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]BIO_new_file %s error(file=%s, function=%s, line=%d)\n", crl_path, __FILE__, __FUNCTION__, __LINE__);
            X509_free(signed_crt_x509);
            X509_free(root_crt_x509);
            X509_STORE_free(ca_cert_store);
            return -1;
        }
        
        crl_x509 = PEM_read_bio_X509_CRL(bio, NULL, custom_pem_passwd_cb, crl_passwd);
        if(crl_x509 == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]PEM_read_bio_X509_CRL error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            BIO_free(bio);
            X509_free(signed_crt_x509);
            X509_free(root_crt_x509);
            X509_STORE_free(ca_cert_store);
            return -1;
        }
        BIO_free(bio);
        bio = NULL;
        
        ret = X509_STORE_set_flags(ca_cert_store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_set_flags error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            X509_free(signed_crt_x509);
            X509_free(root_crt_x509);
            X509_CRL_free(crl_x509);
            X509_STORE_free(ca_cert_store);
            return -1;
        }
        
        ret = X509_STORE_add_crl(ca_cert_store ,crl_x509);
        if(ret <= 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_add_crl error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            X509_free(signed_crt_x509);
            X509_free(root_crt_x509);
            X509_CRL_free(crl_x509);
            X509_STORE_free(ca_cert_store);
            return -1;
        }
    }
    
    cert_ctx = X509_STORE_CTX_new();
    if(cert_ctx == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_CTX_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_free(signed_crt_x509);
        X509_free(root_crt_x509);
        if(crl_x509 != NULL)
        {
            X509_CRL_free(crl_x509);
        }
        X509_STORE_free(ca_cert_store);
        return -1;
    }
    
    ret = X509_STORE_CTX_init(cert_ctx, ca_cert_store, signed_crt_x509, cert_stack);
    if(ret <= 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]X509_STORE_CTX_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        X509_free(signed_crt_x509);
        X509_free(root_crt_x509);
        if(crl_x509 != NULL)
        {
            X509_CRL_free(crl_x509);
        }
        X509_STORE_free(ca_cert_store);
        X509_STORE_CTX_cleanup(cert_ctx);
        X509_STORE_CTX_free(cert_ctx);
        return -1;
    }
    
    ret = X509_verify_cert(cert_ctx);
    if(ret <= 0)
    {
        X509_free(signed_crt_x509);
        X509_free(root_crt_x509);
        if(crl_x509 != NULL)
        {
            X509_CRL_free(crl_x509);
        }
        X509_STORE_free(ca_cert_store);
        X509_STORE_CTX_cleanup(cert_ctx);
        X509_STORE_CTX_free(cert_ctx);
        return -2;
    }
    
    X509_free(signed_crt_x509);
    X509_free(root_crt_x509);
    if(crl_x509 != NULL)
    {
        X509_CRL_free(crl_x509);
    }
    X509_STORE_free(ca_cert_store);
    X509_STORE_CTX_cleanup(cert_ctx);
    X509_STORE_CTX_free(cert_ctx);
    
    return 0;
}

extern int enos_rsa_public_encrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);
    if(output_max < support_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, support_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int rsa_padding_mode = 0;
    if(padding_mode < 0)
    {
        rsa_padding_mode = ENOS_DEFAULT_RSA_PADDING;
    }
    else
    {
        ret = rsa_padding_int_to_use(padding_mode, &rsa_padding_mode);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]rsa_padding_int_to_use error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }

    switch(rsa_padding_mode)
    {
    case RSA_NO_PADDING:
        if (input_len != support_len)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d != support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        ret = RSA_public_encrypt(support_len, (const unsigned char*)input, (unsigned char*)output, rsa_key_ptr, rsa_padding_mode); // 加密
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_public_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        *output_len = ret;
        return 0;
        break;
    case RSA_PKCS1_PADDING:
        if (input_len > support_len - RSA_PKCS1_PADDING_SIZE)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d > support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len - RSA_PKCS1_PADDING_SIZE, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        ret = RSA_public_encrypt(input_len, (const unsigned char*)input, (unsigned char*)output, rsa_key_ptr, rsa_padding_mode); // 加密
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_public_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        *output_len = ret;
        return 0;
        break;
    default:
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]unsupported PADDING mode(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
        break;
    }
    return 0;
}

extern int enos_rsa_private_decrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);

    if (input_len != support_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d != support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int rsa_padding_mode = 0;
    if(padding_mode < 0)
    {
        rsa_padding_mode = ENOS_DEFAULT_RSA_PADDING;
    }
    else
    {
        ret = rsa_padding_int_to_use(padding_mode, &rsa_padding_mode);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]rsa_padding_int_to_use error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }

    switch(rsa_padding_mode)
    {
    case RSA_NO_PADDING:
    case RSA_PKCS1_PADDING:
        break;
    default:
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]unsupported PADDING mode(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
        break;
    }

    int output_temp_max = support_len + 10;
    char *output_temp = (char *)malloc(output_temp_max);
    if(output_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(output_temp, 0, output_temp_max);
    
    ret = RSA_private_decrypt(input_len, (const unsigned char*)input, (unsigned char*)output_temp, rsa_key_ptr, rsa_padding_mode); // 解密
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_private_decrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(output_temp);
        return -1;
    }
    
    if(output_max < ret)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, ret, __FILE__, __FUNCTION__, __LINE__);
        free(output_temp);
        return -1;
    }
    
    memcpy(output, output_temp, ret);
    *output_len = ret;
    
    free(output_temp);
    return 0;
}

extern int enos_rsa_public_encrypt_advance(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    memset(output, 0, output_max);
    *output_len = 0;
    
    if(input_len <= 0)
    {
        return 0;
    }
    
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);
    int pkcs1_support_len = support_len - RSA_PKCS1_PADDING_SIZE;
    
    int need_len = 0;
    if(input_len % pkcs1_support_len == 0)
    {
        need_len = (input_len / pkcs1_support_len) * support_len;
    }
    else
    {
        need_len = (input_len / pkcs1_support_len + 1) * support_len;
    }
    
    if(output_max < need_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, need_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ii = 0;
    int output_len_temp = 0;
    *output_len = 0;

    for(ii = 0; ii < input_len / pkcs1_support_len; ii++)
    {
        output_len_temp = 0;
        ret = enos_rsa_public_encrypt(pkey, input + ii * pkcs1_support_len, pkcs1_support_len, output + ii * support_len, support_len, &output_len_temp, ENOS_RSA_PADDING_PKCS1_PADDING);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_rsa_public_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            memset(output, 0, output_max);
            *output_len = 0;
            return -1;
        }
        *output_len += output_len_temp;
    }

    int left_len = input_len % pkcs1_support_len;
    if(left_len > 0)
    {
        output_len_temp = 0;
        ret = enos_rsa_public_encrypt(pkey, input + ii * pkcs1_support_len, left_len, output + ii * support_len, support_len, &output_len_temp, ENOS_RSA_PADDING_PKCS1_PADDING);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_rsa_public_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            memset(output, 0, output_max);
            *output_len = 0;
            return -1;
        }
        *output_len += output_len_temp;
    }
    
    return 0;
}

extern int enos_rsa_private_decrypt_advance(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    memset(output, 0, output_max);
    *output_len = 0;
    
    if(input_len <= 0)
    {
        return 0;
    }
    
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);
//    int pkcs1_support_len = support_len - RSA_PKCS1_PADDING_SIZE;
    
    if(input_len % support_len > 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d(%%%d > 0, need == 0) error(file=%s, function=%s, line=%d)\n", input_len, support_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int output_temp_max = input_len;
    char *output_temp = (char *)malloc(output_temp_max);
    if(output_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(output_temp, 0, output_temp_max);
    int output_temp_current_len = 0;
    
    int ii = 0;
    int output_len_temp = 0;

    for(ii = 0; ii < input_len / support_len; ii++)
    {
        output_len_temp = 0;
        ret = enos_rsa_private_decrypt(pkey, input + ii * support_len, support_len, output_temp + output_temp_current_len, output_temp_max - output_temp_current_len, &output_len_temp, ENOS_RSA_PADDING_PKCS1_PADDING);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_rsa_private_decrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            memset(output, 0, output_max);
            *output_len = 0;
            free(output_temp);
            return -1;
        }
        output_temp_current_len += output_len_temp;
    }
    
    if(output_max < output_temp_current_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, output_temp_current_len, __FILE__, __FUNCTION__, __LINE__);
        memset(output, 0, output_max);
        *output_len = 0;
        free(output_temp);
        return -1;
    }
    
    memcpy(output, output_temp, output_temp_current_len);
    *output_len = output_temp_current_len;
    
    free(output_temp);
    return 0;
}

extern int enos_rsa_public_encrypt_file(EVP_PKEY *pkey, char *file_in, char *file_out)
{
    if((pkey == NULL) || (file_in == NULL) || (file_out == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);
    int pkcs1_support_len = support_len - RSA_PKCS1_PADDING_SIZE;
    
    int file_in_len = 0;
    file_in_len = get_file_size(file_in);
    if(file_in_len < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]get_file_size %s error(file=%s, function=%s, line=%d)\n", file_in, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(file_in_len == 0)
    {
        FILE *fp = NULL;
        fp = fopen(file_out, "w+b");
        if(fp == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]fopen %s error(file=%s, function=%s, line=%d)\n", file_out, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    
        fclose(fp);
    }
    
    int need_len = 0;
    if(file_in_len % pkcs1_support_len == 0)
    {
        need_len = (file_in_len / pkcs1_support_len) * support_len;
    }
    else
    {
        need_len = (file_in_len / pkcs1_support_len + 1) * support_len;
    }
    
    int file_in_buf_max = file_in_len + 10;
    char *file_in_buf = (char *)malloc(file_in_buf_max);
    if(file_in_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]file_in_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(file_in_buf, 0, file_in_buf_max);
    
    ret = read_file_content(file_in, file_in_buf, file_in_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]read_file_content error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        return -1;
    }
    
    int file_out_buf_max = need_len + 10;
    char *file_out_buf = (char *)malloc(file_out_buf_max);
    if(file_out_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]file_out_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        return -1;
    }
    memset(file_out_buf, 0, file_out_buf_max);
    int file_out_len = 0;
    
    ret = enos_rsa_public_encrypt_advance(pkey, file_in_buf, file_in_len, file_out_buf, file_out_buf_max, &file_out_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_rsa_public_encrypt_advance error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        free(file_out_buf);
        return -1;
    }
    
    ret = write_file_content(file_out, file_out_buf, file_out_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]write_file_content error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        free(file_out_buf);
        return -1;
    }
    
    free(file_in_buf);
    free(file_out_buf);
    return 0;
}

extern int enos_rsa_private_decrypt_file(EVP_PKEY *pkey, char *file_in, char *file_out)
{
    if((pkey == NULL) || (file_in == NULL) || (file_out == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    
    int file_in_len = 0;
    file_in_len = get_file_size(file_in);
    if(file_in_len < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]get_file_size %s error(file=%s, function=%s, line=%d)\n", file_in, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(file_in_len == 0)
    {
        FILE *fp = NULL;
        fp = fopen(file_out, "w+b");
        if(fp == NULL)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]fopen %s error(file=%s, function=%s, line=%d)\n", file_out, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    
        fclose(fp);
    }
    
    int need_len = file_in_len;
    
    int file_in_buf_max = file_in_len + 10;
    char *file_in_buf = (char *)malloc(file_in_buf_max);
    if(file_in_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]file_in_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(file_in_buf, 0, file_in_buf_max);
    
    ret = read_file_content(file_in, file_in_buf, file_in_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]read_file_content error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        return -1;
    }
    
    int file_out_buf_max = need_len + 10;
    char *file_out_buf = (char *)malloc(file_out_buf_max);
    if(file_out_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]file_out_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        return -1;
    }
    memset(file_out_buf, 0, file_out_buf_max);
    int file_out_len = 0;
    
    ret = enos_rsa_private_decrypt_advance(pkey, file_in_buf, file_in_len, file_out_buf, file_out_buf_max, &file_out_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]enos_rsa_private_decrypt_advance error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        free(file_out_buf);
        return -1;
    }
    
    ret = write_file_content(file_out, file_out_buf, file_out_len);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]write_file_content error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(file_in_buf);
        free(file_out_buf);
        return -1;
    }
    
    free(file_in_buf);
    free(file_out_buf);
    return 0;
}

extern int enos_rsa_private_encrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);
    if(output_max < support_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, support_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int rsa_padding_mode = 0;
    if(padding_mode < 0)
    {
        rsa_padding_mode = ENOS_DEFAULT_RSA_PADDING;
    }
    else
    {
        ret = rsa_padding_int_to_use(padding_mode, &rsa_padding_mode);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]rsa_padding_int_to_use error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }

    switch(rsa_padding_mode)
    {
    case RSA_NO_PADDING:
        if (input_len != support_len)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d != support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        ret = RSA_private_encrypt(support_len, (const unsigned char*)input, (unsigned char*)output, rsa_key_ptr, rsa_padding_mode); // 加密
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_private_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        *output_len = ret;
        return 0;
        break;
    case RSA_PKCS1_PADDING:
        if (input_len > support_len - RSA_PKCS1_PADDING_SIZE)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d > support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len - RSA_PKCS1_PADDING_SIZE, __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        ret = RSA_private_encrypt(input_len, (const unsigned char*)input, (unsigned char*)output, rsa_key_ptr, rsa_padding_mode); // 加密
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_private_encrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        *output_len = ret;
        return 0;
        break;
    default:
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]unsupported PADDING mode(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
        break;
    }
    return 0;
}

extern int enos_rsa_public_decrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode)
{
    if((pkey == NULL) || (input == NULL) || (output == NULL) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int ret = 0;
    RSA *rsa_key_ptr = EVP_PKEY_get0_RSA(pkey);
    if(rsa_key_ptr == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_PKEY_get0_RSA error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    int support_len = RSA_size(rsa_key_ptr);

    if (input_len != support_len)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]input_len=%d != support_len=%d error(file=%s, function=%s, line=%d)\n", input_len, support_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    int rsa_padding_mode = 0;
    if(padding_mode < 0)
    {
        rsa_padding_mode = ENOS_DEFAULT_RSA_PADDING;
    }
    else
    {
        ret = rsa_padding_int_to_use(padding_mode, &rsa_padding_mode);
        if(ret < 0)
        {
            enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]rsa_padding_int_to_use error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }

    switch(rsa_padding_mode)
    {
    case RSA_NO_PADDING:
    case RSA_PKCS1_PADDING:
        break;
    default:
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]unsupported PADDING mode(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
        break;
    }

    int output_temp_max = support_len + 10;
    char *output_temp = (char *)malloc(output_temp_max);
    if(output_temp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_temp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memset(output_temp, 0, output_temp_max);
    
    ret = RSA_public_decrypt(input_len, (const unsigned char*)input, (unsigned char*)output_temp, rsa_key_ptr, rsa_padding_mode); // 解密
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]RSA_public_decrypt error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(output_temp);
        return -1;
    }
    
    if(output_max < ret)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, ret, __FILE__, __FUNCTION__, __LINE__);
        free(output_temp);
        return -1;
    }
    
    memcpy(output, output_temp, ret);
    *output_len = ret;
    
    free(output_temp);
    return 0;
}

extern int enos_hash(char *input, int input_len, char *hash_fun, char *output, int output_max, int *output_len)
{
    if((input == NULL) || (input_len <= 0) || (hash_fun == NULL) || (output == NULL) || (output_max <= 0) || (output_len == NULL))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    OpenSSL_add_all_algorithms();
    
    int ret = 0;
    EVP_MD_CTX *md_ctx = NULL;
	const EVP_MD *md = NULL;
	char md_result[256];
	memset(md_result, 0, sizeof(md_result));
	int md_result_len = 0;
	
	md = EVP_get_digestbyname(hash_fun);
	if(md == NULL)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_get_digestbyname %s error(file=%s, function=%s, line=%d)\n", hash_fun, __FILE__, __FUNCTION__, __LINE__);
        return -1;
	}
	
	md_ctx = EVP_MD_CTX_new();
	if(md_ctx == NULL)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_MD_CTX_new error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
	}
	
	ret = EVP_MD_CTX_init(md_ctx);
	if(ret <= 0)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_MD_CTX_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        EVP_MD_CTX_free(md_ctx);
        return -1;
	}
	
	ret = EVP_DigestInit_ex(md_ctx, md, NULL);
	if(ret <= 0)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_DigestInit_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    EVP_MD_CTX_free(md_ctx);
        return -1;
	}
	
	ret = EVP_DigestUpdate(md_ctx, input, (size_t)input_len);
	if(ret <= 0)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_DigestUpdate error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    EVP_MD_CTX_free(md_ctx);
        return -1;
	}

	ret = EVP_DigestFinal_ex(md_ctx, (unsigned char *)md_result, (unsigned int *)&md_result_len);
	if(ret <= 0)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]EVP_DigestFinal_ex error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
	    EVP_MD_CTX_free(md_ctx);
        return -1;
	}
	
	EVP_MD_CTX_free(md_ctx);
	
	if(md_result_len > output_max)
	{
	    enos_printf(NULL, ENOS_LOG_ERROR, "[ENOS_ACCESS_API]output_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", output_max, md_result_len, __FILE__, __FUNCTION__, __LINE__);
        return -1;
	}
	
	memcpy(output, md_result, md_result_len);
	*output_len = md_result_len;
	
	return 0;
}

