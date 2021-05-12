#pragma once
#include<stdio.h>
#include<openssl/pem.h>

#define KEY_LENGTH 4096
#define PRI_KEY_FILE "prikey.pem"
#define PUB_KEY_FILE "pubkey.pem"
bool GenerateRSAKey(void);