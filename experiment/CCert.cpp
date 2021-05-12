#include "CCert.h"
#include <stdio.h>
#include <iostream>
using namespace std;
CCert::CCert() {
	memset(&m_Keylen, 0, 3 * DATALEN + KEYLEN + HASHLEN + sizeof(m_Keylen));
}

bool CCert::SetOwner(char* pOwner) {
	return strcat(m_Owner, pOwner);
}
char* CCert::GetOwner(void) {
	return m_Owner;
}

bool CCert::SetLusser(char* pLusser) {
	return strcat(m_Lssuer, pLusser);
}
char* CCert::GetLusser(void) {
	return m_Lssuer;
}

bool CCert::SetHash(char* pHash) {
	return strcat(m_Hash, pHash);
}
char* CCert::GetHash(void) {
	return m_Hash;
}

bool CCert::SetPubKey(char* pPubKey) {
	return memmove(m_PubKey, pPubKey, m_Keylen);
}
char* CCert::GetPubKey(void) {
	return m_PubKey;
}

bool CCert::SetHashValue(char* pHashValue) {
	return memmove(m_HashValue, pHashValue, HASHLEN);
}
char* CCert::GetHashValue(void) {
	return m_HashValue;
}

bool CCert::SetKeylen(int x) {
	if (x > KEYLEN)
		return false;
	m_Keylen = x;
	return true;
}
int CCert::GetKeylen(void) {
	return m_Keylen;
}

bool CCert::Serialize(const char* pFilePath)const {
	FILE* fp = fopen(pFilePath, "wb");

	fwrite(m_Lssuer, 1, DATALEN, fp);
	fwrite(m_Owner, 1, DATALEN, fp);
	fwrite(m_Hash, 1, DATALEN, fp);
	fwrite(m_HashValue, 1, HASHLEN, fp);
	fwrite(m_PubKey, 1, KEYLEN, fp);
	fwrite(&m_Keylen, 1, sizeof(m_Keylen), fp);

	fclose(fp);
	return true;
}
bool CCert::DeSerialize(const char* pFilePath) {
	FILE* fp = fopen(pFilePath, "rb");

	fread(m_Lssuer, 1, DATALEN, fp);
	fread(m_Owner, 1, DATALEN, fp);
	fread(m_Hash, 1, DATALEN, fp);
	fread(m_HashValue, 1, HASHLEN, fp);
	fread(m_PubKey, 1, KEYLEN, fp);
	fread(&m_Keylen, 1, sizeof(m_Keylen), fp);

	fclose(fp);
	return true;
}