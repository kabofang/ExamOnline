#pragma once
#define KEY_FILE "prikey.pem"
#include<openssl/pem.h>
#include<openssl/rsa.h>
#include<openssl/err.h>

#include<stdio.h>
#include<string.h>

RSA* GetkeygenRSA(void);
bool RSA_Encrypt(int lenplaintext,char* plaintext,char*pciphertext,RSA* p_Key);
bool RSA_Decrypt(int lenciphertext,char* pciphertext,char*pplaintext,RSA* p_Key);
