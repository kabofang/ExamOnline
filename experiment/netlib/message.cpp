/************************************************************
  Copyright (C), 2009-, Fangmin Tech. Co., Ltd.
  FileName: message.cpp
  Author: Qian Weizhong       Version :  V1.0        Date:2009.11
  Description:     // 消息处理函数实现代码，包括发送和接收功能      
  Function List:   // 主要函数及其功能
    1. bool Cmessage::init_socket();//套接字初始化
	2. int Cmessage::send(char *pbuf,unsigned int len,M~SGHEAD *phead,SOCKADDR *dst_addr);//以SSL
					UDP加密方式传递buffer内容，返回成功或失败，实现消息窗口管理
	3. int Cmessage::recv(char *pdata,unsigned int size,MSGHEAD *phead,SOCKADDR *addrClient,int *len)
  History:         // 历史修改记录
      <author>  <time>   <version >   <desc>  
***********************************************************/
#include "../des.h"
#include "../dh.h"
#include "message.h"
#include <memory.h>
#include <Winbase.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

void del_buf(char **p)
{
	if (*p)
		delete *p;
	*p = NULL;
}

//初始化id
//初始化发送序列号数组sndseqs
//初始化发送序列号指针cursndseq
void Cmessage::init_id()
{
	memset(&m_id,0,sizeof(MID));
	memset(sndseqs,-1,sizeof(sndseqs));
	cursndseq = 0;
}

Cmessage::Cmessage()
{
	StartService();
}

void Cmessage::StartService()
{
	if (is_initialized)
		return;
	precv = NULL;
	is_initsock = false;
	init_id();
	
	time_t t;
	srand((unsigned) time(&t));
	m_id.id = WORD(rand() % 65536);

	if (is_initialized == false)
		is_valid = true;
	else
		is_valid = false;
	
	if (!init_socket())
		is_valid = false;
	is_initialized = true;
}

void Cmessage::StopService()
{
	if (!is_initialized)
		return;
	closesocket(sock);
	is_initialized = false;
}

bool Cmessage::is_initialized = false;

//检查在生成Cmessage对象时，是否系统已经生成了，避免生成多个Cmessage对象
bool Cmessage::check_valid()
{
	return is_valid;
}

bool Cmessage::init_socket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int recvbufsize=RECV_BUF_SIZE;
	int sndbufsize=SEND_BUF_SIZE;
	int optlen=sizeof(int);
	int tmout=SND_TIMEOUT;   //设置发送超时2秒
	int rcvtimeout=RCV_TIMEOUT;
	int err;
 
	if (!check_valid())
		return false;

	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		return false;
	}
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 )
	{
		WSACleanup( );
		return false; 
	}

	sock = socket(AF_INET,SOCK_DGRAM,0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(LISTEN_PORT);
	bind(sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));

	if (setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&recvbufsize,sizeof(recvbufsize))==SOCKET_ERROR)
	{
		return false;
	}
	if (setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&sndbufsize,sizeof(sndbufsize))==SOCKET_ERROR)
	{
		return false;	  
	}
	if(setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&tmout,sizeof(tmout))==SOCKET_ERROR)   
	{   
		return false;   
	}
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&rcvtimeout,sizeof(rcvtimeout))==SOCKET_ERROR)   
	{   
		return false;   
	}
	
	is_initsock = true;
	return true;
}

//发送成功后，设置发送序列号缓冲区sndseqs
void Cmessage::setsndseq()
{
	sndseqs[cursndseq] = m_id.id;
	cursndseq++;
	cursndseq = cursndseq % SND_BUF_SIZE; 
}
//设置下一序列号
void Cmessage::setnextid()
{
	
	m_id.id++;
	m_id.id = m_id.id % 65535;
}

