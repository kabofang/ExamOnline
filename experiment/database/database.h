#ifndef _H_DATABASE
#define _H_DATABASE
#include "./ibpp/ibpp.h"

/*���ݿⶨ��*/
#define STR_DB_PATH	".\\TEST.GDB"
#define STR_DB_ADMIN	"sysdba"
#define STR_DB_PASSWD	"masterkey"

struct ssql{
	LPTSTR str_sql;
	char *pmsg;//blob����
	struct ssql *next; 
};//�洢SQL����������Ϊ1������ִ�е�����SQL������ڸ�������

typedef struct ssql SSQL;

class CSQL {//����SQL������
	SSQL *lstsql;//���SQL����ͷָ��
	SSQL *ptail;//���SQL����βָ��
public:
	CSQL();//���캯��
	~CSQL();//��������
	SSQL *GetLstHead();//���SQL����ͷָ��
	bool AppendSQL(const LPTSTR str_sql);//����1��SQL���������
	bool CSQL::AppendBLOBSQL(const LPTSTR str_sql,char *pstrblob);//����1��SQL���������������blob����
};

class CDATABASE {
	static IBPP::Database db;
	std::string m_dbname;	// FDB extension (GDB is hacked by Windows Me/XP "System Restore")
	std::string m_username;
	std::string m_password;
	std::string m_servername;
public:
	CDATABASE(const LPTSTR db_name,const LPTSTR username,const LPTSTR password,const LPTSTR servername);
	CDATABASE::~CDATABASE();
	IBPP::Database get_db();
	bool is_initialized();
	bool isconnected();
	void connect();
	void disconnect();
	bool ExecSqlTransaction(CSQL &sql);
	bool ExecSqlTransaction(CSQL &sql,IBPP::Statement &st,IBPP::Transaction &tr);//���غ���������IBPP::Statement���������ô���,��ŷ������ݿ��ȡ������
	//bool ExecINSERTBLOBSqlTransaction(char *sql,unsigned char *pxml,int len);//����BLOB���ݣ����ַ���
};
extern CDATABASE database;
#endif