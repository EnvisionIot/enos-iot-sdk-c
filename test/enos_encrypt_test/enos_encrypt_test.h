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

#ifndef ENOS_ENCRYPT_TEST_H
#define ENOS_ENCRYPT_TEST_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#if defined(_WIN32) || defined(WIN32)
    #include "windows.h"
    #include "direct.h"
#else
    #include "sys/stat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "enos_c_api/enos_c_api.h"

#ifdef __cplusplus
}
#endif

//log level
enum ENOS_ENCRYPT_TEST_LOG_LEVEL
{
    ENOS_ENCRYPT_TEST_LOG_NOTHING = 0,
    ENOS_ENCRYPT_TEST_LOG_ERROR,
    ENOS_ENCRYPT_TEST_LOG_WARN,
    ENOS_ENCRYPT_TEST_LOG_INFO,
    ENOS_ENCRYPT_TEST_LOG_DEBUG,
    ENOS_ENCRYPT_TEST_LOG_ALL
};

//current log level
#define ENOS_ENCRYPT_TEST_CURRENT_LOG_LEVEL ENOS_ENCRYPT_TEST_LOG_ALL

#endif