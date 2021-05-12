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
	CString strrandom, strinput;

	GetDlgItemText(IDC_STATICRANDOM, strrandom);
	//验证校验码是否匹配，同学们自行处理
	//
	GetDlgItemText(IDC_EDITCHECKSUM, strinput);
	if (0 != strcmp(strinput, strrandom)) {
		AfxMessageBox("校验码错误！");
		return;
	}


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

BOOL CDLGLOGON::OnInitDialog() {
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	time_t t;
	srand((unsigned)time(&t));

	char s[5];
	memset(s, 0, 5);
	for (int i = 0; i < 4; i++)
	{
		s[i] = 'A' + rand() % 10;
	}

	SetDlgItemText(IDC_STATICRANDOM, s);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}