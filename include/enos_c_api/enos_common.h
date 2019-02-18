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

#ifndef ENOS_COMMON_H
#define ENOS_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
    #if _MSC_VER
        #if defined(ENOS_C_API_EXPORTS) || defined(enos_c_api_EXPORTS)
            #define ENOS_C_API_DLL_EXPORT __declspec(dllexport)
        #else
            #define ENOS_C_API_DLL_EXPORT __declspec(dllimport)
        #endif
        #define _CRT_SECURE_NO_WARNINGS
        #define snprintf _snprintf
        #define vsnprintf _vsnprintf
        #define write _write
    #else
        #define ENOS_C_API_DLL_EXPORT
    #endif
#else
    #define ENOS_C_API_DLL_EXPORT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
//    #include <winsock2.h>
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/time.h>
#endif

#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509v3.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#include "openssl/bio.h"
#include "openssl/evp.h"
#include "openssl/ui.h"

#include "pthread.h"
#include "cjson/cJSON.h"
#include "iconv/iconv.h"

//log level
enum ENOS_LOG_LEVEL
{
    ENOS_LOG_NOTHING = 0,
    ENOS_LOG_ERROR,
    ENOS_LOG_WARN,
    ENOS_LOG_INFO,
    ENOS_LOG_DEBUG,
    ENOS_LOG_ALL
};

//current log level
#define ENOS_CURRENT_LOG_LEVEL ENOS_LOG_ERROR

//default charset of current file
#define ENOS_CURRENT_FILE_CHARSET "GBK"

//default charset of application
#define ENOS_DEFAULT_APP_CHARSET "UTF-8"

//default charset of server
#define ENOS_DEFAULT_CLOUD_CHARSET "UTF-8"

//max length of general buf
#define ENOS_GENERAL_BUF_MAX 128

//max length of a path
#define ENOS_PATH_BUF_MAX 1024

#define ENOS_CHARSET_TRANS_MULTI 2

//pair of para name and para value 
struct ENOS_C_API_DLL_EXPORT enos_para_value
{
    char para_name[ENOS_GENERAL_BUF_MAX];
    char para_value[ENOS_GENERAL_BUF_MAX];
};

//common function-->

/************************************************************
 * name:enos_printf
 * desc:common printf function
 *
 * para:[in] print_arg          reserve
 *      [in] print_level        print_level
 *      [in] format             print format
 *      [in] ...                print parameters
 * return:>=0         success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_printf(void *print_arg, int print_level, const char *format, ...);

/************************************************************
 * name:enos_char_code_convert
 * desc:transform charset from from_charset to to_charset
 *
 * para:[in]  from_charset          source charset
 *      [in]  to_charset            dst charset
 *      [in]  inbuf                 source buf
 *      [in]  inlen                 source len
 *      [out] outbuf                dst buf
 *      [in]  outbuf_max_len        max length of dst buf
 *      [out] out_len               dst len
 * return:>=0         success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_char_code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outbuf_max_len, int *out_len);

/************************************************************
 * name:enos_gettimeofday
 * desc:get current time
 *
 * para:[out]  tvp          struct timeval
 *      [out]  tzp          timezone
 * return:>=0         success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_gettimeofday(struct timeval *tvp, void *tzp);

/************************************************************
 * name:enos_cal_time_gap
 * desc:calculate tvp2-tvp1(ms)
 *
 * para:[in] tvp1        struct timeval  
 *      [in] tvp2        struct timeval
 * return:result of tvp2-tvp1(ms)
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern long long int enos_cal_time_gap(struct timeval *tvp1, struct timeval *tvp2);

/************************************************************
 * name:enos_cal_time_elapsed
 * desc:calculate tvp-current_time(ms)
 *
 * para:[in] tvp        struct timeval  
 * return:result of tvp-current_time(ms)
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern long long int enos_cal_time_elapsed(struct timeval *tvp);

/************************************************************
 * name:enos_sleep
 * desc:sleep
 *
 * para:[in] ms       time to sleep(ms)
 * return:>=0         success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_sleep(long long int ms);
//<--common function

#ifdef __cplusplus
}
#endif

#endif