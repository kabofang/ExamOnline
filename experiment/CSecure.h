#pragma once
#define SERVER
#define MSG_ENCRYPT
#define NEG_ENCRYPT
#define KEY_FILE "prikey.pem"
#define PUBKEY_FILE "pubkey.pem"
#define CERT_FILE "cert"
class CSecure {
protected:
	void* m_pKey;
	int m_Keylen;
protected:
	virtual bool Char2Key(char*, int);

	virtual bool SetKeylen();
public:
	int GetKeylen();

public:
	bool Init(char*);
	bool Init(char*, int);

	virtual int Encrypt(int lenplaintext, char* pplaintext, char** pciphertext) = 0;
	virtual int Decrypt(int lenciphertext, char* pciphertext, char** pplaintext) = 0;

	virtual ~CSecure();
};
