// experimentDlg.h : header file
//

#if !defined(AFX_EXPERIMENTDLG_H__A3D7F738_CA33_4F30_AC01_7EBF46B560F3__INCLUDED_)
#define AFX_EXPERIMENTDLG_H__A3D7F738_CA33_4F30_AC01_7EBF46B560F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CExperimentDlg dialog

class CExperimentDlg : public CDialog
{
// Construction
public:
	CExperimentDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CExperimentDlg)
	enum { IDD = IDD_EXPERIMENT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExperimentDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CExperimentDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPERIMENTDLG_H__A3D7F738_CA33_4F30_AC01_7EBF46B560F3__INCLUDED_)
