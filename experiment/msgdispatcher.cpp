#include "dh.h"
#include "msgdispatcher.h"
#include "database/database.h"
#include "xml/xmlnodewrapper.h"

#include "CSecure.h"
#include "CRsa.h"
#include "CDes.h"
#include "tempvar.h"
#include "CCert.h"
#define KEY_FILE "prikey.pem"


extern DWORD WINAPI msgdispatcher();

//�����յ�����Ϣ������Ϣ�������̴߳���
S_MSGLST::S_MSGLST()
{
	end_flg = 0;		//��Ϣ�߳���ֹ��־Ϊ0��ʾ����ֹ
	phead = NULL;//�����Ϣ����ͷָ��
	ptail = NULL;//�����Ϣ����βָ��
	InitializeCriticalSection(&msgdis_section);
}

/*��������*/
S_MSGLST::~S_MSGLST()
{
	struct recvd_msg* ptmp = phead;
	while (phead)//�ͷŷ����������Ϣ����
	{
		phead = phead->next;
		if (ptmp->data)
			delete ptmp->data;
		delete(ptmp);
		ptmp = phead;
	}
	DeleteCriticalSection(&msgdis_section);
}

void S_MSGLST::set_endflg(bool flg)
{
	EnterCriticalSection(&msgdis_section);//���������
	end_flg = flg;//���ñ�־
	LeaveCriticalSection(&msgdis_section);//�ͷ�������
}

bool S_MSGLST::get_endflg()
{
	bool rtn_flg;
	EnterCriticalSection(&msgdis_section);//���������
	rtn_flg = end_flg;//��ȡ��ֹ��־
	LeaveCriticalSection(&msgdis_section);//�ͷ�������

	return rtn_flg;
}

bool S_MSGLST::copytolst(MSGHEAD* pmsghead, char* pdata, void* paddr)
{
	RECVD_MSG* pmsg = NULL;

	if (pmsghead == NULL)
		return false;
	pmsg = new (RECVD_MSG);
	if (pmsg == NULL)
	{
		return false;
	}
	memset(pmsg, 0, sizeof(RECVD_MSG));
	memcpy(&pmsg->m_id, &pmsghead->m_id, sizeof(pmsghead->m_id));
	memcpy(&pmsg->dst_addr, paddr, sizeof(SOCKADDR));
	pmsg->m_type = pmsghead->m_type;
	pmsg->m_subtype = pmsghead->m_subtype;
	pmsg->m_datalen = pmsghead->m_datalen;

	if (pmsghead->m_datalen && pdata)//��������
	{
		pmsg->data = new char[pmsghead->m_datalen];
		if (!pmsg->data)
		{
			if (pmsg)
				delete(pmsg);
			return false;
		}
		memcpy(pmsg->data, pdata, pmsghead->m_datalen);
	}

	EnterCriticalSection(&msgdis_section);//���������

	if (!phead)//����Ϊ��
	{
		pmsg->next = phead;
		phead = pmsg;
		ptail = phead;
	}
	else//��������һ���ڵ�
	{
		pmsg->next = ptail->next;
		ptail = pmsg;
	}

	LeaveCriticalSection(&msgdis_section);//�ͷ�������

	return true;
}

RECVD_MSG* S_MSGLST::popmsgls()//����һ����Ϣ
{
	RECVD_MSG* pmsg = NULL;
	EnterCriticalSection(&msgdis_section);//����������
	if (phead)
	{
		pmsg = phead;
		phead = pmsg->next;
	}
	LeaveCriticalSection(&msgdis_section);//�ͷ�������
	return pmsg;
}

//������recv_proc(),���մ���������Ϣ�ַ�����
//pobj�����ӦĿ���ַ
int recv_proc(void* pobj, MSGHEAD* phead, char* pdata, char** presdata)
{
	if (phead == NULL)
		return PROC_FAIL;

	//���յ���Ӧ��Ϣ��ֱ�ӷ���
	if ((phead->m_type == MSG_RESPONSE_SUCCESS) || (phead->m_type == MSG_RESPONSE_FAIL))
		return PROC_SUCCESS;

	if (!g_msglst.copytolst(phead, pdata, pobj))//�����յ�����Ϣ������Ϣ�������̴߳���
		return PROC_FAIL;
	return PROC_PROCESSING;//�ύ��msgdispatcher�̴߳���
}

