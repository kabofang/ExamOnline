// DlgAddUser.cpp : implementation file
//

#include "stdafx.h"
#include "experiment.h"
#include "DlgAddUser.h"
#include ".\dlgadduser.h"
#include "database/database.h"

// CDlgAddUser dialog

IMPLEMENT_DYNCREATE(CDlgAddUser, CDialog)

CDlgAddUser::CDlgAddUser(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAddUser::IDD, pParent)
{
}

CDlgAddUser::~CDlgAddUser()
{
}

void CDlgAddUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CDlgAddUser::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDlgAddUser, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// CDlgAddUser message handlers

void CDlgAddUser::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDATABASE db(STR_DB_PATH,STR_DB_ADMIN,STR_DB_PASSWD,"");//数据库
	CSQL sql;
	CString strsql;
	CString strusername;
	CString strpasswd;
	
	GetDlgItemText(IDC_EDITUSERNAME,strusername);
	GetDlgItemText(IDC_EDITPASSWORD,strpasswd);
	
	strsql.Format("INSERT INTO USERS (USERNAME,PASSWD,SALT) values ('%s','%s',%d)",strusername,strpasswd,0);
	sql.AppendSQL((LPTSTR)(LPCTSTR)(strsql));
	bool rtnflg = db.ExecSqlTransaction(sql);
	if (rtnflg == false)
		AfxMessageBox("保存用户名失败！");
	else
		AfxMessageBox("保存用户名成功！");
}