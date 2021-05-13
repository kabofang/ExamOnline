#pragma once
#include "CSecure.h"

class CRsa :public CSecure {
private:
	virtual bool Char2Key(char*, int);
	virtual bool SetKeylen();
public:
	virtual int Encrypt(int lenplaintext, char* pplaintext, char** pciphertext);
	virtual int Decrypt(int lenciphertext, char* pciphertext, char** pplaintext);
	~CRsa();
};

