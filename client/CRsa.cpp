#include "CRsa.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <fileapi.h>
#include<openssl/pem.h>
#include<openssl/rsa.h>
bool CRsa::Char2Key(char* pkeychar,int len) {
	m_pKey = new RSA;
	char temppath[512] = "tempkey.pem";
	//GetTempPathA(512, temppath);
	//strcat(temppath, "tempkey.pem");
	FILE*fp = fopen(temppath, "wb+");
	if (!fp) {
		return NULL;
	}
	fwrite(pkeychar, 1, len, fp);
	fclose(fp);
	fp=fopen(temppath, "r");
#ifdef CLIENT
	m_pKey = PEM_read_RSAPublicKey(fp, 0, 0, 0);
#else
	m_pKey = PEM_read_RSAPrivateKey(fp, 0, 0, 0);
#endif
	fclose(fp);
	remove(temppath);
	return true;
}

bool CRsa::SetKeylen() {
	m_Keylen = RSA_size((const RSA*)m_pKey);
	return true;
}
int CRsa::Encrypt(int lenplaintext, char* pplaintext, char** pciphertext) {
	if (lenplaintext <= 0)
		return 0;
	int lenpaddingtext = ((lenplaintext - 1) / m_Keylen + 1) * m_Keylen;
	char* paddingtext = new char[lenpaddingtext];
	*pciphertext = new char[lenpaddingtext];
	memset(paddingtext, 0, lenpaddingtext);
	memmove(paddingtext, pplaintext, lenplaintext);
	int lencipher = 0;
	for (int i = 0; i < lenpaddingtext; i += m_Keylen)
#ifdef CLIENT
		lencipher += RSA_public_encrypt(m_Keylen, (unsigned char*)(paddingtext + i), \
			(unsigned char*)(*pciphertext + i), (RSA*)m_pKey, RSA_NO_PADDING);
#else
		lencipher += RSA_private_encrypt(m_Keylen, (unsigned char*)(paddingtext + i), \
			(unsigned char*)(*pciphertext + i), (RSA*)m_pKey, RSA_NO_PADDING);
#endif
	delete[] paddingtext;
	return lencipher;
}

int CRsa::Decrypt(int lenciphertext, char* pciphertext, char** pplaintext) {
	*pplaintext = new char[lenciphertext];
	int lenplain = 0;
	for (int i = 0; i < lenciphertext; i += m_Keylen)
#ifdef CLIENT
		lenplain += RSA_public_decrypt(m_Keylen, (unsigned char*)(pciphertext + i), \
			(unsigned char*)(*pplaintext + i), (RSA*)m_pKey, RSA_NO_PADDING);
#else
		lenplain += RSA_private_decrypt(m_Keylen, (unsigned char*)(pciphertext + i), \
			(unsigned char*)(*pplaintext + i), (RSA*)m_pKey, RSA_NO_PADDING);
#endif
	return lenplain;
}

bool CRsa::Release() {
	RSA_free((RSA*)m_pKey);
	return true;
}