/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/*  Porting by Michael Vysotsky <michaelvy@hotmail.com> August 2011   */

#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "includes.h"
#include "delay.h"
#include "arch/sys_arch.h"
#include "malloc.h"

#if !NO_SYS

#define SCB_ICSR_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
    
xTaskHandle xTaskGetCurrentTaskHandle( void ) PRIVILEGED_FUNCTION;

struct timeoutlist
{
	struct sys_timeouts timeouts;
	xTaskHandle pid;
};

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX 6

static struct timeoutlist s_timeoutlist[SYS_THREAD_MAX];
static u16_t s_nextthread = 0;

 

//创建一个消息邮箱
//*mbox:消息邮箱
//size:邮箱大小
//返回值:ERR_OK,创建成功
//         其他,创建失败
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	
	(void)size;
	
	*mbox = xQueueCreate(MAX_QUEUES, MAX_QUEUE_ENTRIES);
    
    (void)mbox;

#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
      if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
         lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
	  }
#endif /* SYS_STATS */

	return ERR_OK;
} 
//释放并删除一个消息邮箱
//*mbox:要删除的消息邮箱
void sys_mbox_free(sys_mbox_t *mbox)
{
	if( uxQueueMessagesWaiting(*mbox) )
	{
		/* Line for breakpoint.  Should never break here! */
		portNOP();
#if SYS_STATS
	    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
			
		// TODO notify the user of failure.
        
        printf("free mbox failed\r\n");
	}

	vQueueDelete( *mbox );

#if SYS_STATS
     --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}
//向消息邮箱中发送一条消息(必须发送成功)
//*mbox:消息邮箱
//*msg:要发送的消息
void sys_mbox_post(sys_mbox_t *mbox,void *msg)
{  
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
    
    if(SCB_ICSR_REG&0xFF)  //在中断里
	{
       while(xQueueSendToBackFromISR(*mbox, &msg, &xHigherPriorityTaskWoken) != pdPASS);
	   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);;
	}
	else  //在线程
	{
	    while (xQueueSendToBack(*mbox, &msg, portMAX_DELAY ) != pdTRUE );
	}
    
}
//尝试向一个消息邮箱发送消息
//此函数相对于sys_mbox_post函数只发送一次消息，
//发送失败后不会尝试第二次发送
//*mbox:消息邮箱
//*msg:要发送的消息
//返回值:ERR_OK,发送OK
// 	     ERR_MEM,发送失败
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{ 
    err_t result = ERR_OK;
    
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
    
    if(SCB_ICSR_REG&0xFF)//在中断里
	{
		if(xQueueSendToBackFromISR(*mbox, &msg, &xHigherPriorityTaskWoken)!= pdPASS) {

#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */            
	        return ERR_MEM;
        }
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else  //在线程
	{
       if (xQueueSend(*mbox, &msg, 0 ) != pdPASS )
       {
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
           result = ERR_MEM;
       }
	}


			   
     return result;
}

//等待邮箱中的消息
//*mbox:消息邮箱
//*msg:消息
//timeout:超时时间，如果timeout为0的话,就一直等待
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{ 
    void *dummyptr;
    portTickType StartTime, EndTime, Elapsed;

	StartTime = xTaskGetTickCount();

	if ( msg == NULL )
	{
		msg = &dummyptr;
	}
		
	if ( timeout != 0 )
	{
		if ( pdTRUE == xQueueReceive(*mbox, &(*msg), timeout / portTICK_RATE_MS ) )
		{
			EndTime = xTaskGetTickCount();
			Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;
			
			return ( Elapsed );
		}
		else // timed out blocking for message
		{
			*msg = NULL;
			
			return SYS_ARCH_TIMEOUT;
		}
	}
	else // block forever for a message.
	{
		while( pdTRUE != xQueueReceive(*mbox, &(*msg), portMAX_DELAY ) ){} // time is arbitrary
		EndTime = xTaskGetTickCount();
		Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;
		
		return ( Elapsed ); // return time blocked TODO test	
	}
}
//尝试获取消息
//*mbox:消息邮箱
//*msg:消息
//返回值:等待消息所用的时间/SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
//	return sys_arch_mbox_fetch(mbox,msg,1);//尝试获取一个消息
	
	void *dummyptr;

	if ( msg == NULL )
	{
		msg = &dummyptr;
	}

	if ( pdTRUE == xQueueReceive(*mbox, &(*msg), 0 ) )
	{
	  return ERR_OK;
	}
	else
	{
	  return SYS_MBOX_EMPTY;
	}
}
//检查一个消息邮箱是否有效
//*mbox:消息邮箱
//返回值:1,有效.
//      0,无效
int sys_mbox_valid(sys_mbox_t *mbox)
{  
	if(*mbox != NULL)
	{
	   return 1;
	}		
	return 0;
    
} 

