#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "./netlib/message.h"

extern int recv_proc(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);
extern DWORD WINAPI msgdispatcher();
struct answer{
	int no;
	int key;
};

typedef struct answer ANSWER;
//定义接收到的消息链表
typedef struct recvd_msg
{
	MID m_id;					//消息ID
	WORD m_type;				//消息类型ID
	WORD m_subtype;				//消息子类型ID
	DWORD m_datalen;			//数据长度
	SOCKADDR dst_addr;
	char *data;//消息数据指针
	struct recvd_msg *next;
} RECVD_MSG;

class S_MSGLST
{
	CRITICAL_SECTION msgdis_section;//用于链表的互斥访问
	bool    end_flg;		//消息线程终止标志
	struct recvd_msg *phead;//存放消息链表头指针
	struct recvd_msg *ptail;//存放消息链表尾指针
public:
	S_MSGLST();
	~S_MSGLST();

	void set_endflg(bool flg);
	bool get_endflg();
	bool copytolst(MSGHEAD *phead,char *pdata,void *paddr);
	RECVD_MSG* popmsgls();//弹出一条消息
};//接收到的消息管理结构
extern S_MSGLST g_msglst;//消息管理结构全局变量
#endif