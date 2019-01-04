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

#include "enos_encrypt_test.h"

static int myprintf(void *print_arg, int print_level, const char *format, ...);

static int myprintf(void *print_arg, int print_level, const char *format, ...)
{
    if(print_level > ENOS_ENCRYPT_TEST_CURRENT_LOG_LEVEL)
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

#if defined(ENABLE_TPM)
//rsa public key encrypt and private key decrypt(tpm2)
int rsa_public_encrypt_private_decrypt_tpm2_test()
{
    int ret = 0;
    
    char tpm2_key_path[1024];
    memset(tpm2_key_path, 0, sizeof(tpm2_key_path));
    snprintf(tpm2_key_path, sizeof(tpm2_key_path), "%s", "./enos_encrypt_test_tmp/rsa_key.tss");
    
    char *owner_hierarchy_passwd = NULL;
    
    char key_passwd[1024];
    memset(key_passwd, 0, sizeof(key_passwd));
    snprintf(key_passwd, sizeof(key_passwd), "%s", "123456");
    
    int key_size = 2048;
    EVP_PKEY *pkey = NULL;
    
    ret = enos_generate_rsa_key_tpm2(tpm2_key_path, owner_hierarchy_passwd, key_passwd, key_size, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key_tpm2 error\n");
        return -1;
    }
    
    char origin_text[1024];
    int ii = 0;
    for(ii = 0; ii < (int)(sizeof(origin_text)); ii++)
    {
        origin_text[ii] = ii & 0xff;
    }
    
    char enc_text[2048];
    memset(enc_text, 0, sizeof(enc_text));
    
    char dec_text[2048];
    memset(dec_text, 0, sizeof(dec_text));
    
    int enc_len = 0;
    int dec_len = 0;
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test fail\n", ii);
        }
    }
    EVP_PKEY_free(pkey);
    
    pkey = NULL;
    ret = enos_read_rsa_key_from_file_tpm2(tpm2_key_path, owner_hierarchy_passwd, key_passwd, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_key_from_file_tpm2 error\n");
        return -1;
    }
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test2 success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test2 fail\n", ii);
        }
    }
    EVP_PKEY_free(pkey);
    
    return 0;
}

#endif

//rsa public key encrypt and private key decrypt
int rsa_public_encrypt_private_decrypt_test()
{
    int ret = 0;
    
    char pub_key_path[1024];
    memset(pub_key_path, 0, sizeof(pub_key_path));
    snprintf(pub_key_path, sizeof(pub_key_path), "%s", "./enos_encrypt_test_tmp/pub_key.pem");
    
    char pri_key_path[1024];
    memset(pri_key_path, 0, sizeof(pri_key_path));
    snprintf(pri_key_path, sizeof(pri_key_path), "%s", "./enos_encrypt_test_tmp/pri_key.pem");
    
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
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key error\n");
        return -1;
    }
    
    EVP_PKEY *pri_key = NULL;
    ret = enos_read_rsa_pri_key_from_file(pri_key_path, key_passwd, &pri_key);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_pri_key_from_file error\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    EVP_PKEY *pub_key = NULL;
    ret = enos_read_rsa_pub_key_from_file(pub_key_path, &pub_key);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_pub_key_from_file error\n");
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(pri_key);
        return -1;
    }
    
    char origin_text[1024];
    int ii = 0;
    for(ii = 0; ii < (int)(sizeof(origin_text)); ii++)
    {
        origin_text[ii] = ii & 0xff;
    }
    
    char enc_text[2048];
    memset(enc_text, 0, sizeof(enc_text));
    
    char dec_text[2048];
    memset(dec_text, 0, sizeof(dec_text));
    
    int enc_len = 0;
    int dec_len = 0;
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test fail\n", ii);
        }
    }
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pub_key, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pri_key, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pub_key pri_key test success\n", ii);
        }
        else
        {
            printf("ii=%d, pub_key pri_key test fail\n", ii);
        }
    }
    
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(pri_key);
    EVP_PKEY_free(pub_key);
    return 0;
}

//enos_generate_cert_req
int enos_generate_cert_req_test()
{
    int ret = 0;
    
    char pub_key_path[1024];
    memset(pub_key_path, 0, sizeof(pub_key_path));
    snprintf(pub_key_path, sizeof(pub_key_path), "%s", "./enos_encrypt_test_tmp/csr_pub_key.pem");
    
    char pri_key_path[1024];
    memset(pri_key_path, 0, sizeof(pri_key_path));
    snprintf(pri_key_path, sizeof(pri_key_path), "%s", "./enos_encrypt_test_tmp/csr_pri_key.pem");
    
    char req_path[1024];
    memset(req_path, 0, sizeof(req_path));
    snprintf(req_path, sizeof(req_path), "%s", "./enos_encrypt_test_tmp/req.csr");
    
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
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key error\n");
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
    snprintf(input_info.CN_commonname, sizeof(input_info.CN_commonname), "%s", "device SN");
    
    ret = enos_generate_cert_req(req_path, &input_info, pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_cert_req error\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    return 0;
}

//hash test
int enos_hash_test()
{
    char *input = "abc123";
    int input_len = strlen(input);
    char *hash_fun = "sha1";
    
    char output[256];
    memset(output, 0, sizeof(output));
    int output_max = sizeof(output);
    
    int output_len = 0;
    int ret = 0;
    
    ret = enos_hash(input, input_len, hash_fun, output, output_max, &output_len);
    if(ret < 0)
    {
        printf("enos_hash error\n");
        return -1;
    }
    
    int ii = 0;
    printf("enos_hash input(%d):%s\n", input_len, input);
    printf("enos_hash result(%d):\n", output_len);
    for(ii = 0; ii < output_len; ii++)
    {
        printf("%02x", (unsigned char)(output[ii]));
    }
    printf("\n");
    
    return 0;
}



int main(int argc, char **argv)
{
    char *tmp_dir = "enos_encrypt_test_tmp";
    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
        #if _MSC_VER
            _mkdir(tmp_dir);
        #else
            mkdir(tmp_dir);
        #endif
    #else
        mkdir(tmp_dir, 0777);
    #endif
    
    #if defined(ENABLE_TPM)
        rsa_public_encrypt_private_decrypt_tpm2_test();
    #endif
    rsa_public_encrypt_private_decrypt_test();
    enos_generate_cert_req_test();
    enos_hash_test();
    return 0;
}