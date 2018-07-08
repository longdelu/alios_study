#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"

#define TCP_SERVER_RX_BUFSIZE	2000		//����tcp server���������ݳ���
#define TCP_SERVER_PORT			8088	//����tcp server�Ķ˿�
 



//tcp����������״̬
enum tcp_server_states
{
	ES_TCPSERVER_NONE = 0,		//û������
	ES_TCPSERVER_ACCEPTED,		//�пͻ����������� 
	ES_TCPSERVER_CLOSING,		//�����ر�����
}; 
//LWIP�ص�����ʹ�õĽṹ��
struct tcp_server_struct
{
	u8 state;               //��ǰ����״
	struct tcp_pcb *pcb;    //ָ��ǰ��pcb
	struct pbuf *p;         //ָ�����/�����pbuf
};   

void tcp_server_set_remoteip(void);

void tcp_server_demo_init(void);
//lwIP tcp_accept()�Ļص�����
err_t tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err);
err_t tcp_server_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err);
err_t tcp_server_usersent(struct tcp_pcb *tpcb);
void tcp_server_error(void *arg,err_t err);
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct * es);
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct * es );
void tcp_server_remove_timewait(void);

//TCP�ͻ��˲���
void tcp_demo_init(void);

#endif

