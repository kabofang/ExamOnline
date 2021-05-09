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
//������յ�����Ϣ����
typedef struct recvd_msg
{
	MID m_id;					//��ϢID
	WORD m_type;				//��Ϣ����ID
	WORD m_subtype;				//��Ϣ������ID
	DWORD m_datalen;			//���ݳ���
	SOCKADDR dst_addr;
	char *data;//��Ϣ����ָ��
	struct recvd_msg *next;
} RECVD_MSG;

class S_MSGLST
{
	CRITICAL_SECTION msgdis_section;//��������Ļ������
	bool    end_flg;		//��Ϣ�߳���ֹ��־
	struct recvd_msg *phead;//�����Ϣ����ͷָ��
	struct recvd_msg *ptail;//�����Ϣ����βָ��
public:
	S_MSGLST();
	~S_MSGLST();

	void set_endflg(bool flg);
	bool get_endflg();
	bool copytolst(MSGHEAD *phead,char *pdata,void *paddr);
	RECVD_MSG* popmsgls();//����һ����Ϣ
};//���յ�����Ϣ����ṹ
extern S_MSGLST g_msglst;//��Ϣ����ṹȫ�ֱ���
#endif