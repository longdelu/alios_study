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
 ALIENTEK ������STM32F429������ FreeRTOSʵ��14-4
 FreeRTOS�����ź�������ʵ��-HAL��汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO			2
//�����ջ��С	
#define START_STK_SIZE 			256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define LOW_TASK_PRIO			3
//�����ջ��С	
#define LOW_STK_SIZE 			256  
//������
TaskHandle_t LowTask_Handler;
//������
void low_task(void *pvParameters);

//�������ȼ�
#define MIDDLE_TASK_PRIO 		4
//�����ջ��С	
#define MIDDLE_STK_SIZE  		256 
//������
TaskHandle_t MiddleTask_Handler;
//������
void middle_task(void *pvParameters);

//�������ȼ�
#define HIGH_TASK_PRIO 			5
//�����ջ��С	
#define HIGH_STK_SIZE  			256 
//������
TaskHandle_t HighTask_Handler;
//������
void high_task(void *pvParameters);

//�����ź������

SemaphoreHandle_t MutexSemaphore;	//�����ź���


//������
void display_task(void *pdata);

void show_address(u8 mode)
{
	if(mode==2)
	{
		printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ

		printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ

		printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ

	}
	else 
	{
		printf("Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ

		printf("Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ


	}	
}

int main(void)
{   
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz   
    HAL_Init();                     //��ʼ��HAL��
    

    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ������
    LED_Init();                     //��ʼ��LED 
    KEY_Init();                     //��ʼ������
    PCF8574_Init();                 //��ʼ��PCF8574
    SDRAM_Init();                   //��ʼ��SDRAM
    my_mem_init(SRAMIN);            //��ʼ���ڲ��ڴ��	
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);		    //��ʼ��CCM�ڴ��
    
    printf("Apollo STM32F4/F7\r\n");
	printf("LWIP+UCOSII Test\r\n");
	printf("ATOM@ALIENTEK\r\n");
	printf("2016/1/14\r\n");
    
    /* �����ȼ���4λ�������ȼ�Ҳ��4λ����stm32����ֻ��4λ��Ч */
//    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    
    while(lwip_comm_init()) 	    //lwip��ʼ��
	{
		printf("Lwip Init failed!"); 	//lwip��ʼ��ʧ��

		delay_ms(500);
	}
    
    printf("lwip init sucess\r\n");
    
    //������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������                
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
	//���������ź���
	MutexSemaphore=xSemaphoreCreateMutex();
	
    //���������ȼ�����
    xTaskCreate((TaskFunction_t )high_task,             
                (const char*    )"high_task",           
                (uint16_t       )HIGH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )HIGH_TASK_PRIO,        
                (TaskHandle_t*  )&HighTask_Handler);   
    //�����е����ȼ�����
    xTaskCreate((TaskFunction_t )middle_task,     
                (const char*    )"middle_task",   
                (uint16_t       )MIDDLE_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )MIDDLE_TASK_PRIO,
                (TaskHandle_t*  )&MiddleTask_Handler); 
	//���������ȼ�����
    xTaskCreate((TaskFunction_t )low_task,     
                (const char*    )"low_task",   
                (uint16_t       )LOW_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LOW_TASK_PRIO,
                (TaskHandle_t*  )&LowTask_Handler);
                
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
#endif
                
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//�����ȼ������������
void high_task(void *pvParameters)
{
  
	while(1)
	{
#if LWIP_DHCP									//������DHCP��ʱ��
		if(lwipdev.dhcpstatus != 0) 			//DHCP���ɹ���ȡ��IP��ַʱ
		{
			show_address(lwipdev.dhcpstatus );	//��ʾ��ַ��Ϣ
			vTaskSuspend(HighTask_Handler); 		//��ʾ���ַ��Ϣ�������������
		}
#else
		show_address(0); 						//��ʾ��̬��ַ
		vTaskSuspend(HighTask_Handler);		 	//��ʾ���ַ��Ϣ�������������
#endif //LWIP_DHCP
        
		vTaskDelay(500);	//��ʱ500ms��Ҳ����500��ʱ�ӽ���	

	}
}

//�е����ȼ������������
void middle_task(void *pvParameters)
{

    //HAL_Printf("middle_task  Running!\r\n");

	while(1)
	{
        HAL_Printf("middle_task  Running!\r\n");
        LED0 = !LED0;
		vTaskDelay(500);	//��ʱ500ms��Ҳ����500��ʱ�ӽ���	

	}
}


//�����ȼ������������
void low_task(void *pvParameters)
{
	static u32 times;

	while(1)
	{
        HAL_Printf("lowerst_task  Running!\r\n");
		//xSemaphoreTake(MutexSemaphore,portMAX_DELAY);	//��ȡ�����ź���

		//xSemaphoreGive(MutexSemaphore);					//�ͷŻ����ź���
		vTaskDelay(1000);	//��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}

