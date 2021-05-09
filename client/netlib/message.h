/*************************************************
  Copyright (C), 2009-, Fangmin Tech. Co., Ltd.
  File name:      // message.h
  Author:Qian Weizhong   Date: 2009.11
  Description:    // 消息类低层处理函数实现，包括消息发送、接收功能
  Others:         // 无
  Function List:  // 主要函数列表，每条记录应包括函数名及功能简要说明
    1. bool Cmessage::init_socket();//套接字初始化
	2. int Cmessage::send(char *pbuf,unsigned int len,MSGHEAD *phead,SOCKADDR *dst_addr,int M_TYPE);			//以SSL UDP加密方式传递buffer内容，返回成功或失败，实现消息窗口管理
	3. int Cmessage::recv(char *pdata,unsigned int size,MSGHEAD *phead,SOCKADDR *addrClient,int *len)
  History:        // 
*************************************************/
#ifndef MESSAGE_H
#define MESSAGE_H
#include "../stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <afxmt.h>
//#include "dialogimage.h"
#define SND_BUF_SIZE 100
#define RECV_BUF_SIZE 65535
#define SEND_BUF_SIZE 65535
#define CLIENT
#ifdef SERVER
#define LISTEN_PORT 7000
#else
#define LISTEN_PORT 7001
#endif

#define SERVER_IP	"127.0.0.1"
#define SERVER_PORT "7000"

#define SEND_ONCE_SIZE 60000
#define RCV_ONCE_SIZE 60000

#define SND_TIMEOUT   2000
#define RCV_TIMEOUT	  500

/*安全中心服务端口*/
//定义命令、响应
#define M_TYPE_CMD	1
#define M_TYPE_RES  0
/*消息类定义*/
#define MSG_MANAGE 0x0001
#define MSG_RESPONSE_SUCCESS 0x0002
#define MSG_RESPONSE_FAIL 0x0003
/*消息子类定义*/
#define MSG_LOGON 0x0001					//登录命令
#define MSG_QUESTION_SAVE 0x0002
#define MSG_TEST_UPDATE 0x0003				//考试结果上传
#define MSG_HISTORY_QUERY 0x0004			//历史查询
#define MSG_TEST_LOAD 0x0005
#define MSG_TEST_COMMIT 0x0006
#define MSG_KEY_NEGOTIATE 0x0007			//协商密钥

#define MAX_SLICEITEM_SLEEP		120			//重组分组等待最多数据报，如果等待了120个报文，还没有成功重组，则从链表中删除

//定义消息接收函数的处理结果
#define PROC_SUCCESS				1			//处理成功
#define PROC_PROCESSING				2			//正在线程处理
#define PROC_FAIL					0			//处理失败

#define ENCRYPT

#define LOGON_FAIL		0
#define LOGON_SUC		1
#define LOGON_TIMEOUT	2


typedef struct
{
	WORD  id;				//自增型消息ID
} MID; 

typedef struct {
	WORD m_no;					//消息分片编号，从0开始编号，用于消息分片的组装
	WORD m_b_nolast;			//片未完标志，除最后一个分片为0外，所有分片为1
} SLICE;

typedef struct
{
	MID m_id;					//消息ID
	WORD m_type;				//消息类型ID
	WORD m_subtype;				//消息子类型ID
	SLICE m_slice;				//消息分片
	DWORD m_datalen;			//数据长度
} MSGHEAD;					//消息头部

struct sliceitem
{
	int num_item_wait;//计数器，用于计算item在链表中最长停留等待数据报个数，最大值为MAX_SLICEITEM_SLEEP
	MSGHEAD msghead;//消息头部
	char *data;//消息数据指针
	struct sliceitem *next;
};

typedef struct sliceitem SLICEITEM;

typedef struct _ScreenShotCFG
{
	int resolution;
	int colordepth;
}ScreenShotCFG;

typedef struct _msgheadsaved
{
	CRITICAL_SECTION msgcts_section;//用于链表的互斥访问
	bool b_recvd;//发送命令是否得到响应,1,得到响应，0，未得到响应
	bool b_result;//false,失败,true,成功
	bool is_end;
	MSGHEAD msghead;//缓存发送消息头部
}MSGHEADSAVED;

class Cmessage {
	bool is_initsock;	//Check if sock initialized true initialized false not initialized
	bool is_valid;
	SOCKET sock;
	static bool is_initialized;
	WORD sndseqs[SND_BUF_SIZE];//缓存发送的100条消息
	int cursndseq;//记录当前发送序列号的指针
	SLICEITEM *precv;//接收队列
public:
	MID m_id;
	void StartService();
	void StopService();
	Cmessage();
	void setsndseq();//设置发送序列号
	void appendslice(SLICEITEM *pitem);//对接收到的分片加入接收队列
	int get_whole_data(MSGHEAD *phead,char **pdata,int *rtn_data_len);
	void init_id();
	void setnextid();
	bool check_valid();
	int (*pproc)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);								//定义消息处理函数的函数指针
	bool check_initsock(); //true initialized false otherwise
	bool init_socket();
	int send(char *pbuf,MSGHEAD *phead,SOCKADDR *dst_addr,int m_type);			//以SSL UDP加密方式传递buffer内容，返回成功或失败，实现消息窗口管理
	int recv(char **pdata,MSGHEAD *phead,SOCKADDR *addrClient);			//阻塞方式接收数据，超时时间2s，返回接收到的消息长度，为0标明接收失败，否则返回消息长度及指向消息头部的指针。及实现消息窗口管理
};

class Config : public Cmessage{
	void *m_pobj;//指向回调函数引用对象指针
	MSGHEADSAVED msg_saved;
friend DWORD WINAPI recvmsg(Config *pcfg);
public:
	Config();
	Config(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata));
	~Config();
	void SetEndflg(bool flg);
	bool GetEndflg();
	int (*pproc)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);								//定义消息处理函数的函数指针
	void setproc(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata));		//设置消息处理函数指针
	int sendcmd(MSGHEAD *phead,char *psnddata,SOCKADDR *dst_addr);
	bool recvcmd(SOCKADDR *dst_addr);
};
extern void del_buf(char **p);
extern Config g_cfg;;
extern DWORD WINAPI recvmsg(Config *pcfg);
extern int DoMsgSend_negotiate(int type,int subtype,char *pdata,int datalen);
extern int DoMsgSend(int type,int subtype,char *pdata,int datalen);
#endif