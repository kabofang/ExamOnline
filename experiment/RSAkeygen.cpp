extern "C"
{
#include <openssl/applink.c>
};
#include"RSAkeygen.h"

RSA* GetkeygenRSA(void){
	FILE* fp;
	RSA* p_Key;
	if (NULL == (fp = fopen(KEY_FILE, "r")))
		return NULL;
	if (NULL == (p_Key = PEM_read_RSAPrivateKey(fp, 0, 0, 0))){
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	return p_Key;
}

bool RSA_Encrypt(int lenplaintext,char* plaintext,char*pciphertext,RSA* p_Key){
	int Keylen=RSA_size(p_Key);
	if (lenplaintext > Keylen)
		return false;
	char* temp = new char[Keylen];
	memset(temp, 0, Keylen);
	memmove(temp, plaintext, lenplaintext);
	if (RSA_private_encrypt(Keylen, (unsigned char*)temp, \
		(unsigned char*)pciphertext, p_Key, RSA_NO_PADDING) < 0){
		delete temp;
		return false;
	}
	delete temp;
	return true;
}

bool RSA_Decrypt(int lenciphertext,char* pciphertext,char*pplaintext,RSA* p_Key){
	int Keylen=RSA_size(p_Key);
	if (RSA_private_decrypt(Keylen, (unsigned char*)pciphertext, \
		(unsigned char*)pplaintext, p_Key, RSA_NO_PADDING) < 0)
		return false;
	return true;
}