Config::Config()
{
	InitializeCriticalSection(&msg_saved.msgcts_section);
	msg_saved.b_recvd = false;//发送命令是否得到响应,1,得到响应，0，未得到响应
	msg_saved.b_result = false;//false,失败,true,成功
	msg_saved.is_end = false;
	memset(&msg_saved.msghead,0,sizeof(MSGHEAD));
}

Config::~Config()
{
	DeleteCriticalSection(&msg_saved.msgcts_section);
}

void Config::SetEndflg(bool flg)
{
	EnterCriticalSection(&msg_saved.msgcts_section);//申请访问锁
	msg_saved.is_end = flg;//设置标志
	LeaveCriticalSection(&msg_saved.msgcts_section);//释放链表锁
}

bool Config::GetEndflg()
{
	bool rtn_flg;
	EnterCriticalSection(&msg_saved.msgcts_section);//申请访问锁
	rtn_flg = msg_saved.is_end;//获取终止标志
	LeaveCriticalSection(&msg_saved.msgcts_section);//释放链表锁

	return rtn_flg;
}

Config::Config(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata))
{
	InitializeCriticalSection(&msg_saved.msgcts_section);
	setproc(pobj,pproc_cmd);
}

void Config::setproc(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata))
{
	m_pobj = pobj;
	pproc = pproc_cmd;
}

//创建发送数据
//MSGHEAD *phead,消息头部指针
//WORD slice_no,分片编号
//WORD is_no_last，是否不是最后一分片
bool gen_snddata(char **ppbuf,char *pdata,MSGHEAD *phead,WORD slice_no,WORD is_no_last,WORD len)
{
	MSGHEAD head;
	char *ptmp = NULL;
	memset(&head,0,sizeof(MSGHEAD));
	memcpy(&head,phead,sizeof(MSGHEAD));
	head.m_slice.m_no = slice_no;
	head.m_slice.m_b_nolast = is_no_last;
	
	//if (len < sizeof(MSGHEAD))
	//	return false;
	*ppbuf = new char[len+sizeof(MSGHEAD)];
	if (ppbuf == NULL)
	{
		return false;
	}
	memset(*ppbuf,0,len+sizeof(MSGHEAD));
	head.m_datalen = len;
	memcpy(*ppbuf,&head,sizeof(MSGHEAD));
	
	if (len > 0)
	{
		ptmp = *ppbuf + sizeof(MSGHEAD);
		memcpy(ptmp,pdata,len);
	}

	return true;
}

 int Cmessage::send(char *pbuf,MSGHEAD *phead,SOCKADDR *dst_addr,int m_type)
{
	char *psendbuf = NULL;
	char *pdata =NULL;
	
	int sendlen = phead->m_datalen;//发送的字节数
	int result_len;

	if (!check_valid())
		return -1;

	if (phead == NULL)
	{
		return -1;
	}
	
	if (m_type == M_TYPE_CMD)
		phead->m_id.id = m_id.id;//设置数据报id
	
	pdata = pbuf;
	
	int sendlen_once = 0;
	int slice_no = 0;
	int is_no_last = 0;
	if (!sendlen)//数据区为0，只发送头部数据
	{
		sendlen_once = sendlen;
		if (gen_snddata(&psendbuf,pdata,phead,slice_no,is_no_last,sendlen_once) == false)
		{
			del_buf(&psendbuf);
			return -1;
		}
		result_len = sendto(sock,psendbuf,sendlen_once+sizeof(MSGHEAD),0,(SOCKADDR*)dst_addr,sizeof(SOCKADDR));
		if (result_len < 0)
		{
			del_buf(&psendbuf);
			return result_len;
		}
		sendlen -= sendlen_once;
		pdata += sendlen_once;
		slice_no++;
		del_buf(&psendbuf);
		return sendlen+sizeof(MSGHEAD);
	}
	while (sendlen > 0)
	{
		if (sendlen > SEND_ONCE_SIZE)
		{
			sendlen_once = SEND_ONCE_SIZE;
			is_no_last = 1;//分片
		}
		else
		{
			sendlen_once = sendlen;
			is_no_last = 0;//结尾分片
		}

		if (gen_snddata(&psendbuf,pdata,phead,slice_no,is_no_last,sendlen_once) == false)
		{
			del_buf(&psendbuf);
			return -1;
		}

		result_len = sendto(sock,psendbuf,sendlen_once+sizeof(MSGHEAD),0,(SOCKADDR*)dst_addr,sizeof(SOCKADDR));
		if (result_len < 0)
		{
			del_buf(&psendbuf);
			return result_len;
		}
		sendlen -= sendlen_once;
		pdata += sendlen_once;
		slice_no++;
		del_buf(&psendbuf);
	}

	if (m_type == M_TYPE_CMD)
	{
    	setsndseq();//设置发送序列号
		setnextid();//设置数据报id为下一id
	}
	return phead->m_datalen+sizeof(MSGHEAD);
}

