#include "CSecure.h"
#include <stdio.h>
#define SERVER

bool CSecure::Init(char* FilePath) {
	FILE*fp=fopen(FilePath, "rb");
	if (!fp) {
		return false;
	}
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* pKeychar = new char[filesize];
	if (0 > fread(pKeychar, 1, filesize, fp)) {
		fclose(fp);
		return false;
	}
	Char2Key(pKeychar, filesize);
	SetKeylen();
	return true;
}
bool CSecure::Init(char* pKeychar,int len) {
	Char2Key(pKeychar,len);
	m_Keylen = SetKeylen();
	return true;
}

bool CSecure::Char2Key(char* pKeychar, int len){
	m_pKey = pKeychar;
	return true;
}
bool CSecure::SetKeylen() {
	m_Keylen = 0;
	return 0;
}

CSecure::~CSecure() {
	Release();
	return;
}
bool CSecure::Release() {
	return true;
}

int CSecure::GetKeylen() {
	return m_Keylen;
}