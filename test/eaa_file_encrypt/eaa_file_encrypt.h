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

#ifndef EAA_FILE_ENCRYPT_H
#define EAA_FILE_ENCRYPT_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#if defined(_WIN32) || defined(WIN32)
    #include "windows.h"
#else

#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "enos_c_api/enos_c_api.h"

#ifdef __cplusplus
}
#endif

//日志等级
enum EAA_FILE_E_LOG_LEVEL
{
    EAA_FILE_E_LOG_NOTHING = 0,
    EAA_FILE_E_LOG_ERROR,
    EAA_FILE_E_LOG_WARN,
    EAA_FILE_E_LOG_INFO,
    EAA_FILE_E_LOG_DEBUG,
    EAA_FILE_E_LOG_ALL
};

#define EAA_FILE_E_CURRENT_LOG_LEVEL EAA_FILE_E_LOG_INFO //当前日志等级

#endif