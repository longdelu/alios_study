#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
//////////////////////////////////////////////////////////////////////////////////    
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4&F7开发板
//UDP 测试代码      
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/2/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved                             
//*******************************************************************************
//修改信息
//开发板作为udp服务器，绑定到一个熟知的端口号上面
//////////////////////////////////////////////////////////////////////////////////       
 
//UDP接收数据缓冲区
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];   //UDP接收数据缓冲区 

/*
 * \brief UDP回调函数
 */
static void __udp_demo_recv_callback(void *arg,
		                             struct udp_pcb *upcb,
									 struct pbuf *p,
									 struct ip_addr *addr,
									 u16_t port)
{

	udp_sendto(upcb, p,addr,port); //直接将接收到的数据返回到源端

	pbuf_free(p);                  //释放掉数据包空间。
}


//UDP测试
void udp_demo_init(void)
{
 
   struct udp_pcb *udp_pcb;

//   struct ip_addr   server_ipaddr;  	//开发板本地(ip)服务器ip地址

//   IP4_ADDR(&server_ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);

   /* 创建UDP控制块 */
   udp_pcb = udp_new();
   
   /* 开发板作为服务器，绑定到一个熟知的端口(echo端口) */
   udp_bind(udp_pcb, IP_ADDR_ANY, UDP_DEMO_PORT);

   /* 注册UDP服务器接收回调函数 */
   udp_recv(udp_pcb, __udp_demo_recv_callback, NULL);
} 

























