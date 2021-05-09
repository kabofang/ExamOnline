#include "des.h"
#include "dh.h"
#include "msgdispatcher.h"
#include "RSAkeygen.h"

HWND CMSGDISPATCHER::h_dlgtest = NULL;
extern char sa[MAX],sb[MAX],p[MAX],g[MAX],au[MAX];
extern int a;
//������Ϣ�ַ�������ʵ�ֶ��ڽ��յ���������Ϣ�Ĵ���
bool CMSGDISPATCHER::do_response(ULONG saddr,int subtype,char *pdata,int len)
{
	bool rtnflg = false;
	char *pnew = NULL;
	if (len < 1)
		return true;
	
	switch(subtype)
	{
	case MSG_TEST_LOAD:
		{
			pnew = new char[len];
			memset(pnew,0,len);
			strcpy(pnew,pdata);
			PostMessage(CMSGDISPATCHER::h_dlgtest,WM_LOADTEST,(WPARAM)pnew,NULL);
		}
		break;
	case MSG_TEST_COMMIT:
		{
		CString str_score;
		str_score.Format("���ĵ÷�Ϊ%d",*(int *)pdata);
		AfxMessageBox(str_score);
		}
		break;
	//������ԿЭ�̷��ص���Ӧ��ͬѧ���������
	case MSG_KEY_NEGOTIATE:
		RSA* p_Key = GetkeygenRSA();
		int Keylen = RSA_size(p_Key);
		if (Keylen < MAX)
			return false;
		char* pdatatemp = new char[Keylen];
		RSA_Decrypt(Keylen, pdata, pdatatemp, p_Key);
		memmove(sb, pdatatemp,MAX);
		getkey(a,sb,p,g,au);
		break;
	}
	return rtnflg;
}

//��Ϣ���մ���ַ���ں���
int msgdispatcher(void *pobj,MSGHEAD *phead,char *pdata,char **presdata)
{
	bool rtnflg = true;
	if (phead == NULL)
		return false;
	
	//������ܣ�ͬѧ�����д���
	
#ifdef ENCRYPT
	char* plainText;
	if (phead->m_subtype != MSG_KEY_NEGOTIATE && phead->m_datalen)
	{
		int dlen = phead->m_datalen;
		DES_Decrypt(pdata,dlen ,au,&plainText);
		phead->m_datalen = dlen;
		memset(pdata, 0, phead->m_datalen);
		memcpy(pdata, plainText, phead->m_datalen);
		delete plainText;
	}
#endif

	SOCKADDR_IN *paddr = (SOCKADDR_IN *)pobj;
	switch(phead->m_type)
	{
	case MSG_RESPONSE_SUCCESS://���յ��ɹ�����Ӧ
		rtnflg = CMSGDISPATCHER::do_response(paddr->sin_addr.S_un.S_addr,phead->m_subtype,pdata,phead->m_datalen);
		break;
	case MSG_RESPONSE_FAIL://���յ�ʧ�ܵ���Ӧ
		break;
	}

	return rtnflg;
}