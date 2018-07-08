#include "tcp_server_demo.h" 
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
//TCP 测试代码      
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/2/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved                             
//*******************************************************************************
//修改信息
//开发板作为tcp服务器，绑定到一个熟知的端口号上面
//////////////////////////////////////////////////////////////////////////////////          、

/* TCP Client接收数据缓冲区 */
u8 tcp_server_recvbuf[TCP_SERVER_RX_BUFSIZE];    
//TCP服务器发送数据内容

const u8 *tcp_server_sendbuf="TCP server send data\r\n";

//TCP Client 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上服务器;1,连接上服务器了.
//bit4~0:保留
u8 tcp_server_flag;




//TCP测试
void tcp_server_demo_init(void)
{
    
    struct tcp_pcb *tcppcbnew;  	//定义一个TCP服务器控制块
	struct tcp_pcb *tcppcbconn;  	//定义一个TCP服务器控制块
    
    u8 *tbuf;
    u8 key;
    u8 res=0;        
    u8 t=0; 
    
    int err = 0;
    
    int i =0;
    
    printf("TCP SERVER Test");
    printf("KEY0:Send data");  
    printf("KEY_UP:Quit");  
    printf("When break,please quit!");  
    tbuf=mymalloc(SRAMIN,200);    //申请内存
    if(tbuf==NULL)return ;        //内存申请失败了,直接退出
    sprintf((char*)tbuf,"Local server IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
    printf("%s",tbuf);  
    sprintf((char*)tbuf,"server Port:%d",TCP_SERVER_PORT);//客户端端口号
    printf("%s",tbuf);
    printf("STATUS:Disconnected\r\n"); 
    tcppcbnew=tcp_new();    //创建一个新的pcb
    if(tcppcbnew) {         //创建成功

		err=tcp_bind(tcppcbnew,IP_ADDR_ANY,TCP_SERVER_PORT);	//将本地IP与指定的端口号绑定在一起,IP_ADDR_ANY为绑定本地所有的IP地址
		if(err==ERR_OK)	//绑定完成
		{
			tcppcbconn=tcp_listen(tcppcbnew); 			//设置tcppcb进入监听状态
			tcp_accept(tcppcbconn,tcp_server_accept); 	//初始化LWIP的tcp_accept的回调函数
		}else res=1; 

    }else {
        res=1;
    }
    while(res==0)
    {
        key=KEY_Scan(0);
        if(key==WKUP_PRES)break;
        if(key==KEY0_PRES)//KEY0按下了,发送数据
        {
            tcp_server_usersent(tcppcbnew);    //发送数据
        }
        if(tcp_server_flag&1<<6) {           //是否收到数据?

            printf("%s",tcp_server_recvbuf); //显示接收到的数据

            tcp_server_flag&=~(1<<6);        //标记数据已经被处理了.
        } if(tcp_server_flag&1<<5)  {        //是否连接上
            
             if (i == 0) {
                 sprintf((char*)tbuf,"Client IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//客户端IP
                 printf("%s\r\n",tbuf);
                 
                 i = 1;
             }                 
            
//             printf(" STATUS:Connected\r\n   ");//提示消息
//             printf("Receive Data:\r\n");//提示消息    
        
        }else if((tcp_server_flag&1<<5)==0)
        {
             // printf("STATUS:Disconnected\r\n");

        } 

        delay_ms(2);
        t++;
        if(t==200)
        {
            t=0;
            LED0 = ~LED0;
        }        
    }
    tcp_server_connection_close(tcppcbnew,0);//关闭TCP Client连接
    tcp_server_connection_close(tcppcbconn,0);//关闭TCP Server连接 
	tcp_server_remove_timewait(); 
	memset(tcppcbnew,0,sizeof(struct tcp_pcb));
	memset(tcppcbconn,0,sizeof(struct tcp_pcb)); 

    printf("TCP Server Test\rn"); 
    
    printf("Connect break\r\n");  
    printf("KEY1:Connect\r\n");
    myfree(SRAMIN,tbuf);
} 

/* lwIP tcp_recv()函数的回调函数 */
err_t tcp_server_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
    u32 data_len = 0;
    struct pbuf *q;
    struct tcp_server_struct *es;
    err_t ret_err; 
    LWIP_ASSERT("arg != NULL",arg != NULL);
    es=(struct tcp_server_struct *)arg; 
    if(p==NULL)//从客户端接收到空数据接
    {
        es->state=ES_TCPSERVER_CLOSING;//需要关闭TCP 连接了 
         es->p=p; 
        ret_err=ERR_OK;
    }else if(err!= ERR_OK)//当接收到一个非空的数据帧,但是err!=ERR_OK
    { 
        if(p)pbuf_free(p);//释放接收pbuf
        ret_err=err;
    }else if(es->state==ES_TCPSERVER_ACCEPTED)    //当处于连接状态时
    {
        if(p!=NULL)//当处于连接状态并且接收到的数据不为空时
        {
            memset(tcp_server_recvbuf,0,TCP_SERVER_RX_BUFSIZE);  //数据接收缓冲区清零
            for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
            {
                //判断要拷贝到TCP_SERVER_RX_BUFSIZE中的数据是否大于TCP_SERVER_RX_BUFSIZE的剩余空间，如果大于
                //的话就只拷贝TCP_SERVER_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
                if(q->len > (TCP_SERVER_RX_BUFSIZE-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(TCP_SERVER_RX_BUFSIZE-data_len));//拷贝数据
                else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
                data_len += q->len;      
                if(data_len > TCP_SERVER_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出    
            }
            tcp_server_flag|=1<<6;        //标记接收到数据了
            lwipdev.remoteip[0]=tpcb->remote_ip.addr&0xff; 		//IADDR4
			lwipdev.remoteip[1]=(tpcb->remote_ip.addr>>8)&0xff; //IADDR3
			lwipdev.remoteip[2]=(tpcb->remote_ip.addr>>16)&0xff;//IADDR2
			lwipdev.remoteip[3]=(tpcb->remote_ip.addr>>24)&0xff;//IADDR1 
            tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
            pbuf_free(p);      //释放内存
            ret_err=ERR_OK;
        }
    }else  //接收到数据但是连接已经关闭,
    { 
        tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
        es->p=NULL;
        pbuf_free(p); //释放内存
        ret_err=ERR_OK;
    }
    return ret_err;
}

//lwIP tcp_err函数的回调函数
void tcp_server_error(void *arg,err_t err)
{  
    //这里我们不做任何处理
    LWIP_UNUSED_ARG(err);  
	printf("tcp error:%x\r\n",(u32)arg);
	if(arg!=NULL)mem_free(arg);//释放内存
} 

//LWIP数据发送，用户应用程序调用此函数来发送数据
//tpcb:TCP控制块
//返回值:0，成功；其他，失败
err_t tcp_server_usersent(struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es; 
    es=tpcb->callback_arg;
    if(es!=NULL)  //连接处于空闲可以发送数据
    {
        es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_server_sendbuf),PBUF_POOL);    //申请内存 
        pbuf_take(es->p,(char*)tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf));    //将tcp_server_sentbuf[]中的数据拷贝到es->p_tx中
        tcp_server_senddata(tpcb,es);//将tcp_server_sentbuf[]里面复制给pbuf的数据发送出去
        tcp_server_flag&=~(1<<7);    //清除数据发送标志
        if(es->p)pbuf_free(es->p);    //释放内存
        ret_err=ERR_OK;
    }else
    { 
        tcp_abort(tpcb);//终止连接,删除pcb控制块
        ret_err=ERR_ABRT;
    }
    return ret_err;
}
//lwIP tcp_poll的回调函数
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es; 
    es=(struct tcp_server_struct*)arg;
    if(es->state==ES_TCPSERVER_CLOSING)         //连接断开
    { 
         tcp_server_connection_close(tpcb,es);   //关闭TCP连接
    } 
    ret_err=ERR_OK;
    return ret_err;
} 

