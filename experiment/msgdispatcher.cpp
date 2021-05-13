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

//将接收到的消息加入消息链表，供线程处理
S_MSGLST::S_MSGLST()
{
	end_flg = 0;		//消息线程终止标志为0表示不终止
	phead = NULL;//存放消息链表头指针
	ptail = NULL;//存放消息链表尾指针
	InitializeCriticalSection(&msgdis_section);
}

/*析构函数*/
S_MSGLST::~S_MSGLST()
{
	struct recvd_msg* ptmp = phead;
	while (phead)//释放分配的所有消息缓存
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
	EnterCriticalSection(&msgdis_section);//申请访问锁
	end_flg = flg;//设置标志
	LeaveCriticalSection(&msgdis_section);//释放链表锁
}

bool S_MSGLST::get_endflg()
{
	bool rtn_flg;
	EnterCriticalSection(&msgdis_section);//申请访问锁
	rtn_flg = end_flg;//获取终止标志
	LeaveCriticalSection(&msgdis_section);//释放链表锁

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

	if (pmsghead->m_datalen && pdata)//存在数据
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

	EnterCriticalSection(&msgdis_section);//申请访问锁

	if (!phead)//链表为空
	{
		pmsg->next = phead;
		phead = pmsg;
		ptail = phead;
	}
	else//链表至少一个节点
	{
		pmsg->next = ptail->next;
		ptail = pmsg;
	}

	LeaveCriticalSection(&msgdis_section);//释放链表锁

	return true;
}

RECVD_MSG* S_MSGLST::popmsgls()//弹出一条消息
{
	RECVD_MSG* pmsg = NULL;
	EnterCriticalSection(&msgdis_section);//申请链表锁
	if (phead)
	{
		pmsg = phead;
		phead = pmsg->next;
	}
	LeaveCriticalSection(&msgdis_section);//释放链表锁
	return pmsg;
}

//函数名recv_proc(),接收处理，创建消息分发进程
//pobj存放响应目标地址
int recv_proc(void* pobj, MSGHEAD* phead, char* pdata, char** presdata)
{
	if (phead == NULL)
		return PROC_FAIL;

	//接收到响应消息，直接返回
	if ((phead->m_type == MSG_RESPONSE_SUCCESS) || (phead->m_type == MSG_RESPONSE_FAIL))
		return PROC_SUCCESS;

	if (!g_msglst.copytolst(phead, pdata, pobj))//将接收到的消息加入消息链表，供线程处理
		return PROC_FAIL;
	return PROC_PROCESSING;//提交到msgdispatcher线程处理
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
		sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//加入SQL语句
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
	CXmlDocumentWrapper xmlDoc;//存放XML包装类对象
	xmlDoc.LoadXML("<QUESTIONS></QUESTIONS>");
	CXmlNodeWrapper rootnode(xmlDoc.AsNode());
	//
	strsql.Format("select * from QUESTIONS");

	CSQL sql;
	IBPP::Statement st;
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//加入SQL语句
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
		while (st->Fetch())//对角色数据集进行遍历
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

/*用户登录服务器端的实现函数*/
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
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));//加入SQL语句
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
		while (st->Fetch())//对角色数据集进行遍历
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

//定义密钥协商参数
char sb[MAX], sa[MAX], p[MAX], g[MAX], bu[MAX];
int b = 23456;
/*函数名msgdispatcher()，在线程中调用
  功能：实现消息的分发，根据MSGHEAD头部中定义的消息类型，将控制转入对应的功能子程序中执行
  输入参数
  无
*/

DWORD WINAPI msgdispatcher()
{
	CDATABASE database(STR_DB_PATH, STR_DB_ADMIN, STR_DB_PASSWD, "");//数据库
	RECVD_MSG* pmsg = NULL;
	MSGHEAD res_head;
	int res_flg = 0;
	char* presdata = NULL;
	int reslen = 0;
	CoInitialize(NULL);

	while (!g_msglst.get_endflg())//该标志为0表示循环执行，为1表示终止消息分发线程
	{
		pmsg = g_msglst.popmsgls();//从消息链表头部弹出一条消息

		if (!pmsg)		  //链表为空，单次循环结束
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
		switch (pmsg->m_type)//根据消息类型进行控制分发
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
				fp = fopen(CERT_FILE, "rb+");//将证书文件读入内存
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
				NegKey->Init(KEY_FILE);//创建加密DH协商的RSA对象

				char* pplain;
				NegKey->Decrypt(pmsg->m_datalen, pmsg->data, &pplain);//解密协商信息
				memmove(p, pplain, MAX);
				memmove(g, pplain + MAX, MAX);				//获取p g sa
				memmove(sa, pplain + MAX * 2, MAX);
				delete[] pplain;
#else
				memmove(p, pmsg->data, MAX);
				memmove(g, pmsg->data + MAX, MAX);		//加密关闭时直接读取数据
				memmove(sa, pmsg->data + MAX * 2, MAX);
#endif
				recon(b, p, g, sb);
				getkey(b, sa, p, g, bu);		//利用客户端发来的p g sa 生成自己的des密钥bu

#ifdef MSG_ENCRYPT
				Key = new CDes;
				Key->Init(bu, MAX);				//利用密钥bu，创建用于后续通信数据加密的DES对象
#endif
				presdata = new char[MAX];
				memmove(presdata, sb, MAX);
				reslen = MAX;
				res_flg = 1;
				is_negotialte = true;
				break;
			}
		}
		/*消息处理完毕，重新填写消息头部，向客户端返回响应数据*/
		memset(&res_head, 0, sizeof(res_head));
		if (res_flg)//返回值,1,成功,0,失败
			res_head.m_type = MSG_RESPONSE_SUCCESS;
		else
			res_head.m_type = MSG_RESPONSE_FAIL;

		memcpy(&res_head.m_id, &pmsg->m_id, sizeof(pmsg->m_id));//对接收到命令id的响应
		res_head.m_subtype = pmsg->m_subtype;
		res_head.m_datalen = reslen;
		char* pciphertext = presdata;//pciphertext三种情况：1、不加密 2、协商时RSA加密 3、协商成功后DES加密

#ifdef NEG_ENCRYPT
		if (is_negotialte == true && is_reqcert == false) {		//如果是密钥协商消息，则发送服务器中间密钥bu
			res_head.m_datalen = NegKey->Encrypt(MAX, presdata, &pciphertext);//RSA加密协商信息
			delete NegKey;//协商结束RSA释放对象
		}

		if (is_reqcert == true) {
			res_head.m_datalen = CertLen;//如果是证书请求消息，则发送的数据为证书
			pciphertext = pCert;
		}
#endif

#ifdef MSG_ENCRYPT
		if (is_negotialte == false && is_reqcert == false)
			int lencipher = Key->Encrypt(res_head.m_datalen, pmsg->data, &pciphertext);//DES加密数据
#endif

		int max_try = 3;//udp报文可能由于网络状况而丢失，max_try给定重试发送的次数
		while (max_try > 0)
		{
			if (cfg.send(pciphertext, &res_head, &pmsg->dst_addr, M_TYPE_RES) < 0)//发送响应
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

		del_buf(&pmsg->data);/*将动态分配的数据清空，回收空间*/
		if (presdata)
			del_buf(&presdata);/*将动态分配的数据清空，回收空间*/
		if (pmsg)
		{
			delete pmsg;/*将动态分配的数据清空，回收空间*/
			pmsg = NULL;
		}
		//
	}

	g_msglst.set_endflg(false);/*设置线程的终止状态*/
	CoUninitialize();
	return res_flg;
}