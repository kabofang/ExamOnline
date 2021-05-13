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
	GenerateRSAKey();						//生成CA公私密钥对
	while (true) {
		Sleep(1000);
		FILE* fp;
		while (true)
			if (fp = fopen(CERT_FILE, "r"))	//循环判断文件是否存在，
				break;						//若存在，则说明有服务区发送了证书请求
		fclose(fp);
		CCert Cert;
		Cert.DeSerialize(CERT_FILE);		//反序列化证书，读入内存
		Cert.SetHash("DIY_SHA");			//填充相关字段
		Cert.SetLusser("ExamOnline CA");
		SHA1_CONTEXT ctx;
		sha1_init(&ctx);					//自定义的sha1算法消息摘要
		sha1_write(&ctx, (unsigned char*)&Cert, 3*DATALEN + KEYLEN + HASHLEN + sizeof(int));
		sha1_final(&ctx);
		Cert.SetHashValue((char*)(ctx.buf));//填充算好的Hash值并序列化到证书文件
		Cert.Serialize("certplain");
		CRsa* pRsa = new CRsa;
		pRsa->Init(PRI_KEY_FILE);
		char* pciphtertext;					//对证书文件用CA私钥加密
		int ret = pRsa->Encrypt(sizeof(Cert), (char*)&Cert, &pciphtertext);
		fp = fopen(SERVER_CERT_FILE,"wb");	//发送证书给服务器
		fwrite(pciphtertext, ret, 1, fp);
		fclose(fp);
		rename(CERT_FILE, CERT_FILE_OK);
	}
}