//检查接收消息与发送消息头部是否匹配
bool checkmatched(MSGHEAD *prech,MSGHEAD *psndh)
{
	if ((prech->m_id.id != psndh->m_id.id) || (prech->m_subtype != psndh->m_subtype))
		return false;
	return true;
}
//对链表进行维护，如果链表项等待数据报个数超过MAX_SLICEITEM_SLEEP,则从链表中删除
void do_maintainess_slices(SLICEITEM **phead)
{
	SLICEITEM *pprev = *phead;
	SLICEITEM *pitem = *phead;
	SLICEITEM *ptmp = NULL;
	while (pitem != NULL)
	{
		if (pitem->num_item_wait == 0)//等待时间到
		{
            if (pitem == *phead)
			{
				*phead = pitem->next;
				pprev = *phead;
				ptmp = pitem;
				pitem = *phead;
			}
			else
			{
				pprev->next = pitem->next;
				ptmp = pitem;
				pitem = pprev->next;
			}
			if (ptmp)
			{
				if (ptmp->data)
					delete ptmp->data;
				delete ptmp;
				ptmp = NULL;
			}
		}
		else
		{
			pprev = pitem;
			pitem = pitem->next;
		}
	}
}

//将接收到的分片插入链表
void Cmessage::appendslice(SLICEITEM *pitem)
{
	pitem->next = precv;
	precv = pitem;
}

//重组所有属于同一数据报分片
//MSGHEAD *phead,指向消息头部指针
//char **pdata,返回重组后的消息数据指针
//int *rtn_data_len,返回消息数据长度指针
//返回值,int
//1,获得了完整的数据
//2,获得了一个分片
//-1,出错
int Cmessage::get_whole_data(MSGHEAD *phead,char **pdata,int *rtn_data_len)
{
	int len = 0;
    SLICEITEM *ptmp = precv;
	char *ptr = NULL;
	*pdata = NULL;
	int total_get_slice = 0;//已获得分片数目
	//获得整个数据长度len
	int last_no = -1;
	
	while (ptmp)
	{
		if ((ptmp->msghead.m_id.id == phead->m_id.id) && (ptmp->msghead.m_type == phead->m_type) && (ptmp->msghead.m_subtype == phead->m_subtype))
		{
			total_get_slice++;
			len += ptmp->msghead.m_datalen;
			if (!ptmp->msghead.m_slice.m_b_nolast)//分片为最后1分片,记录最后1分片编号
				last_no = ptmp->msghead.m_slice.m_no;
		}
		ptmp = ptmp->next;
	}

	if (total_get_slice != last_no+1)//判断是否已搜集所有的分片，total_get_slice为已获得的分片数目，last_no为最后分片编号
		return 2;
	//获得了完整的数据
	*pdata = new char[len];
	if (*pdata == NULL)
		return -1;
	//组装完整的数据
	ptmp = precv;
	ptr = *pdata;
	*rtn_data_len = len;

	total_get_slice = 0;
	while (ptmp)
	{
		if ((ptmp->msghead.m_id.id == phead->m_id.id) && (ptmp->msghead.m_type == phead->m_type) && (ptmp->msghead.m_subtype == phead->m_subtype))
		{
			//分片编号，用于分片的顺序组装
			total_get_slice++;
			memcpy(ptr+ptmp->msghead.m_slice.m_no*SEND_ONCE_SIZE,ptmp->data,ptmp->msghead.m_datalen); //
			ptmp->num_item_wait = 0;
			if (total_get_slice == last_no+1)
				break;
		}
		ptmp = ptmp->next;
	}

	if (!ptmp)
		return -1;
	return 1;
}