//lwIP tcp_accept()的回调函数
err_t tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
 	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(newpcb,TCP_PRIO_MIN);//设置新创建的pcb优先级
	es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //分配内存
 	if(es!=NULL) //内存分配成功
	{
		es->state=ES_TCPSERVER_ACCEPTED;  	//接收连接
		es->pcb=newpcb;
		es->p=NULL;
		
		tcp_arg(newpcb,es);
		tcp_recv(newpcb,tcp_server_recv);	//初始化tcp_recv()的回调函数
		tcp_err(newpcb,tcp_server_error); 	//初始化tcp_err()回调函数
		tcp_poll(newpcb,tcp_server_poll,1);	//初始化tcp_poll回调函数
		tcp_sent(newpcb,tcp_server_sent);  	//初始化发送回调函数
		  
		tcp_server_flag|=1<<5;				//标记有客户端连上了
		lwipdev.remoteip[0]=newpcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(newpcb->remote_ip.addr>>8)&0xff;  	//IADDR3
		lwipdev.remoteip[2]=(newpcb->remote_ip.addr>>16)&0xff; 	//IADDR2
		lwipdev.remoteip[3]=(newpcb->remote_ip.addr>>24)&0xff; 	//IADDR1 
		ret_err=ERR_OK;
	}else ret_err=ERR_MEM;
	return ret_err;
}

