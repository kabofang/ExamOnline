#pragma once


// CDlgAddUser dialog

class CDlgAddUser : public CDialog
{
	DECLARE_DYNCREATE(CDlgAddUser)

public:
	CDlgAddUser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAddUser();

// Dialog Data
	enum { IDD = IDD_DIALOGAdduser};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
