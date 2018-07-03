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
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F4&F7������
//TCP ���Դ���      
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/2/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2009-2019
//All rights reserved                             
//*******************************************************************************
//�޸���Ϣ
//��������Ϊtcp���������󶨵�һ����֪�Ķ˿ں�����
//////////////////////////////////////////////////////////////////////////////////          ��

/* TCP Client�������ݻ����� */
u8 tcp_server_recvbuf[TCP_CLIENT_RX_BUFSIZE];    
//TCP������������������

const u8 *tcp_server_sendbuf="TCP Client send data\r\n";

//TCP Client ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����
u8 tcp_server_flag;




//TCP����
void tcp_demo_init(void)
{
 
    struct tcp_pcb *tcppcb;      //����һ��TCP���������ƿ�
    struct ip_addr rmtipaddr;    //Զ��ip��ַ
    
    u8 *tbuf;
    u8 key;
    u8 res=0;        
    u8 t=0; 
    
    tcp_server_set_remoteip();//��ѡ��IP

    printf("TCP Client Test");
    printf("KEY0:Send data");  
    printf("KEY_UP:Quit");  
    printf("When break,please quit!");  
    tbuf=mymalloc(SRAMIN,200);    //�����ڴ�
    if(tbuf==NULL)return ;        //�ڴ�����ʧ����,ֱ���˳�
    sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
    printf("%s",tbuf);  
    sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
    printf("%s",tbuf);  
    sprintf((char*)tbuf,"Remotewo Port:%d",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
    printf("%s",tbuf);
    printf("STATUS:Disconnected"); 
    tcppcb=tcp_new();    //����һ���µ�pcb
    if(tcppcb) {         //�����ɹ�

        IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
         res = tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_server_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_server_connected()����

         if (res == ERR_OK) {
             printf("connected req sented\r\n");
         }

    }else {
        res=1;
    }
    while(res==0)
    {
        key=KEY_Scan(0);
        if(key==WKUP_PRES)break;
        if(key==KEY0_PRES)//KEY0������,��������
        {
            tcp_server_usersent(tcppcb);    //��������
        }
        if(tcp_server_flag&1<<6) {           //�Ƿ��յ�����?

            printf("%s",tcp_server_recvbuf); //��ʾ���յ�������

            tcp_server_flag&=~(1<<6);        //��������Ѿ���������.
        } if(tcp;._server_flag&1<<5)  {        //�Ƿ�������

            //printf(" STATUS:Connected\r\n   ");   //��ʾ��Ϣ
            //printf("Receive Data:\r\n");//��ʾ��Ϣ        

        }else if((tcp_server_flag&1<<5)==0)
        {
              printf("STATUS:Disconnected");

        } 

        delay_ms(2);
        t++;
        if(t==200)
        {
            if((tcp_server_flag&1<<5)==0)//δ������,��������
            { 
                tcp_server_connection_close(tcppcb,0);//�ر�����
                tcppcb=tcp_new();    //����һ���µ�pcb
                if(tcppcb)            //�����ɹ�
                { 
                    tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_server_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_server_connected()����
                }
            }
            t=0;
            LED0 = ~LED0;
        }        
    }
    tcp_server_connection_close(tcppcb,0);//�ر�TCP Client����

    printf("TCP Client Test"); 
    
    printf("Connect break��");  
    printf("KEY1:Connect");
    myfree(SRAMIN,tbuf);
} 


//����Զ��IP��ַ, �������ӵ���ȷ�ķ���������
void tcp_server_set_remoteip(void)
{
    
    uint8_t key = 0;
      
    //ǰ����IP���ֺ�DHCP�õ���IPһ��
    lwipdev.remoteip[0]=lwipdev.ip[0];
    lwipdev.remoteip[1]=lwipdev.ip[1];
    lwipdev.remoteip[2]=lwipdev.ip[2];
    
    /* ������ĸ�Ĭ�ϴ�1��ʼ���� */
    lwipdev.remoteip[3]=100;
    
    printf("Current Remote IP :%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);

    printf("please set tcp server ip right");

    while(1) {

        key=KEY_Scan(0);

        if(key==WKUP_PRES) {
            break;
        } else if(key) {
            
            if(key==KEY0_PRES)lwipdev.remoteip[3]++;//IP����
            if(key==KEY2_PRES)lwipdev.remoteip[3]--;//IP����
            
            printf("Remote IP :%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
        }
    }
    
}

/* lwIP tcp_recv()�����Ļص����� */
err_t tcp_server_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
    u32 data_len = 0;
    struct pbuf *q;
    struct tcp_server_struct *es;
    err_t ret_err; 
    LWIP_ASSERT("arg != NULL",arg != NULL);
    es=(struct tcp_server_struct *)arg; 
    if(p==NULL)//����ӷ��������յ��յ�����֡�͹ر�����
    {
        es->state=ES_TCPCLIENT_CLOSING;//��Ҫ�ر�TCP ������ 
         es->p=p; 
        ret_err=ERR_OK;
    }else if(err!= ERR_OK)//�����յ�һ���ǿյ�����֡,����err!=ERR_OK
    { 
        if(p)pbuf_free(p);//�ͷŽ���pbuf
        ret_err=err;
    }else if(es->state==ES_TCPCLIENT_CONNECTED)    //����������״̬ʱ
    {
        if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
        {
            memset(tcp_server_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
            for(q=p;q!=NULL;q=q->next)  //����������pbuf����
            {
                //�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
                //�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
                if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
                else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
                data_len += q->len;      
                if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����    
            }
            tcp_server_flag|=1<<6;        //��ǽ��յ�������
             tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
            pbuf_free(p);      //�ͷ��ڴ�
            ret_err=ERR_OK;
        }
    }else  //���յ����ݵ��������Ѿ��ر�,
    { 
        tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
        es->p=NULL;
        pbuf_free(p); //�ͷ��ڴ�
        ret_err=ERR_OK;
    }
    return ret_err;
}

//lwIP tcp_err�����Ļص�����
void tcp_server_error(void *arg,err_t err)
{  
    //�������ǲ����κδ���
} 

//LWIP���ݷ��ͣ��û�Ӧ�ó�����ô˺�������������
//tpcb:TCP���ƿ�
//����ֵ:0���ɹ���������ʧ��
err_t tcp_server_usersent(struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es; 
    es=tpcb->callback_arg;
    if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
    {
        es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_server_sendbuf),PBUF_POOL);    //�����ڴ� 
        pbuf_take(es->p,(char*)tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf));    //��tcp_server_sentbuf[]�е����ݿ�����es->p_tx��
        tcp_server_senddata(tpcb,es);//��tcp_server_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
        tcp_server_flag&=~(1<<7);    //������ݷ��ͱ�־
        if(es->p)pbuf_free(es->p);    //�ͷ��ڴ�
        ret_err=ERR_OK;
    }else
    { 
        tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
        ret_err=ERR_ABRT;
    }
    return ret_err;
}
//lwIP tcp_poll�Ļص�����
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es; 
    es=(struct tcp_server_struct*)arg;
    if(es->state==ES_TCPCLIENT_CLOSING)         //���ӶϿ�
    { 
         tcp_server_connection_close(tpcb,es);   //�ر�TCP����
    } 
    ret_err=ERR_OK;
    return ret_err;
} 
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_server_struct *es;
    LWIP_UNUSED_ARG(len);
    es=(struct tcp_server_struct*)arg;
    if(es->p)tcp_server_senddata(tpcb,es);//��������
    return ERR_OK;
}


