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

#include "eaa_file_encrypt.h"

static int myprintf(void *print_arg, int print_level, const char *format, ...);

static int myprintf(void *print_arg, int print_level, const char *format, ...)
{
    if(print_level > EAA_FILE_E_CURRENT_LOG_LEVEL)
    {
        return -1;
    }

    char sprint_buf[1024];
    va_list args;
    int n;
    va_start(args, format);
    n = vsnprintf(sprint_buf, sizeof(sprint_buf), format, args);
    va_end(args);
    #if defined(_WIN32) || defined(WIN32)
        _write(_fileno(stdout), sprint_buf, n);
    #else
        write(STDOUT_FILENO, sprint_buf, n);
    #endif
    return n;
}

static int show_help(char *program_name)
{
    myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]usage:<%s> <mode> <rsa_key_path> <file_src_path> <file_dst_path>\n", program_name);
    myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]mode=0 encrypt, mode=1 decrypt\n");
    
    return 0;
}

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        show_help(argv[0]);
        return -1;
    }
    
    int ret = 0;
    int mode = 0;
    sscanf(argv[1], "%d", &mode);
    char rsa_key_path[1024];
    memset(rsa_key_path, 0, sizeof(rsa_key_path));
    snprintf(rsa_key_path, sizeof(rsa_key_path), "%s", argv[2]);
    
    char file_src_path[1024];
    memset(file_src_path, 0, sizeof(file_src_path));
    snprintf(file_src_path, sizeof(file_src_path), "%s", argv[3]);
    
    char file_dst_path[1024];
    memset(file_dst_path, 0, sizeof(file_dst_path));
    snprintf(file_dst_path, sizeof(file_dst_path), "%s", argv[4]);
    
    if(mode == 0)
    {//¼ÓÃÜ
        EVP_PKEY *pkey = NULL;
        ret = enos_read_rsa_key_from_file_tpm2(rsa_key_path, NULL, NULL, &pkey);
        if(ret < 0)
        {
            myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]enos_read_rsa_key_from_file_tpm2 error\n");
            return -1;
        }
        
        ret = enos_rsa_public_encrypt_file(pkey, file_src_path, file_dst_path);
        if(ret < 0)
        {
            myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]enos_rsa_public_encrypt_file error\n");
            EVP_PKEY_free(pkey);
            return -1;
        }
        EVP_PKEY_free(pkey);
        return 0;
    } 
    else if(mode == 1)
    {
        EVP_PKEY *pkey = NULL;
        ret = enos_read_rsa_key_from_file_tpm2(rsa_key_path, NULL, NULL, &pkey);
        if(ret < 0)
        {
            myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]enos_read_rsa_key_from_file_tpm2 error\n");
            return -1;
        }
        
        ret = enos_rsa_private_decrypt_file(pkey, file_src_path, file_dst_path);
        if(ret < 0)
        {
            myprintf(NULL, EAA_FILE_E_LOG_ERROR, "[EAA_FILE_E]enos_rsa_private_decrypt_file error\n");
            EVP_PKEY_free(pkey);
            return -1;
        }
        EVP_PKEY_free(pkey);
        return 0;
    }
    else
    {
        show_help(argv[0]);
        return -1;
    }
    
    return 0;
}