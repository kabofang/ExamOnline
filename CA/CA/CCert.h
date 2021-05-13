#pragma once
#define DATALEN 100
#define KEYLEN 1024
#define HASHLEN 64
class CCert {
private:
	int m_Keylen;//公钥长度
	char m_Owner[DATALEN];//所有者
	char m_Lssuer[DATALEN];//颁发者
	char m_Hash[DATALEN];//消息摘要算法
	char m_HashValue[HASHLEN];//Hash值
	char m_PubKey[KEYLEN];//公钥
public:
	CCert();

	bool SetOwner(char*pOwner);
	char* GetOwner(void);

	bool SetLusser(char* pLusser);
	char* GetLusser(void);

	bool SetHash(char* pHash);
	char* GetHash(void);

	bool SetPubKey(char* pOwner);
	char* GetPubKey(void);

	bool SetHashValue(char* pOwner);
	char* GetHashValue(void);

	bool SetKeylen(int x);
	int GetKeylen(void);

	bool Serialize(const char* pFilePath) const;
	bool DeSerialize(const char* pFilePath);
};