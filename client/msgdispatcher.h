#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "stdafx.h"
#include "./netlib/message.h"
#define  WM_LOADTEST		WM_USER + 100
class CMSGDISPATCHER
{
public:
	static HWND h_dlgtest;//±£´æDialogTest´°¿Ú¾ä±ú
	static bool do_response(ULONG saddr,int subtype,char *pdata,int len);
};

extern int msgdispatcher(void *pobj,MSGHEAD *phead,char *pdata,char **presdata);
#endif