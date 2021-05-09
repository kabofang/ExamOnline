// DLGLOGON.cpp : implementation file
//

#include "stdafx.h"
#include "client.h"
#include "DLGLOGON.h"
#include "./netlib/message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDLGLOGON dialog


CDLGLOGON::CDLGLOGON(CWnd* pParent /*=NULL*/)
	: CDialog(CDLGLOGON::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDLGLOGON)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDLGLOGON::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDLGLOGON)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDLGLOGON, CDialog)
	//{{AFX_MSG_MAP(CDLGLOGON)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDLGLOGON message handlers
#define LEN_USER 20
#define LEN_PASSWD 20
void CDLGLOGON::OnOK() 
{
	CString StrUsername, StrPassword;
	GetDlgItemText(IDC_EDITUSERNAME, StrUsername);
	GetDlgItemText(IDC_EDITPASSWD, StrPassword);

	if (StrUsername.GetLength() >= LEN_USER || \
		StrPassword.GetLength() >= LEN_PASSWD) {
		MessageBox("It's too long for input!", "Error", MB_OK);
		return;
	}

	char* pMsg = new char[LEN_USER + LEN_PASSWD];
	memset(pMsg, 0, LEN_USER + LEN_PASSWD);

	memmove(pMsg, (char*)(LPCTSTR)StrUsername, StrUsername.GetLength());
	memmove(pMsg + 20, (char*)(LPCTSTR)StrPassword, StrPassword.GetLength());

	switch (DoMsgSend(MSG_MANAGE, MSG_LOGON, pMsg, LEN_USER + LEN_PASSWD)) {
	case LOGON_SUC:
		CDialog::OnOK();
		break;
	case LOGON_FAIL:
		AfxMessageBox("Authentication Failed!");
		break;
	case LOGON_TIMEOUT:
		AfxMessageBox("Logon timeout!");
		break;
	default:
		break;
	}
	if (pMsg)
		delete[]pMsg;
}