//接收函数
//输入
//char **pdata,返回接收到的数据指针，如果没有接收到有效数据,返回的指针为NULL
//MSGHEAD *phead,返回接收到的消息头指针
//SOCKADDR *addrClient,返回接收到的源地址指针
//返回
//1 成功，且得到完整数据
//2 成功，得到分片数据
//-1 失败
int Cmessage::recv(char **pdata,MSGHEAD *phead,SOCKADDR *addrClient)
{
	int result_len;
	int len = sizeof(SOCKADDR);
	//int datalen;
	char buf[RECV_BUF_SIZE];
	char *ptmp;
	SLICEITEM *pslice = NULL;
	
	*pdata = NULL;
	
	if (!check_valid())
		return -1;

	if (phead == NULL)
	{
		return -1;
	}
	
	memset(phead,0,sizeof(MSGHEAD));
		
	result_len = recvfrom(sock,buf,RECV_BUF_SIZE,0,addrClient,&len);
	
	if (result_len==0)
	{
		return 0;
	}
	
	if (result_len < 0)
	{
		return -1;
	}

	if (result_len - sizeof(MSGHEAD)< 0)
	{
		return -1;
	}

	memcpy(phead,buf,sizeof(MSGHEAD));

	if ((phead->m_slice.m_no == 0) && (!phead->m_slice.m_b_nolast))//未分片
	{
		if (phead->m_datalen)
		{
			*pdata = new char[phead->m_datalen];
			if (*pdata == NULL)
				return false;
			ptmp = *pdata;
			int length = result_len-sizeof(MSGHEAD);
			memcpy(ptmp,(char *)(buf + sizeof(MSGHEAD)),length);//拷贝数据返回
		}
		return 1;
	}

	//if (phead->m_slice.m_b_nolast)//中间分片，加入链表
	//{
		SLICEITEM *pitem;
		pitem = new SLICEITEM;//生成新的链表节点
		if (!pitem)
			return -1;
		memset(pitem,0,sizeof(SLICEITEM));
		pitem->num_item_wait = MAX_SLICEITEM_SLEEP;
		memcpy(&pitem->msghead,phead,sizeof(MSGHEAD));
		if (phead->m_datalen > 0)
		{
			pitem->data = new char[phead->m_datalen];
			if (!pitem->data)
			{
				del_buf((char **)&pitem);
				return -1;
			}
			memcpy(pitem->data,buf+sizeof(MSGHEAD),phead->m_datalen);
		}
		appendslice(pitem);
	//}

	int rtn_flg;
	int total_size;
	//对分片进行重组
	rtn_flg = get_whole_data(phead,pdata,&total_size);//获取完整数据，可能由于乱序到达，中间还有分片未到达，这种情况下返回true，上层调用者判断pdata指针处理
	
	if (rtn_flg == 1)//获得了完整的数据
	{
		phead->m_datalen = total_size;
	}
	
	do_maintainess_slices(&precv);
	return rtn_flg;
}