bool score_test(char* pdata, int datalen, char** presdata, int& reslen, CDATABASE& db)
{
	bool rtn_flg = false;
	int count = datalen / sizeof(ANSWER);
	ANSWER* pitem = (ANSWER*)pdata;
	CString strsql;
	std::string strkey;
	CString str_key;
	int key;
	int truecount = 0;
	int result;

	for (int i = 0; i < count; i++)
	{
		strsql.Format("select answerkey from QUESTIONS where QUESTION_ID = %d", pitem[i].no);
		//
		CSQL sql;
		IBPP::Statement st;
		sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//����SQL���
		if (!db.isconnected())
			return false;
		IBPP::Transaction tr = IBPP::TransactionFactory(db.get_db());
		try {
			tr->Start();
			rtn_flg = db.ExecSqlTransaction(sql, st, tr);
			if (rtn_flg == false)
			{
				tr->Rollback();
				return false;
			}
			rtn_flg = true;
			if (st->Fetch())
			{
				st->Get("answerkey", strkey);
				str_key = strkey.c_str();
				str_key.MakeUpper();
				if (str_key == "A")
					key = 1;
				if (str_key == "B")
					key = 2;
				if (str_key == "C")
					key = 3;
				if (str_key == "D")
					key = 4;

				if (key == pitem[i].key)
					truecount++;
			}
			tr->Commit();
		}
		catch (IBPP::SQLException& e) {
			tr->Rollback();
			rtn_flg = false;
			return rtn_flg;
		}
	}

	result = 0;
	result = (double)((double)truecount / (double)count) * 100;

	reslen = sizeof(int);
	*presdata = (char*)new int;
	memset(*presdata, 0, sizeof(int));
	*(int*)*presdata = result;
	rtn_flg = true;



	return rtn_flg;
}

bool load_test(char** presdata, int& reslen, CDATABASE& db)
{
	bool rtn_flg = false;
	int question_id;
	std::string str_body, str_optiona, str_optionb, str_optionc, str_optiond, str_answerkey;
	CString strsql = "";
	//
	CXmlDocumentWrapper xmlDoc;//���XML��װ�����
	xmlDoc.LoadXML("<QUESTIONS></QUESTIONS>");
	CXmlNodeWrapper rootnode(xmlDoc.AsNode());
	//
	strsql.Format("select * from QUESTIONS");

	CSQL sql;
	IBPP::Statement st;
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//����SQL���
	if (!db.isconnected())
		return false;
	IBPP::Transaction tr = IBPP::TransactionFactory(db.get_db());
	tr->Start();

	try {
		rtn_flg = db.ExecSqlTransaction(sql, st, tr);
		if (rtn_flg == false)
		{
			tr->Rollback();
			return false;
		}
		rtn_flg = true;
		while (st->Fetch())//�Խ�ɫ���ݼ����б���
		{
			st->Get("QUESTION_ID", question_id);
			st->Get("body", str_body);
			st->Get("optiona", str_optiona);
			st->Get("optionb", str_optionb);
			st->Get("optionc", str_optionc);
			st->Get("optiond", str_optiond);
			st->Get("answerkey", str_answerkey);
			//
			CXmlNodeWrapper node(rootnode.InsertNode(-1, "QUESTION"));
			node.SetValue("id", question_id);
			CXmlNodeWrapper bodynode(node.InsertNode(-1, "BODY"));
			bodynode.SetText(str_body.c_str());
			CXmlNodeWrapper node_a(node.InsertNode(-1, "OPTIONA"));
			node_a.SetText(str_optiona.c_str());
			CXmlNodeWrapper node_b(node.InsertNode(-1, "OPTIONB"));
			node_b.SetText(str_optionb.c_str());
			CXmlNodeWrapper node_c(node.InsertNode(-1, "OPTIONC"));
			node_c.SetText(str_optionc.c_str());
			CXmlNodeWrapper node_d(node.InsertNode(-1, "OPTIOND"));
			node_d.SetText(str_optiond.c_str());
		}
		tr->Commit();
	}
	catch (IBPP::SQLException& e) {
		tr->Rollback();
		rtn_flg = false;
	}
	CString strxml = xmlDoc.GetXML();
	reslen = strxml.GetLength() + 1;
	*presdata = new char[strxml.GetLength() + 1];
	memset(*presdata, 0, strxml.GetLength() + 1);
	memcpy(*presdata, (char*)(LPCTSTR)strxml, strxml.GetLength());

	return rtn_flg;
}

