#include "dh.h"
#include "msgdispatcher.h"
#include "CSecure.h"
#include "CDes.h"
#include "CRsa.h"
#include "tempvar.h"
#include "sha.h"

HWND CMSGDISPATCHER::h_dlgtest = NULL;
extern char sa[MAX],sb[MAX],p[MAX],g[MAX],au[MAX];
extern int a;

//������Ϣ�ַ�������ʵ�ֶ��ڽ��յ���������Ϣ�Ĵ���
int CMSGDISPATCHER::do_response(ULONG saddr,int subtype,char *pdata,int len)
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
#ifdef NEG_ENCRYPT
	case MSG_REQCERT:
		//char* pCert = new char[len];
		char* pPlain;
		NegKey->Decrypt(len, pdata, &pPlain);				//ʹ��CA��Կ��֤�����
		Cert = new CCert;
		memmove(Cert, pPlain, sizeof(CCert));
		if (strcmp(Cert->GetOwner(), "ExamOnline Server") || strcmp(Cert->GetLusser(), "ExamOnline CA") )
			return CERTFAIL;								//��֤֤�������е�owner�Ƿ�Ϊ����ϵͳ���������䷢���Ƿ����Զ����CA
		if (!strcmp(Cert->GetHash(), "DIY_SHA")) {			//�鿴��ϢժҪ�㷨��������ϢժҪ�㷨�Ĳ�ͬ�����в�ͬ�Ĳ���
			unsigned char pHashValue[64];					//����ֻ���Զ���sha1�㷨������ֱ�Ӽ���hashֵ
			memmove(pHashValue, Cert->GetHashValue(), 64);
			memset(Cert->GetHashValue(), 0, 64);
			SHA1_CONTEXT ctx;
			sha1_init(&ctx);
			sha1_write(&ctx, (unsigned char*)Cert, 3 * DATALEN + KEYLEN + HASHLEN + sizeof(int));
			sha1_final(&ctx);
			for (int i = 0; i < 64; i++)
				if (ctx.buf[i] != pHashValue[i])			//�Ѽ������hashֵ��ԭ֤���е�hashֵ�Աȣ�����֤֤���������
					return CERTFAIL;
			delete (CRsa*)NegKey;
			NegKey = new CRsa;								//֤��Ϸ�������������ȡ����Կ������Rsa����
			NegKey->Init(Cert->GetPubKey(), Cert->GetKeylen());
		}
		else
			return CERTFAIL;
		break;
#endif
	case MSG_KEY_NEGOTIATE:
#ifdef NEG_ENCRYPT
		int Keylen = NegKey->GetKeylen();
		char* pplain;
		NegKey->Decrypt(Keylen, pdata, &pplain);//RSA����Э����Ϣ
		delete NegKey;//Э�̽����ͷ���Դ
		memmove(sb, pplain,MAX);
		delete pplain;
#else
		memmove(sb, pdata, MAX);
#endif
		getkey(a,sb,p,g,au);

#ifdef MSG_ENCRYPT
		Key = new CDes;
		Key->Init(au, MAX);//��Э�̺õ�DES��Կ����DES����
#endif
		break;
	}
	return rtnflg;
}

//��Ϣ���մ���ַ���ں���
int msgdispatcher(void *pobj,MSGHEAD *phead,char *pdata,char **presdata)
{
	int rtnflg = true;
	if (phead == NULL)
		return false;
	
#ifdef MSG_ENCRYPT
	char* plainText;
	if (phead->m_subtype != MSG_KEY_NEGOTIATE&& phead->m_subtype != MSG_REQCERT && phead->m_datalen)
	{
		Key->Decrypt(phead->m_datalen, pdata, &plainText);//DES��������
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