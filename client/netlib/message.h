/*************************************************
  Copyright (C), 2009-, Fangmin Tech. Co., Ltd.
  File name:      // message.h
  Author:Qian Weizhong   Date: 2009.11
  Description:    // ��Ϣ��Ͳ㴦����ʵ�֣�������Ϣ���͡����չ���
  Others:         // ��
  Function List:  // ��Ҫ�����б�ÿ����¼Ӧ���������������ܼ�Ҫ˵��
    1. bool Cmessage::init_socket();//�׽��ֳ�ʼ��
	2. int Cmessage::send(char *pbuf,unsigned int len,MSGHEAD *phead,SOCKADDR *dst_addr,int M_TYPE);			//��SSL UDP���ܷ�ʽ����buffer���ݣ����سɹ���ʧ�ܣ�ʵ����Ϣ���ڹ���
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

/*��ȫ���ķ���˿�*/
//���������Ӧ
#define M_TYPE_CMD	1
#define M_TYPE_RES  0
/*��Ϣ�ඨ��*/
#define MSG_MANAGE 0x0001
#define MSG_RESPONSE_SUCCESS 0x0002
#define MSG_RESPONSE_FAIL 0x0003
/*��Ϣ���ඨ��*/
#define MSG_LOGON 0x0001					//��¼����
#define MSG_QUESTION_SAVE 0x0002
#define MSG_TEST_UPDATE 0x0003				//���Խ���ϴ�
#define MSG_HISTORY_QUERY 0x0004			//��ʷ��ѯ
#define MSG_TEST_LOAD 0x0005
#define MSG_TEST_COMMIT 0x0006
#define MSG_KEY_NEGOTIATE 0x0007			//Э����Կ

#define MAX_SLICEITEM_SLEEP		120			//�������ȴ�������ݱ�������ȴ���120�����ģ���û�гɹ����飬���������ɾ��

//������Ϣ���պ����Ĵ�����
#define PROC_SUCCESS				1			//����ɹ�
#define PROC_PROCESSING				2			//�����̴߳���
#define PROC_FAIL					0			//����ʧ��

#define ENCRYPT

#define LOGON_FAIL		0
#define LOGON_SUC		1
#define LOGON_TIMEOUT	2


typedef struct
{
	WORD  id;				//��������ϢID
} MID; 

typedef struct {
	WORD m_no;					//��Ϣ��Ƭ��ţ���0��ʼ��ţ�������Ϣ��Ƭ����װ
	WORD m_b_nolast;			//Ƭδ���־�������һ����ƬΪ0�⣬���з�ƬΪ1
} SLICE;

typedef struct
{
	MID m_id;					//��ϢID
	WORD m_type;				//��Ϣ����ID
	WORD m_subtype;				//��Ϣ������ID
	SLICE m_slice;				//��Ϣ��Ƭ
	DWORD m_datalen;			//���ݳ���
} MSGHEAD;					//��Ϣͷ��

struct sliceitem
{
	int num_item_wait;//�����������ڼ���item���������ͣ���ȴ����ݱ����������ֵΪMAX_SLICEITEM_SLEEP
	MSGHEAD msghead;//��Ϣͷ��
	char *data;//��Ϣ����ָ��
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
	CRITICAL_SECTION msgcts_section;//��������Ļ������
	bool b_recvd;//���������Ƿ�õ���Ӧ,1,�õ���Ӧ��0��δ�õ���Ӧ
	bool b_result;//false,ʧ��,true,�ɹ�
	bool is_end;
	MSGHEAD msghead;//���淢����Ϣͷ��
}MSGHEADSAVED;

class Cmessage {
	bool is_initsock;	//Check if sock initialized true initialized false not initialized
	bool is_valid;
	SOCKET sock;
	static bool is_initialized;
	WORD sndseqs[SND_BUF_SIZE];//���淢�͵�100����Ϣ
	int cursndseq;//��¼��ǰ�������кŵ�ָ��
	SLICEITEM *precv;//���ն���
public:
	MID m_id;
	void StartService();
	void StopService();
	Cmessage();
	void setsndseq();//���÷������к�
	void appendslice(SLICEITEM *pitem);//�Խ��յ��ķ�Ƭ������ն���
	int get_whole_data(MSGHEAD *phead,char **pdata,int *rtn_data_len);
	void init_id();
	void setnextid();
	bool check_valid();
	int (*pproc)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);								//������Ϣ�������ĺ���ָ��
	bool check_initsock(); //true initialized false otherwise
	bool init_socket();
	int send(char *pbuf,MSGHEAD *phead,SOCKADDR *dst_addr,int m_type);			//��SSL UDP���ܷ�ʽ����buffer���ݣ����سɹ���ʧ�ܣ�ʵ����Ϣ���ڹ���
	int recv(char **pdata,MSGHEAD *phead,SOCKADDR *addrClient);			//������ʽ�������ݣ���ʱʱ��2s�����ؽ��յ�����Ϣ���ȣ�Ϊ0��������ʧ�ܣ����򷵻���Ϣ���ȼ�ָ����Ϣͷ����ָ�롣��ʵ����Ϣ���ڹ���
};

class Config : public Cmessage{
	void *m_pobj;//ָ��ص��������ö���ָ��
	MSGHEADSAVED msg_saved;
friend DWORD WINAPI recvmsg(Config *pcfg);
public:
	Config();
	Config(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata));
	~Config();
	void SetEndflg(bool flg);
	bool GetEndflg();
	int (*pproc)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);								//������Ϣ�������ĺ���ָ��
	void setproc(void *pobj,int (*pproc_cmd)(void *pobj,MSGHEAD *phead,char *pdata,char **presdata));		//������Ϣ������ָ��
	int sendcmd(MSGHEAD *phead,char *psnddata,SOCKADDR *dst_addr);
	bool recvcmd(SOCKADDR *dst_addr);
};
extern void del_buf(char **p);
extern Config g_cfg;;
extern DWORD WINAPI recvmsg(Config *pcfg);
extern int DoMsgSend_negotiate(int type,int subtype,char *pdata,int datalen);
extern int DoMsgSend(int type,int subtype,char *pdata,int datalen);
#endif