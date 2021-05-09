#pragma once

struct question {
	int id;
	char *body;
	char *optiona;
	char *optionb;
	char *optionc;
	char *optiond;
	int key;//1 A 2 B 3 C 4 D
	struct question *next;
};

struct answer{
	int no;
	int key;
};

typedef struct answer ANSWER;
typedef struct question QUESTION;

// CDLGTEST dialog

class CDLGTEST : public CDialog
{
	DECLARE_DYNAMIC(CDLGTEST)
	QUESTION *pqhead;

public:
	CDLGTEST(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDLGTEST();
	void ParseNode(IDispatch *pNode,QUESTION **pqhead);

// Dialog Data
	enum { IDD = IDD_DIALOG_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnLoadTest(WPARAM wParam, LPARAM lParam);//获取部门、角色、员工信息
	afx_msg void OnBnClickedButtonup();
	afx_msg void OnBnClickedButtondown();
	afx_msg void OnBnClickedButton1();
};
