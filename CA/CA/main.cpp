extern "C"
{
#include <openssl/applink.c>
};
#include "sha.h"
#include "CCert.h"
#include <stdio.h>
#include <windows.h>
#include "CSecure.h"
#include "CRsa.h"
#include "keygen.h"
#define CERT_FILE "cert"
#define CERT_FILE_OK "certok"
#define SERVER_CERT_FILE "../../experiment/cert"

/*extern void sha1_init( SHA1_CONTEXT *hd );
extern void sha1_write( SHA1_CONTEXT *hd, unsigned char *inbuf, size_t inlen);
extern void sha1_final(SHA1_CONTEXT *hd);*/
int main() {
	GenerateRSAKey();
	while (true) {
		Sleep(3000);
		FILE* fp;
		while (true)
			if (fp = fopen(CERT_FILE, "r"))
				break;
		fclose(fp);
		CCert Cert;
		Cert.DeSerialize(CERT_FILE);
		Cert.SetHash("DIY_SHA");
		Cert.SetLusser("ExamOnline CA");
		SHA1_CONTEXT ctx;
		sha1_init(&ctx);
		sha1_write(&ctx, (unsigned char*)&Cert, 3*DATALEN + KEYLEN + HASHLEN + sizeof(int));
		sha1_final(&ctx);
		Cert.SetHashValue((char*)(ctx.buf));
		Cert.Serialize("certplain");
		CRsa* pRsa = new CRsa;
		pRsa->Init(PRI_KEY_FILE);
		char* pciphtertext;
		int ret = pRsa->Encrypt(sizeof(Cert), (char*)&Cert, &pciphtertext);
		fp = fopen(SERVER_CERT_FILE,"wb");
		fwrite(pciphtertext, ret, 1, fp);
		fclose(fp);
		rename(CERT_FILE, CERT_FILE_OK);
	}
}
