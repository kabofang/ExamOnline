#pragma once


// CDLGQUESTIONEDIT dialog

class CDLGQUESTIONEDIT : public CDialog
{
	DECLARE_DYNAMIC(CDLGQUESTIONEDIT)

public:
	CDLGQUESTIONEDIT(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDLGQUESTIONEDIT();

// Dialog Data
	enum { IDD = IDD_DIALOGEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
};