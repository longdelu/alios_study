#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "sdram.h"
#include "key.h"
#include "pcf8574.h"
#include "string.h"
#include "malloc.h"
#include "lan8720.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "iot_import.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
/************************************************
 ALIENTEK 阿波罗STM32F429开发板 FreeRTOS实验14-4
 FreeRTOS互斥信号量操作实验-HAL库版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//任务优先级
#define START_TASK_PRIO			2
//任务堆栈大小	
#define START_STK_SIZE 			256  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LOW_TASK_PRIO			3
//任务堆栈大小	
#define LOW_STK_SIZE 			256  
//任务句柄
TaskHandle_t LowTask_Handler;
//任务函数
void low_task(void *pvParameters);

//任务优先级
#define MIDDLE_TASK_PRIO 		4
//任务堆栈大小	
#define MIDDLE_STK_SIZE  		256 
//任务句柄
TaskHandle_t MiddleTask_Handler;
//任务函数
void middle_task(void *pvParameters);

//任务优先级
#define HIGH_TASK_PRIO 			5
//任务堆栈大小	
#define HIGH_STK_SIZE  			256 
//任务句柄
TaskHandle_t HighTask_Handler;
//任务函数
void high_task(void *pvParameters);

//互斥信号量句柄

SemaphoreHandle_t MutexSemaphore;	//互斥信号量


//任务函数
void display_task(void *pdata);

void show_address(u8 mode)
{
	if(mode==2)
	{
		printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址

		printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址

		printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址

	}
	else 
	{
		printf("Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址

		printf("Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址


	}	
}

int main(void)
{   
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz   
    HAL_Init();                     //初始化HAL库
    

    delay_init(180);                //初始化延时函数
    uart_init(115200);              //初始化串口
    LED_Init();                     //初始化LED 
    KEY_Init();                     //初始化按键
    PCF8574_Init();                 //初始化PCF8574
    SDRAM_Init();                   //初始化SDRAM
    my_mem_init(SRAMIN);            //初始化内部内存池	
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMCCM);		    //初始化CCM内存池
    
    printf("Apollo STM32F4/F7\r\n");
	printf("LWIP+UCOSII Test\r\n");
	printf("ATOM@ALIENTEK\r\n");
	printf("2016/1/14\r\n");
    
    /* 组优先级有4位，次优先级也有4位，但stm32当中只有4位有效 */
//    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    
    while(lwip_comm_init()) 	    //lwip初始化
	{
		printf("Lwip Init failed!"); 	//lwip初始化失败

		delay_ms(500);
	}
    
    printf("lwip init sucess\r\n");
    
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄                
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
	//创建互斥信号量
	MutexSemaphore=xSemaphoreCreateMutex();
	
    //创建高优先级任务
    xTaskCreate((TaskFunction_t )high_task,             
                (const char*    )"high_task",           
                (uint16_t       )HIGH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )HIGH_TASK_PRIO,        
                (TaskHandle_t*  )&HighTask_Handler);   
    //创建中等优先级任务
    xTaskCreate((TaskFunction_t )middle_task,     
                (const char*    )"middle_task",   
                (uint16_t       )MIDDLE_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )MIDDLE_TASK_PRIO,
                (TaskHandle_t*  )&MiddleTask_Handler); 
	//创建低优先级任务
    xTaskCreate((TaskFunction_t )low_task,     
                (const char*    )"low_task",   
                (uint16_t       )LOW_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LOW_TASK_PRIO,
                (TaskHandle_t*  )&LowTask_Handler);
                
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif
                
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//高优先级任务的任务函数
void high_task(void *pvParameters)
{
  
	while(1)
	{
#if LWIP_DHCP									//当开启DHCP的时候
		if(lwipdev.dhcpstatus != 0) 			//DHCP并成功获取到IP地址时
		{
			show_address(lwipdev.dhcpstatus );	//显示地址信息
			vTaskSuspend(HighTask_Handler); 		//显示完地址信息后挂起自身任务
		}
#else
		show_address(0); 						//显示静态地址
		vTaskSuspend(HighTask_Handler);		 	//显示完地址信息后挂起自身任务
#endif //LWIP_DHCP
        
		vTaskDelay(500);	//延时500ms，也就是500个时钟节拍	

	}
}

//中等优先级任务的任务函数
void middle_task(void *pvParameters)
{

    //HAL_Printf("middle_task  Running!\r\n");

	while(1)
	{
        HAL_Printf("middle_task  Running!\r\n");
        LED0 = !LED0;
		vTaskDelay(500);	//延时500ms，也就是500个时钟节拍	

	}
}


//低优先级任务的任务函数
void low_task(void *pvParameters)
{
	static u32 times;

	while(1)
	{
        HAL_Printf("lowerst_task  Running!\r\n");
		//xSemaphoreTake(MutexSemaphore,portMAX_DELAY);	//获取互斥信号量

		//xSemaphoreGive(MutexSemaphore);					//释放互斥信号量
		vTaskDelay(1000);	//延时1s，也就是1000个时钟节拍	
	}
}

