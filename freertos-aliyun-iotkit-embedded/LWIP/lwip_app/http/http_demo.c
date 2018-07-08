#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/timers.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include <stdio.h>	

#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "lwip_comm.h" 

#define HTTP_PORT 80

const unsigned char htmldata[] = "	\
        <html>	\
        <head><title> A LwIP WebServer !!</title></head> \
	    <center><p>A WebServer Based on LwIP v1.4.1!</center>\
	    </html>";

static void http_server_init(void);




//@@code for a http server
//@@send HTTP data to any connected client, and then close connection
static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  char *data = NULL;
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	tcp_recved(pcb, p->tot_len);

    data =  p->payload;
	if(p->len >=3 && data[0] == 'G'&& data[1] == 'E'&& data[2] == 'T')
	{
        tcp_write(pcb, htmldata, sizeof(htmldata), 1);
    }
	else
	{
	    printf("Request error\n");
	}
     pbuf_free(p);
	 tcp_close(pcb);
  } 
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
    return tcp_close(pcb);
  }
  return ERR_OK;
}

/**
  * @brief  This function when the Telnet connection is established
  * @param  arg  user supplied argument 
  * @param  pcb	 the tcp_pcb which accepted the connection
  * @param  err	 error value
  * @retval ERR_OK
  */

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     

  tcp_recv(pcb, http_recv);
  return ERR_OK;
}
/**
  * @brief  Initialize the http application  
  * @param  None 
  * @retval None 
  */
 
static void http_server_init(void)
{
  struct tcp_pcb *pcb = NULL;	            		
  
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	

  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, HTTP_PORT);       


  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				

  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, http_accept);   
										
}

void http_demo(void)
{
    
    u8 *tbuf;       

    printf("http Test \r\n");
    tbuf=mymalloc(SRAMIN,200);    //申请内存
    if(tbuf==NULL)return ;        //内存申请失败了,直接退出
    sprintf((char*)tbuf,"http server IP:%d.%d.%d.%d",lwipdev.ip[0], lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//客户端IP
    printf("%s",tbuf);      
	
    http_server_init();
    
	while(1)
	{

      delay_ms(100);
	}
}


