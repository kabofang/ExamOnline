#ifndef _H_DATABASE
#define _H_DATABASE
#include "./ibpp/ibpp.h"

/*数据库定义*/
#define STR_DB_PATH	".\\TEST.GDB"
#define STR_DB_ADMIN	"sysdba"
#define STR_DB_PASSWD	"masterkey"

struct ssql{
	LPTSTR str_sql;
	char *pmsg;//blob数据
	struct ssql *next; 
};//存储SQL语句的链表，作为1次事务执行的所有SQL语句存放在该链表中

typedef struct ssql SSQL;

class CSQL {//创建SQL操作类
	SSQL *lstsql;//存放SQL链表头指针
	SSQL *ptail;//存放SQL链表尾指针
public:
	CSQL();//构造函数
	~CSQL();//析构函数
	SSQL *GetLstHead();//获得SQL链表头指针
	bool AppendSQL(const LPTSTR str_sql);//加入1条SQL语句至链表
	bool CSQL::AppendBLOBSQL(const LPTSTR str_sql,char *pstrblob);//加入1条SQL语句至链表，并加入blob数据
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
	bool ExecSqlTransaction(CSQL &sql,IBPP::Statement &st,IBPP::Transaction &tr);//重载函数，其中IBPP::Statement对象以引用传入,存放访问数据库获取的数据
	//bool ExecINSERTBLOBSqlTransaction(char *sql,unsigned char *pxml,int len);//插入BLOB数据，非字符串
};
extern CDATABASE database;
#endif