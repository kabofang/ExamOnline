#include<stdio.h>
#include<openssl/pem.h>

#define KEY_LENGTH 4096
#define PRI_KEY_FILE "../../experiment/prikey.pem"
#define PUB_KEY_FILE "../../client/pubkey.pem"
void generateRSAKey(void)
{ 
	size_t pri_len;
	size_t pub_len;
	char* pri_key = NULL;
	char* pub_key = NULL;
  
	RSA* keypair = RSA_generate_key(KEY_LENGTH, RSA_F4, NULL, NULL);

	BIO* pri = BIO_new(BIO_s_mem());
	BIO* pub = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pub, keypair);
  
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	pri_key = (char*)malloc(pri_len + 1);
	pub_key = (char*)malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';


	FILE* pubFile = fopen(PUB_KEY_FILE, "w");
	if (pubFile == NULL)
		return;
	fputs(pub_key, pubFile);
	fclose(pubFile);

	FILE* priFile = fopen(PRI_KEY_FILE, "w");
	if (priFile == NULL)
		return;
	fputs(pri_key, priFile);
	fclose(priFile);

	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);

	free(pri_key);
	free(pub_key);
}

int main() {
	generateRSAKey();
	return 0;
}