//设置一个消息邮箱为无效
//*mbox:消息邮箱
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	*mbox=NULL;
    (void)mbox;
} 
//创建一个信号量
//*sem:创建的信号量
//count:信号量值
//返回值:ERR_OK,创建OK
// 	     ERR_MEM,创建失败
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{  

	vSemaphoreCreateBinary( *sem );
    
	if( sem == NULL )
	{
		
#if SYS_STATS
      ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
			
		return ERR_MEM;	// TODO need assert
	}
	
	if(count == 0)	// Means it can't be taken
	{
		xSemaphoreTake(*sem, 1);
	}

#if SYS_STATS
	++lwip_stats.sys.sem.used;
 	if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
		lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
	}
#endif /* SYS_STATS */
		
	return ERR_OK;
} 
//等待一个信号量
//*sem:要等待的信号量
//timeout:超时时间
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{ 
    portTickType StartTime, EndTime, Elapsed;

	StartTime = xTaskGetTickCount();

	if(	timeout != 0)
	{
		if( xSemaphoreTake( *sem, timeout / portTICK_RATE_MS ) == pdTRUE )
		{
			EndTime = xTaskGetTickCount();
			Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;
			
			return (Elapsed); // return time blocked TODO test	
		}
		else
		{
			return SYS_ARCH_TIMEOUT;
		}
	}
	else // must block without a timeout
	{
		while( xSemaphoreTake( *sem, portMAX_DELAY ) != pdTRUE ){}
		EndTime = xTaskGetTickCount();
		Elapsed = (EndTime - StartTime) * portTICK_RATE_MS;

		return ( Elapsed ); // return time blocked	
		
	}
}
//发送一个信号量
//sem:信号量指针
void sys_sem_signal(sys_sem_t *sem)
{
	xSemaphoreGive(*sem);
}
//释放并删除一个信号量
//sem:信号量指针
void sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
      --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
			
	vQueueDelete(*sem);
    
} 
//查询一个信号量的状态,无效或有效
//sem:信号量指针
//返回值:1,有效.
//      0,无效
int sys_sem_valid(sys_sem_t *sem)
{
    return uxSemaphoreGetCount(*sem);
             
} 
//设置一个信号量无效
//sem:信号量指针
void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem=NULL;
    (void)sem;
      
} 

//arch初始化
void sys_init(void)
{ 
	int i;

	// Initialize the the per-thread sys_timeouts structures
	// make sure there are no valid pids in the list
	for(i = 0; i < SYS_THREAD_MAX; i++)
	{
		s_timeoutlist[i].pid = 0;
		s_timeoutlist[i].timeouts.next = NULL;
	}

	// keep track of how many threads have been created
	s_nextthread = 0;
} 




//创建一个新进程
//*name:进程名称
//thred:进程任务函数
//*arg:进程任务函数的参数
//stacksize:进程任务的堆栈大小
//prio:进程任务的优先级
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    static xTaskHandle CreatedTask;
    int result;

    if ( s_nextthread < SYS_THREAD_MAX )
    {
      result = xTaskCreate( thread, name, stacksize, arg, prio, &CreatedTask );

       // For each task created, store the task handle (pid) in the timers array.
       // This scheme doesn't allow for threads to be deleted
       s_timeoutlist[s_nextthread++].pid = CreatedTask;

       if(result == pdPASS)
       {
           return CreatedTask;
       }
       else
       {
           return NULL;
       }
    }
    else
    {
      return NULL;
    }
}

//获取系统时间,LWIP1.4.1增加的函数
//返回值:当前系统时间(单位:毫秒)
u32_t sys_now(void)
{         
	return freertos_now_ms(); 	//返回lwip_time;
}


/*用在cc.h的SYS_ARCH_PROTECT(lev)*/
uint32_t Enter_Critical(void)
{
	if(SCB_ICSR_REG&0xFF)//在中断里
	{
		return taskENTER_CRITICAL_FROM_ISR();
	}
	else  //在线程
	{
		taskENTER_CRITICAL();
		return 0;
	}
}
/*用在cc.YS_ARCH_UNPROTECT(lev)*/
void Exit_Critical(uint32_t lev)
{
	if(SCB_ICSR_REG&0xFF)//在中断里
	{
		taskEXIT_CRITICAL_FROM_ISR(lev);
	}
	else  //在线程
	{
		taskEXIT_CRITICAL();
	}
}

#else 

//为LWIP提供计时
extern uint32_t lwip_localtime;//lwip本地时间计数器,单位:ms
u32_t sys_now(void){
	return lwip_localtime;
}

#endif



