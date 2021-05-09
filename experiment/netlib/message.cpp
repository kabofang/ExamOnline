/************************************************************
  Copyright (C), 2009-, Fangmin Tech. Co., Ltd.
  FileName: message.cpp
  Author: Qian Weizhong       Version :  V1.0        Date:2009.11
  Description:     // ��Ϣ������ʵ�ִ��룬�������ͺͽ��չ���      
  Function List:   // ��Ҫ�������书��
    1. bool Cmessage::init_socket();//�׽��ֳ�ʼ��
	2. int Cmessage::send(char *pbuf,unsigned int len,M~SGHEAD *phead,SOCKADDR *dst_addr);//��SSL
					UDP���ܷ�ʽ����buffer���ݣ����سɹ���ʧ�ܣ�ʵ����Ϣ���ڹ���
	3. int Cmessage::recv(char *pdata,unsigned int size,MSGHEAD *phead,SOCKADDR *addrClient,int *len)
  History:         // ��ʷ�޸ļ�¼
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

//��ʼ��id
//��ʼ���������к�����sndseqs
//��ʼ���������к�ָ��cursndseq
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

//���������Cmessage����ʱ���Ƿ�ϵͳ�Ѿ������ˣ��������ɶ��Cmessage����
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
	int tmout=SND_TIMEOUT;   //���÷��ͳ�ʱ2��
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

//���ͳɹ������÷������кŻ�����sndseqs
void Cmessage::setsndseq()
{
	sndseqs[cursndseq] = m_id.id;
	cursndseq++;
	cursndseq = cursndseq % SND_BUF_SIZE; 
}
//������һ���к�
void Cmessage::setnextid()
{
	
	m_id.id++;
	m_id.id = m_id.id % 65535;
}

Config::Config()
{
	InitializeCriticalSection(&msg_saved.msgcts_section);
	msg_saved.b_recvd = false;//���������Ƿ�õ���Ӧ,1,�õ���Ӧ��0��δ�õ���Ӧ
	msg_saved.b_result = false;//false,ʧ��,true,�ɹ�
	msg_saved.is_end = false;
	memset(&msg_saved.msghead,0,sizeof(MSGHEAD));
}

Config::~Config()
{
	DeleteCriticalSection(&msg_saved.msgcts_section);
}

void Config::SetEndflg(bool flg)
{
	EnterCriticalSection(&msg_saved.msgcts_section);//���������
	msg_saved.is_end = flg;//���ñ�־
	LeaveCriticalSection(&msg_saved.msgcts_section);//�ͷ�������
}

bool Config::GetEndflg()
{
	bool rtn_flg;
	EnterCriticalSection(&msg_saved.msgcts_section);//���������
	rtn_flg = msg_saved.is_end;//��ȡ��ֹ��־
	LeaveCriticalSection(&msg_saved.msgcts_section);//�ͷ�������

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

//������������
//MSGHEAD *phead,��Ϣͷ��ָ��
//WORD slice_no,��Ƭ���
//WORD is_no_last���Ƿ������һ��Ƭ
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
	
	int sendlen = phead->m_datalen;//���͵��ֽ���
	int result_len;

	if (!check_valid())
		return -1;

	if (phead == NULL)
	{
		return -1;
	}
	
	if (m_type == M_TYPE_CMD)
		phead->m_id.id = m_id.id;//�������ݱ�id
	
	pdata = pbuf;
	
	int sendlen_once = 0;
	int slice_no = 0;
	int is_no_last = 0;
	if (!sendlen)//������Ϊ0��ֻ����ͷ������
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
			is_no_last = 1;//��Ƭ
		}
		else
		{
			sendlen_once = sendlen;
			is_no_last = 0;//��β��Ƭ
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
    	setsndseq();//���÷������к�
		setnextid();//�������ݱ�idΪ��һid
	}
	return phead->m_datalen+sizeof(MSGHEAD);
}

//��������Ϣ�뷢����Ϣͷ���Ƿ�ƥ��
bool checkmatched(MSGHEAD *prech,MSGHEAD *psndh)
{
	if ((prech->m_id.id != psndh->m_id.id) || (prech->m_subtype != psndh->m_subtype))
		return false;
	return true;
}
//���������ά�������������ȴ����ݱ���������MAX_SLICEITEM_SLEEP,���������ɾ��
void do_maintainess_slices(SLICEITEM **phead)
{
	SLICEITEM *pprev = *phead;
	SLICEITEM *pitem = *phead;
	SLICEITEM *ptmp = NULL;
	while (pitem != NULL)
	{
		if (pitem->num_item_wait == 0)//�ȴ�ʱ�䵽
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

//�����յ��ķ�Ƭ��������
void Cmessage::appendslice(SLICEITEM *pitem)
{
	pitem->next = precv;
	precv = pitem;
}

//������������ͬһ���ݱ���Ƭ
//MSGHEAD *phead,ָ����Ϣͷ��ָ��
//char **pdata,������������Ϣ����ָ��
//int *rtn_data_len,������Ϣ���ݳ���ָ��
//����ֵ,int
//1,���������������
//2,�����һ����Ƭ
//-1,����
int Cmessage::get_whole_data(MSGHEAD *phead,char **pdata,int *rtn_data_len)
{
	int len = 0;
    SLICEITEM *ptmp = precv;
	char *ptr = NULL;
	*pdata = NULL;
	int total_get_slice = 0;//�ѻ�÷�Ƭ��Ŀ
	//����������ݳ���len
	int last_no = -1;
	
	while (ptmp)
	{
		if ((ptmp->msghead.m_id.id == phead->m_id.id) && (ptmp->msghead.m_type == phead->m_type) && (ptmp->msghead.m_subtype == phead->m_subtype))
		{
			total_get_slice++;
			len += ptmp->msghead.m_datalen;
			if (!ptmp->msghead.m_slice.m_b_nolast)//��ƬΪ���1��Ƭ,��¼���1��Ƭ���
				last_no = ptmp->msghead.m_slice.m_no;
		}
		ptmp = ptmp->next;
	}

	if (total_get_slice != last_no+1)//�ж��Ƿ����Ѽ����еķ�Ƭ��total_get_sliceΪ�ѻ�õķ�Ƭ��Ŀ��last_noΪ����Ƭ���
		return 2;
	//���������������
	*pdata = new char[len];
	if (*pdata == NULL)
		return -1;
	//��װ����������
	ptmp = precv;
	ptr = *pdata;
	*rtn_data_len = len;

	total_get_slice = 0;
	while (ptmp)
	{
		if ((ptmp->msghead.m_id.id == phead->m_id.id) && (ptmp->msghead.m_type == phead->m_type) && (ptmp->msghead.m_subtype == phead->m_subtype))
		{
			//��Ƭ��ţ����ڷ�Ƭ��˳����װ
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

//���պ���
//����
//char **pdata,���ؽ��յ�������ָ�룬���û�н��յ���Ч����,���ص�ָ��ΪNULL
//MSGHEAD *phead,���ؽ��յ�����Ϣͷָ��
//SOCKADDR *addrClient,���ؽ��յ���Դ��ַָ��
//����
//1 �ɹ����ҵõ���������
//2 �ɹ����õ���Ƭ����
//-1 ʧ��
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

	if ((phead->m_slice.m_no == 0) && (!phead->m_slice.m_b_nolast))//δ��Ƭ
	{
		if (phead->m_datalen)
		{
			*pdata = new char[phead->m_datalen];
			if (*pdata == NULL)
				return false;
			ptmp = *pdata;
			int length = result_len-sizeof(MSGHEAD);
			memcpy(ptmp,(char *)(buf + sizeof(MSGHEAD)),length);//�������ݷ���
		}
		return 1;
	}

	//if (phead->m_slice.m_b_nolast)//�м��Ƭ����������
	//{
		SLICEITEM *pitem;
		pitem = new SLICEITEM;//�����µ�����ڵ�
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
	//�Է�Ƭ��������
	rtn_flg = get_whole_data(phead,pdata,&total_size);//��ȡ�������ݣ������������򵽴�м仹�з�Ƭδ�����������·���true���ϲ�������ж�pdataָ�봦��
	
	if (rtn_flg == 1)//���������������
	{
		phead->m_datalen = total_size;
	}
	
	do_maintainess_slices(&precv);
	return rtn_flg;
}

/*************************************************
  Function:		  sendmsg
  Description:    // ʵ��������Ϣ���ݵķ���
  Calls:          // message��ķ��ͺͽ��պ���
  Called By:      // ����ȫ����
  Table Accessed: // 
  Table Updated:  // 
  Input:          // MSGHEAD *phead����Ϣͷ��ָ�룬
									ָ������͵���Ϣͷ��
					 char *psnddata����Ϣ���ݶ�ָ��
					 Cmessage *pmsg����Ϣ�����ָ�룬����
									�Ͳ������������
					 SOCKADDR *dst_addr,ָ����Ŀ���ַ

  Output:         // char *pdata,	�ں����ж�̬���䣬ָ��
									���յ�����Ϣ��Ӧ����
									�����ݣ���Ҫ�ں�����
									��delete�����ͷ�pdataָ��
  Return:         // bool,  true  �ɹ�
							false ʧ��
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
	
	if (send(psnddata,phead,dst_addr,M_TYPE_CMD) < 0)//���ͼ�ʱ�������ܿض�
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
	memcpy(&msg_saved.msghead,phead,sizeof(MSGHEAD));//���淢�͵���Ϣͷ��
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
		if (msg_saved.b_recvd)//�յ���Ӧ����
		{
			b_recvd = msg_saved.b_recvd;
			b_result = msg_saved.b_result;
			msg_saved.b_recvd = false;//�ָ�Ϊδ�յ�״̬
			msg_saved.b_result = false;//�ָ�Ϊδ����״̬
		}
		LeaveCriticalSection(&msg_saved.msgcts_section);
		Sleep(100);
		i++;
	}

	if (i==5)
	{
		EnterCriticalSection(&msg_saved.msgcts_section);
		msg_saved.b_recvd = false;//�ָ�Ϊδ����״̬
		msg_saved.b_result = false;//�ָ�Ϊδ����״̬
		LeaveCriticalSection(&msg_saved.msgcts_section);
	}
	
	return b_result;//2����û���յ���Ӧ������false�����򷵻�true
#endif
}

/*************************************************
  Function:		  recvres
  Description:    // ʵ����Ӧ��Ϣ���ݵĽ��գ�ͨ�����ڽ����߳��У����ݽ��յ�����Ӧ��Ϣ���ö�Ӧ�Ļص�����
  Calls:          // message��ķ��ͺͽ��պ���,�����������
  Called By:      // �ܿض�\��ȫ����
  Table Accessed: // 
  Table Updated:  // 
  Input:          // SOCKADDR *dst_addr,ָ����Դ��ַ

  Output:         // char *pdata,��Ϣ���ݶ�ָ��
  Return:         // bool,  true  �ɹ�
							false ʧ��
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
	if (recv(&pdata,&head,dst_addr) == false)//��������
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
  Description:    // ʵ���������,��������Ӧ��Ϣ���ݵķ���
  Calls:          // message��ķ��ͺͽ��պ���,�����������
  Called By:      // �ܿض�\��ȫ����
  Table Accessed: // 
  Table Updated:  // 
  Input:          // SOCKADDR *dst_addr,ָ����Դ��ַ

  Output:         // char *pdata,��Ϣ���ݶ�ָ��
  Return:         // bool,  true  �ɹ�
							false ʧ��
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
		return true;//��ʱʱ����δ�յ���Ƭ
	}

	if (rtn_flg == -1)//��������ʧ��
	{
		del_buf(&pdata);
		return false;
	}

	if (rtn_flg == 2)//�����1����Ƭ
	{
		if (pdata)
		{
			delete pdata;
			pdata = NULL;
		}
		return true;
	}


	memset(&res_head,0,sizeof(res_head));

	m_pobj = (void *)dst_addr;//�洢Ŀ���ַ���Ա����߳��з��͵�Ŀ������

	if (pproc)
	{
		res_flg = pproc(m_pobj,&head,pdata,&presdata);
	}
	
	if (res_flg !=PROC_PROCESSING)//��Ϣ�������
	{
		if ((head.m_type != MSG_RESPONSE_SUCCESS) && (head.m_type != MSG_RESPONSE_FAIL))
		{

			memcpy(&res_head,&head,sizeof(MSGHEAD));
			if (res_flg)
				res_head.m_type = MSG_RESPONSE_SUCCESS;
			else
				res_head.m_type = MSG_RESPONSE_FAIL;
			//res_head.m_subtype = head.m_subtype;
			//res_head.m_id.id = head.m_id.id;//�Խ��յ�����id����Ӧ
			//res_head.m_datalen = head.m_datalen;
	
			while (max_try>0)
			{
				if (send(presdata,&res_head,dst_addr,M_TYPE_RES) < 0)//������Ӧ
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
		else//��Ӧ��Ϣ
		{
			EnterCriticalSection(&msg_saved.msgcts_section);
            if (checkmatched(&msg_saved.msghead,&head)==true)//��鷢�͵���Ϣͷ���Ƿ�����Ӧͷ��ƥ��
			{
				msg_saved.b_recvd = true;//�������յ���Ӧ��ʶ
				
				if (head.m_type == MSG_RESPONSE_SUCCESS)
                    msg_saved.b_result = true;//���ճɹ�
				else
					msg_saved.b_result = false;//����ʧ��
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
		if (pcfg->msg_saved.is_end == true)//���̹߳ر�
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