/* lwIP TCP���ӽ�������ûص����� */
err_t tcp_server_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    struct tcp_server_struct *es=NULL;  
    if(err==ERR_OK)   
    {
        es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct));  //�����ڴ�
        if(es) //�ڴ�����ɹ�
        {
            es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
            es->pcb=tpcb;  
            es->p=NULL; 
            tcp_arg(tpcb,es);                    //ʹ��es����tpcb��callback_arg
            tcp_recv(tpcb,tcp_server_recv);      //��ʼ��LwIP��tcp_recv�ص�����   
            tcp_err(tpcb,tcp_server_error);     //��ʼ��tcp_err()�ص�����
            tcp_sent(tpcb,tcp_server_sent);        //��ʼ��LwIP��tcp_sent�ص�����
            tcp_poll(tpcb,tcp_server_poll,1);     //��ʼ��LwIP��tcp_poll�ص����� 
            tcp_server_flag|=1<<5;                 //������ӵ���������
            err=ERR_OK;
        }else
        { 
            tcp_server_connection_close(tpcb,es);//�ر�����
            err=ERR_MEM;    //�����ڴ�������
        }
    }else
    {
        tcp_server_connection_close(tpcb,0);//�ر�����
    }
    return err;
}


//�˺���������������
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct * es)
{
    struct pbuf *ptr; 
    err_t wr_err=ERR_OK;
    while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))){
        ptr=es->p;
        wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1); //��Ҫ���͵����ݼ��뵽���ͻ��������
        if(wr_err==ERR_OK) {
            es->p=ptr->next;            //ָ����һ��pbuf
            if(es->p)pbuf_ref(es->p);    //pbuf��ref��һ
            pbuf_free(ptr);                //�ͷ�ptr 
        }else if(wr_err==ERR_MEM) {
            es->p=ptr;
        }

        tcp_output(tpcb);                //�����ͻ�������е������������ͳ�ȥ
    }     
} 
//�ر��������������
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct * es)
{
    //�Ƴ��ص�
    tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
    tcp_arg(tpcb,NULL);  
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);  

    if (es) {

        mem_free(es);
    }

    tcp_server_flag&=~(1<<5);//������ӶϿ���
}












