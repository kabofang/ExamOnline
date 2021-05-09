#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDLGEDIT dialog

class CDLGEDIT : public CDialog
{
	DECLARE_DYNAMIC(CDLGEDIT)

public:
	CDLGEDIT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDLGEDIT();

// Dialog Data
	enum { IDD = IDD_DIALOGEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_rteditbody;
	CEdit m_edb;
	CEdit m_eda;
	CEdit m_edc;
	CEdit m_edd;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	virtual BOOL OnInitDialog();
};
