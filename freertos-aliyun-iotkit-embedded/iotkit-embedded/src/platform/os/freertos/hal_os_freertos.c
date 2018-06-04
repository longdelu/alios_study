/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <memory.h>
#include "malloc.h"
#include <string.h>
//#include <pthread.h>
//#include <unistd.h>
//#include <sys/prctl.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "iot_import.h"

#define HAL_OS_DEBUG                   

#ifdef HAL_OS_DEBUG
#define HAL_OS_DEBUG_INFO(...)    (int)printf(__VA_ARGS__)
#else
#define HAL_OS_DEBUG_INFO(...)
#endif



void *HAL_MutexCreate(void)
{
   
    //互斥信号量句柄
    SemaphoreHandle_t MutexSemaphore ;	
    
    //创建互斥信号量, 动态创建并进行了初始化
	MutexSemaphore=xSemaphoreCreateMutex();
    
    if (NULL == MutexSemaphore) {
        HAL_OS_DEBUG_INFO("create mutex failed");
        return NULL;
    }


    return MutexSemaphore;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
   
    vSemaphoreDelete(mutex);
    
}

void HAL_MutexLock(_IN_ void *mutex)
{
    int err_num;
    
    err_num = xSemaphoreTake(mutex,portMAX_DELAY);	//获取互斥信号量    
    
    if (pdTRUE != err_num) {
        HAL_OS_DEBUG_INFO("lock mutex failed");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    int err_num;
    
    err_num = xSemaphoreGive(mutex);					//释放互斥信号量
    
    if (pdTRUE != err_num) {
        HAL_OS_DEBUG_INFO("unlock mutex failed");
    }
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return pvPortMalloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    vPortFree(ptr);
}

TickType_t freertos_sys_time_get(void)
{
    return (TickType_t)(xTaskGetTickCount() * 1000 / configTICK_RATE_HZ);
}

uint64_t freertos_now_ms(void)
{
    return freertos_sys_time_get();
}


uint64_t HAL_UptimeMs(void)
{
    return freertos_now_ms();
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    
    TickType_t xDelay = ms / portTICK_PERIOD_MS;
    
    vTaskDelay( xDelay);
        
}

void HAL_Srandom(uint32_t seed)
{
   srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (rand() % region) : 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}


/* 以宏的方式移植 */
// #define HAL_Printf(_IN_ const char *fmt, ...)
//{
//    
//    va_list args;

//    va_start(args, fmt);
//    vprintf(fmt, args);
//    va_end(args);
//    
//    fflush(stdout);
//    
//}

int HAL_GetPartnerID(char pid_str[PID_STRLEN_MAX])
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
#ifdef __UBUNTU_SDK_DEMO__
    strcpy(pid_str, "example.demo.partner-id");
#endif
    return strlen(pid_str);
}

int HAL_GetModuleID(char mid_str[MID_STRLEN_MAX])
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
#ifdef __UBUNTU_SDK_DEMO__
    strcpy(mid_str, "example.demo.module-id");
#endif
    return strlen(mid_str);
}