/*�û���¼�������˵�ʵ�ֺ���*/
bool check_logon(char* pdata, CDATABASE& db)
{
	bool rtnflg = false;
	bool rtn_flg = false;
	std::string strpasswd = "";
	CString strsql = "";
	char chusername[20];
	char chpasswd[20];
	if (!pdata)
		return false;
	memcpy(chusername, pdata, 20);
	memcpy(chpasswd, pdata + 20, 20);

	strsql.Format("select * from USERS where username='%s'", chusername);

	CSQL sql;
	IBPP::Statement st;
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//����SQL���
	if (!db.isconnected())
		return false;
	IBPP::Transaction tr = IBPP::TransactionFactory(db.get_db());
	tr->Start();

	try {
		rtn_flg = db.ExecSqlTransaction(sql, st, tr);
		if (rtn_flg == false)
		{
			tr->Rollback();
			return false;
		}
		rtn_flg = false;
		while (st->Fetch())//�Խ�ɫ���ݼ����б���
		{
			st->Get("PASSWD", strpasswd);
			if (!strcmp(chpasswd, strpasswd.c_str()))
				rtn_flg = true;
		}
		tr->Commit();
	}
	catch (IBPP::SQLException& e) {
		tr->Rollback();
		rtn_flg = false;
	}

	return rtn_flg;
}

bool save_question(char* pxml, CDATABASE& db)
{
	bool rtnflg = false;
	CXmlDocumentWrapper xmlDoc;
	if (xmlDoc.LoadXML(pxml) == false)
	{
		return false;
	}

	CXmlNodeWrapper rootnode(xmlDoc.AsNode());
	CString strnode;
	CString strbody = "";
	CString stra = "";
	CString strb = "";
	CString strc = "";
	CString strd = "";
	CString strkey = "";

	for (int i = 0; i < rootnode.NumNodes(); i++)
	{
		CXmlNodeWrapper node(rootnode.GetNode(i));
		strnode = node.Name();
		if (strnode == "BODY")
			strbody = node.GetText();
		if (strnode == "OPTIONA")
			stra = node.GetText();
		if (strnode == "OPTIONB")
			strb = node.GetText();
		if (strnode == "OPTIONC")
			strc = node.GetText();
		if (strnode == "OPTIOND")
			strd = node.GetText();
		if (strnode == "KEY")
			strkey = node.GetText();
	}

	CSQL sql;
	CString strsql;
	strsql.Format("INSERT INTO QUESTIONS (body,optiona,optionb,optionc,optiond,answerkey) values ('%s','%s','%s','%s','%s','%s')", strbody, stra, strb, strc, strd, strkey);
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));
	rtnflg = db.ExecSqlTransaction(sql);
	return rtnflg;
}

//������ԿЭ�̲���
char sb[MAX], sa[MAX], p[MAX], g[MAX], bu[MAX];
int b = 23456;
/*������msgdispatcher()�����߳��е���
  ���ܣ�ʵ����Ϣ�ķַ�������MSGHEADͷ���ж������Ϣ���ͣ�������ת���Ӧ�Ĺ����ӳ�����ִ��
  �������
  ��
*/