//lwIP tcp_sent的回调函数(当从远端客户端后接收到ACK信号后发送数据)
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_server_struct *es;
    LWIP_UNUSED_ARG(len);
    es=(struct tcp_server_struct*)arg;
    if(es->p)tcp_server_senddata(tpcb,es);//发送数据
    return ERR_OK;
}


//此函数用来发送数据
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct * es)
{
    struct pbuf *ptr;
	u16 plen;    
    err_t wr_err=ERR_OK;
    while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))){
        ptr=es->p;
        wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1); //将要发送的数据加入到发送缓冲队列中
        if(wr_err==ERR_OK) {
            plen=ptr->len;
            es->p=ptr->next;            //指向下一个pbuf
            if(es->p)pbuf_ref(es->p);    //pbuf的ref加一
            pbuf_free(ptr);                //释放ptr 
            tcp_recved(tpcb,plen); 		//更新tcp窗口大小
        }else if(wr_err==ERR_MEM) {
            es->p=ptr;
        }

        tcp_output(tpcb);                //将发送缓冲队列中的数据立即发送出去
    }     
} 
//关闭与服务器的连接
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct * es)
{
    //移除回调
    tcp_close(tpcb);;//终止连接,删除pcb控制块
    tcp_arg(tpcb,NULL);  
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);  

    if (es) {

        mem_free(es);
    }

    tcp_server_flag&=~(1<<5);//标记连接断开了
}


extern void tcp_pcb_purge(struct tcp_pcb *pcb);	//在 tcp.c里面 
extern struct tcp_pcb *tcp_active_pcbs;			//在 tcp.c里面 
extern struct tcp_pcb *tcp_tw_pcbs;				//在 tcp.c里面  
//强制删除TCP Server主动断开时的time wait
void tcp_server_remove_timewait(void)
{
	struct tcp_pcb *pcb,*pcb2; 
	u8 t=0;
	while(tcp_active_pcbs!=NULL&&t<200)
	{
		t++;
		delay_ms(10);			//等待tcp_active_pcbs为空  
	}
	pcb=tcp_tw_pcbs;
	while(pcb!=NULL)//如果有等待状态的pcbs
	{
		tcp_pcb_purge(pcb); 
		tcp_tw_pcbs=pcb->next;
		pcb2=pcb;
		pcb=pcb->next;
		memp_free(MEMP_TCP_PCB,pcb2);	
	}
}















