extern int DES_Encrypt(char *plainText, int datalen,char *keyStr,char *cipherText);
extern int DES_Decrypt(char *cipherText, int &cipherlen,char *keyStr,char **plainText);