#if !defined(AFX_DLGLOGON_H__F8EDFFBC_FC9D_4A55_A3E8_0308789CA68B__INCLUDED_)
#define AFX_DLGLOGON_H__F8EDFFBC_FC9D_4A55_A3E8_0308789CA68B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DLGLOGON.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDLGLOGON dialog

class CDLGLOGON : public CDialog
{
// Construction
public:
	CDLGLOGON(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDLGLOGON)
	enum { IDD = IDD_DIALOGLOGON };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDLGLOGON)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDLGLOGON)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOGON_H__F8EDFFBC_FC9D_4A55_A3E8_0308789CA68B__INCLUDED_)
