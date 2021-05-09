#include "../stdafx.h"
#include "database.h"
#include <string>
IBPP::Database CDATABASE::db;

CDATABASE::CDATABASE(const LPTSTR db_name,const LPTSTR username,const LPTSTR password,const LPTSTR servername)
{
	m_dbname = db_name;
	m_username = username;
	m_password = password;
	m_servername = servername;
	if (!isconnected())
		connect();
}

CDATABASE::~CDATABASE()
{
	//disconnect();
}

IBPP::Database CDATABASE::get_db()
{
	return db;
}

bool CDATABASE::isconnected()
{
	if (db==NULL)
		return false;
	else
		return db->Connected();
}

bool CDATABASE::is_initialized()
{
	if ((m_dbname=="")||(m_username==""))
		return false;
	else
		return true;
}

void CDATABASE::connect()
{
	if (!isconnected())
	{
		if (!is_initialized())//�Ƿ��Ѿ���ȡ������������Ϣ
			return;
		db = IBPP::DatabaseFactory(m_servername, m_dbname, m_username, m_password);
		db->Connect();
	}
}

void CDATABASE::disconnect()
{
	if (isconnected())
	{
		if (db->Connected())
			db->Disconnect();
	}
}

//��1�������SQL�����Ϊ1������������true,�ɹ���false��ʧ��
bool CDATABASE::ExecSqlTransaction(CSQL &sql)
{
	bool rtn_flg = true;
	if (!isconnected())
		return false;//db��Ҫ�����ݿ��Ѿ���������
	
	IBPP::Transaction tr = IBPP::TransactionFactory(db);
	tr->Start();

	try{
		
		IBPP::Statement st = IBPP::StatementFactory(db, tr);
		SSQL *psql = sql.GetLstHead();
		while (psql)
		{
			if (psql->str_sql)
				st->Execute(psql->str_sql);
			psql = psql->next;
		}
		tr->Commit();
	}
	catch(IBPP::SQLException &e){ 
        tr->Rollback();
		rtn_flg = false;
	}
	
	return rtn_flg;
}

bool CDATABASE::ExecSqlTransaction(CSQL &sql,IBPP::Statement &st,IBPP::Transaction &tr)//���غ���������IBPP::Statement���������ô���,��ŷ������ݿ��ȡ������
{
	bool rtn_flg = true;
	
	st = IBPP::StatementFactory(db, tr);
	SSQL *psql = sql.GetLstHead();
	while (psql)
	{
		if (psql->str_sql)
			st->Execute(psql->str_sql);
		psql = psql->next;
	}
	
	return rtn_flg;
}

CSQL::CSQL()
{
	lstsql = NULL;
	ptail = lstsql;
}

CSQL::~CSQL()//���SQL��������
{
	SSQL *ptmp=NULL;
	while (lstsql)
	{
		ptmp = lstsql;
		lstsql = lstsql->next;
		
		if (ptmp->str_sql)
			delete ptmp->str_sql;
		if (ptmp->pmsg)
			delete ptmp->pmsg;
		delete ptmp;
	}
}

bool CSQL::AppendSQL(const LPTSTR str_sql)//����1��SQL���������
{
	SSQL *pnew = NULL;
	if (!str_sql)
		return false;
	
	pnew = new SSQL;
	if (!pnew)
		return false;
	memset(pnew,0,sizeof(SSQL));
	pnew->str_sql = new TCHAR[strlen(str_sql)+1];//����lenΪstr_sql�ַ�������
	if (!pnew->str_sql)
		return false;
	memset(pnew->str_sql,0,strlen(str_sql)+1);
	memcpy(pnew->str_sql,str_sql,strlen(str_sql));

	if (!lstsql)
	{
		pnew->next = lstsql;
		lstsql = pnew;
		ptail = lstsql;
	}
	else
	{
		pnew->next = ptail->next;
		ptail->next = pnew;
		ptail = pnew;
	}

	return true;
}

bool CSQL::AppendBLOBSQL(const LPTSTR str_sql,char *pstrblob)//����1��SQL���������������blob����
{
	SSQL *pnew = NULL;
	if (!str_sql)
		return false;
	
	pnew = new SSQL;
	if (!pnew)
		return false;
	memset(pnew,0,sizeof(SSQL));
	pnew->str_sql = new TCHAR[strlen(str_sql)+1];//����lenΪstr_sql�ַ�������
	if (!pnew->str_sql)
		return false;
	memset(pnew->str_sql,0,strlen(str_sql)+1);
	memcpy(pnew->str_sql,str_sql,strlen(str_sql));
	if (pstrblob)
	{
		pnew->pmsg = new char[strlen(pstrblob)+1];
		if (!pnew->pmsg)
			return false;
		memset(pnew->pmsg,0,strlen(pstrblob)+1);
		memcpy(pnew->pmsg,pstrblob,strlen(pstrblob));
	}


	if (!lstsql)
	{
		pnew->next = lstsql;
		lstsql = pnew;
		ptail = lstsql;
	}
	else
	{
		pnew->next = ptail->next;
		ptail->next = pnew;
		ptail = pnew;
	}

	return true;
}

SSQL *CSQL::GetLstHead()
{
	return lstsql;
}