#include "des.h"
#include "dh.h"
#include "msgdispatcher.h"
#include "RSAkeygen.h"

HWND CMSGDISPATCHER::h_dlgtest = NULL;
extern char sa[MAX],sb[MAX],p[MAX],g[MAX],au[MAX];
extern int a;
//定义消息分发函数，实现对于接收到的任意消息的处理
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
		str_score.Format("您的得分为%d",*(int *)pdata);
		AfxMessageBox(str_score);
		}
		break;
	//处理密钥协商返回的响应，同学们自行完成
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

//消息接收处理分发入口函数
int msgdispatcher(void *pobj,MSGHEAD *phead,char *pdata,char **presdata)
{
	bool rtnflg = true;
	if (phead == NULL)
		return false;
	
	//处理解密，同学们自行处理
	
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
	case MSG_RESPONSE_SUCCESS://接收到成功的响应
		rtnflg = CMSGDISPATCHER::do_response(paddr->sin_addr.S_un.S_addr,phead->m_subtype,pdata,phead->m_datalen);
		break;
	case MSG_RESPONSE_FAIL://接收到失败的响应
		break;
	}

	return rtnflg;
}