DWORD WINAPI msgdispatcher()
{
	CDATABASE database(STR_DB_PATH, STR_DB_ADMIN, STR_DB_PASSWD, "");//���ݿ�
	RECVD_MSG* pmsg = NULL;
	MSGHEAD res_head;
	int res_flg = 0;
	char* presdata = NULL;
	int reslen = 0;
	CoInitialize(NULL);

	while (!g_msglst.get_endflg())//�ñ�־Ϊ0��ʾѭ��ִ�У�Ϊ1��ʾ��ֹ��Ϣ�ַ��߳�
	{
		pmsg = g_msglst.popmsgls();//����Ϣ����ͷ������һ����Ϣ

		if (!pmsg)		  //����Ϊ�գ�����ѭ������
		{
			Sleep(1);
			continue;
		}

#ifdef MSG_ENCRYPT
		//bool is_negotialte = false;
		int dlen = (int)pmsg->m_datalen;
		char* plainText;
		if (pmsg->m_subtype != MSG_KEY_NEGOTIALTE&& pmsg->m_subtype != MSG_REQCERT && pmsg->m_datalen) {
			((CSecure*)Key)->Decrypt(dlen, pmsg->data, &plainText);
			memmove(pmsg->data, plainText, pmsg->m_datalen);
			delete[] plainText;
		}
#endif

		res_flg = 0;
		reslen = 0;
		CSecure* NegKey;
		char* pCert;
		int CertLen=0;
		FILE* fp;
		bool is_reqcert = false;
		bool is_negotialte = false;
		switch (pmsg->m_type)//������Ϣ���ͽ��п��Ʒַ�
		{
		case MSG_MANAGE:
			switch (pmsg->m_subtype)
			{
			case MSG_LOGON:
				res_flg = check_logon(pmsg->data, database);
				break;
			case MSG_QUESTION_SAVE:
				res_flg = save_question(pmsg->data, database);
				break;
			case MSG_TEST_LOAD:
				res_flg = load_test(&presdata, reslen, database);
				break;
			case MSG_TEST_COMMIT:
				res_flg = score_test(pmsg->data, pmsg->m_datalen, &presdata, reslen, database);
				break;
#ifdef NEG_ENCRYPT
			case MSG_REQCERT:
				is_reqcert = true;
				fp = fopen(CERT_FILE, "rb+");//��֤���ļ������ڴ�
				fseek(fp, 0, SEEK_END);
				CertLen = ftell(fp);
				pCert = (char*)malloc(CertLen);
				fseek(fp, 0, SEEK_SET);
				fread(pCert, 1, CertLen, fp);
				fclose(fp);
				res_flg = 1;
					break;
#endif
			case MSG_KEY_NEGOTIALTE:
#ifdef NEG_ENCRYPT
				NegKey = new CRsa;
				NegKey->Init(KEY_FILE);//��������DHЭ�̵�RSA����

				char* pplain;
				NegKey->Decrypt(pmsg->m_datalen, pmsg->data, &pplain);//����Э����Ϣ
				memmove(p, pplain, MAX);
				memmove(g, pplain + MAX, MAX);				//��ȡp g sa
				memmove(sa, pplain + MAX * 2, MAX);
				delete[] pplain;
#else
				memmove(p, pmsg->data, MAX);
				memmove(g, pmsg->data + MAX, MAX);		//���ܹر�ʱֱ�Ӷ�ȡ����
				memmove(sa, pmsg->data + MAX * 2, MAX);
#endif
				recon(b, p, g, sb);
				getkey(b, sa, p, g, bu);		//���ÿͻ��˷�����p g sa �����Լ���des��Կbu

#ifdef MSG_ENCRYPT
				Key = new CDes;
				Key->Init(bu, MAX);				//������Կbu���������ں���ͨ�����ݼ��ܵ�DES����
#endif
				presdata = new char[MAX];
				memmove(presdata, sb, MAX);
				reslen = MAX;
				res_flg = 1;
				is_negotialte = true;
				break;
			}
		}
		/*��Ϣ������ϣ�������д��Ϣͷ������ͻ��˷�����Ӧ����*/
		memset(&res_head, 0, sizeof(res_head));
		if (res_flg)//����ֵ,1,�ɹ�,0,ʧ��
			res_head.m_type = MSG_RESPONSE_SUCCESS;
		else
			res_head.m_type = MSG_RESPONSE_FAIL;

		memcpy(&res_head.m_id, &pmsg->m_id, sizeof(pmsg->m_id));//�Խ��յ�����id����Ӧ
		res_head.m_subtype = pmsg->m_subtype;
		res_head.m_datalen = reslen;
		char* pciphertext = presdata;//pciphertext���������1�������� 2��Э��ʱRSA���� 3��Э�̳ɹ���DES����

#ifdef NEG_ENCRYPT
		if (is_negotialte == true && is_reqcert == false) {		//�������ԿЭ����Ϣ�����ͷ������м���Կbu
			res_head.m_datalen = NegKey->Encrypt(MAX, presdata, &pciphertext);//RSA����Э����Ϣ
			delete NegKey;//Э�̽���RSA�ͷŶ���
		}

		if (is_reqcert == true) {
			res_head.m_datalen = CertLen;//�����֤��������Ϣ�����͵�����Ϊ֤��
			pciphertext = pCert;
		}
#endif

#ifdef MSG_ENCRYPT
		if (is_negotialte == false && is_reqcert == false)
			int lencipher = Key->Encrypt(res_head.m_datalen, pmsg->data, &pciphertext);//DES��������
#endif

		int max_try = 3;//udp���Ŀ�����������״������ʧ��max_try�������Է��͵Ĵ���
		while (max_try > 0)
		{
			if (cfg.send(pciphertext, &res_head, &pmsg->dst_addr, M_TYPE_RES) < 0)//������Ӧ
			{
				max_try--;
				continue;
			}
			break;
		}
#if defined(MSG_ENCRYPT)||defined(NEG_ENCRYPT)
		delete[] pciphertext;
#endif

		if (max_try <= 0)
			res_flg = 0;
		else
			res_flg = 1;

		del_buf(&pmsg->data);/*����̬�����������գ����տռ�*/
		if (presdata)
			del_buf(&presdata);/*����̬�����������գ����տռ�*/
		if (pmsg)
		{
			delete pmsg;/*����̬�����������գ����տռ�*/
			pmsg = NULL;
		}
		//
	}

	g_msglst.set_endflg(false);/*�����̵߳���ֹ״̬*/
	CoUninitialize();
	return res_flg;
}