/*************************************************
  Function:		  sendmsg
  Description:    // 实现命令消息数据的发送
  Calls:          // message类的发送和接收函数
  Called By:      // 各安全部件
  Table Accessed: // 
  Table Updated:  // 
  Input:          // MSGHEAD *phead，消息头部指针，
									指向待发送的消息头部
					 char *psnddata，消息数据段指针
					 Cmessage *pmsg，消息类对象指针，用于
									低层输入输出操作
					 SOCKADDR *dst_addr,指向发送目标地址

  Output:         // char *pdata,	在函数中动态分配，指向
									接收到得消息响应数据
									段内容，需要在函数外
									用delete方法释放pdata指针
  Return:         // bool,  true  成功
							false 失败
  Others:         //
*************************************************/
bool Config::sendcmd(MSGHEAD *phead,char *psnddata,SOCKADDR *dst_addr)
{
	int len = sizeof(SOCKADDR);
	bool rtnflg;
	//bool is_matched = false;
	//int max_try = 3;
	//MSGHEAD rcvh;
	//char *pdata = NULL;
	
	SOCKADDR_IN *paddr = (SOCKADDR_IN *)dst_addr;

	if ((phead == NULL) || (dst_addr == NULL))
	{
		if (psnddata)
		{
			delete psnddata;
			psnddata = NULL;
		}
		return false;
	}
	
	if (send(psnddata,phead,dst_addr,M_TYPE_CMD) < 0)//发送即时命令至受控端
			return false;
	else
		rtnflg = true;
	if (psnddata)
	{
		delete psnddata;
		psnddata = NULL;
	}
#ifdef SERVER
	return rtnflg;	
#endif
#ifndef SERVER
	EnterCriticalSection(&msg_saved.msgcts_section);
	memcpy(&msg_saved.msghead,phead,sizeof(MSGHEAD));//缓存发送的消息头部
	LeaveCriticalSection(&msg_saved.msgcts_section);

	int i = 0;
	bool b_result = false;
	bool b_recvd = false;
	EnterCriticalSection(&msg_saved.msgcts_section);
	msg_saved.b_recvd = false;
	msg_saved.b_result = false;
	b_recvd = msg_saved.b_recvd;
	LeaveCriticalSection(&msg_saved.msgcts_section);
	while ((i<5) && (!b_recvd))
	{
		EnterCriticalSection(&msg_saved.msgcts_section);
		if (msg_saved.b_recvd)//收到响应报文
		{
			b_recvd = msg_saved.b_recvd;
			b_result = msg_saved.b_result;
			msg_saved.b_recvd = false;//恢复为未收到状态
			msg_saved.b_result = false;//恢复为未接受状态
		}
		LeaveCriticalSection(&msg_saved.msgcts_section);
		Sleep(100);
		i++;
	}

	if (i==5)
	{
		EnterCriticalSection(&msg_saved.msgcts_section);
		msg_saved.b_recvd = false;//恢复为未接受状态
		msg_saved.b_result = false;//恢复为未接受状态
		LeaveCriticalSection(&msg_saved.msgcts_section);
	}
	
	return b_result;//2秒内没有收到响应，返回false，否则返回true
#endif
}

/*************************************************
  Function:		  recvres
  Description:    // 实现响应消息数据的接收，通常用在接收线程中，根据接收到的响应消息调用对应的回调函数
  Calls:          // message类的发送和接收函数,命令解析函数
  Called By:      // 管控端\安全中心
  Table Accessed: // 
  Table Updated:  // 
  Input:          // SOCKADDR *dst_addr,指向发送源地址

  Output:         // char *pdata,消息数据段指针
  Return:         // bool,  true  成功
							false 失败
  Others:         //
*************************************************/
/*bool Config::recvres(SOCKADDR *dst_addr)
{
	char *pdata = NULL;
	char *presdata = NULL;
	MSGHEAD head;
	bool res_flg = true;

	if (dst_addr == NULL)
		return false;
	if (recv(&pdata,&head,dst_addr) == false)//接收命令
	{
		del_buf(&pdata);
		return false;
	}
	memset(&res_head,0,sizeof(res_head));
	if (pproc)
	{
		res_flg = pproc(m_pobj,&head,pdata,&presdata);
	}

	del_buf(&pdata);
	del_buf(&presdata);

	return res_flg;	
}*/

