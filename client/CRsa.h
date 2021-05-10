#pragma once
#include "CSecure.h"
#define CLIENT
class CRsa:public CSecure{
private:
	virtual bool Char2Key(char*,int);
	virtual bool SetKeylen();
	virtual bool Release();
public:
	virtual int Encrypt(int lenplaintext, char* pplaintext, char** pciphertext);
	virtual int Decrypt(int lenciphertext, char* pciphertext, char** pplaintext);
};

