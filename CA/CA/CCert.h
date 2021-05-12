#pragma once
#define DATALEN 100
#define KEYLEN 1024
#define HASHLEN 64
class CCert {
private:
	int m_Keylen;
	char m_Owner[DATALEN];
	char m_Lssuer[DATALEN];
	char m_Hash[DATALEN];
	char m_HashValue[HASHLEN];
	char m_PubKey[KEYLEN];
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