/*************************************************
  Function:		  recvcmd
  Description:    // 实现命令接收,解析及响应消息数据的发送
  Calls:          // message类的发送和接收函数,命令解析函数
  Called By:      // 受控端\安全中心
  Table Accessed: // 
  Table Updated:  // 
  Input:          // SOCKADDR *dst_addr,指向发送源地址

  Output:         // char *pdata,消息数据段指针
  Return:         // bool,  true  成功
							false 失败
  Others:         //
*************************************************/
bool Config::recvcmd(SOCKADDR *dst_addr)
{
	char *pdata = NULL;
	char *presdata = NULL;
	MSGHEAD head;
	MSGHEAD res_head;
	int max_try = 3;
	int res_flg = -1;

	if (dst_addr == NULL)
		return false;
	int rtn_flg;
	rtn_flg = recv(&pdata,&head,dst_addr);

	if (rtn_flg == 0)
	{
		return true;//超时时间内未收到分片
	}

	if (rtn_flg == -1)//接收命令失败
	{
		del_buf(&pdata);
		return false;
	}

	if (rtn_flg == 2)//获得了1个分片
	{
		if (pdata)
		{
			delete pdata;
			pdata = NULL;
		}
		return true;
	}


	memset(&res_head,0,sizeof(res_head));

	m_pobj = (void *)dst_addr;//存储目标地址，以便在线程中发送到目标主机

	if (pproc)
	{
		res_flg = pproc(m_pobj,&head,pdata,&presdata);
	}
	
	if (res_flg !=PROC_PROCESSING)//消息处理完毕
	{
		if ((head.m_type != MSG_RESPONSE_SUCCESS) && (head.m_type != MSG_RESPONSE_FAIL))
		{

			memcpy(&res_head,&head,sizeof(MSGHEAD));
			if (res_flg)
				res_head.m_type = MSG_RESPONSE_SUCCESS;
			else
				res_head.m_type = MSG_RESPONSE_FAIL;
			//res_head.m_subtype = head.m_subtype;
			//res_head.m_id.id = head.m_id.id;//对接收到命令id的响应
			//res_head.m_datalen = head.m_datalen;
	
			while (max_try>0)
			{
				if (send(presdata,&res_head,dst_addr,M_TYPE_RES) < 0)//发送响应
				{
					max_try--;
					continue;
				}
				break;
			}

			if (max_try<=0)
				res_flg = false;
			else
				res_flg = true;
		}
#ifndef SECCENTER
		else//响应消息
		{
			EnterCriticalSection(&msg_saved.msgcts_section);
            if (checkmatched(&msg_saved.msghead,&head)==true)//检查发送的消息头部是否与响应头部匹配
			{
				msg_saved.b_recvd = true;//设置已收到响应标识
				
				if (head.m_type == MSG_RESPONSE_SUCCESS)
                    msg_saved.b_result = true;//接收成功
				else
					msg_saved.b_result = false;//接收失败
			}
			LeaveCriticalSection(&msg_saved.msgcts_section);
		}
#endif
	}

	del_buf(&pdata);
	del_buf(&presdata);

	return true;	
}

DWORD WINAPI recvmsg(Config *pcfg)
{
	SOCKADDR dst_addr;
	bool is_running = true;
	while (is_running)
	{
		EnterCriticalSection(&pcfg->msg_saved.msgcts_section);
		if (pcfg->msg_saved.is_end == true)//主线程关闭
		{
			pcfg->msg_saved.is_end = false;
			is_running = false;
		}
		LeaveCriticalSection(&pcfg->msg_saved.msgcts_section);
		if (is_running)
		{
			pcfg->recvcmd(&dst_addr);
		}
		Sleep(1);
	}
	return 0;
}