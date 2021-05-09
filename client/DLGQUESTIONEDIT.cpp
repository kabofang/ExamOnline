// DLGQUESTIONEDIT.cpp : implementation file
//

#include "stdafx.h"
#include "client.h"
#include "DLGQUESTIONEDIT.h"
#include "./netlib/message.h"
#include "./xml/XmlNodeWrapper.h"


// CDLGQUESTIONEDIT dialog

IMPLEMENT_DYNAMIC(CDLGQUESTIONEDIT, CDialog)
CDLGQUESTIONEDIT::CDLGQUESTIONEDIT(CWnd* pParent /*=NULL*/)
	: CDialog(CDLGQUESTIONEDIT::IDD, pParent)
{
}

CDLGQUESTIONEDIT::~CDLGQUESTIONEDIT()
{
}

void CDLGQUESTIONEDIT::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDLGQUESTIONEDIT, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
END_MESSAGE_MAP()


// CDLGQUESTIONEDIT message handlers

void CDLGQUESTIONEDIT::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_EDITBODY,"");
	SetDlgItemText(IDC_EDIT1,"");
	SetDlgItemText(IDC_EDIT2,"");
	SetDlgItemText(IDC_EDIT3,"");
	SetDlgItemText(IDC_EDIT4,"");
}

void CDLGQUESTIONEDIT::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CString strtext;
	GetDlgItemText(IDC_EDIT5,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	if (strtext == "")
	{
		AfxMessageBox("答案不能为空！");
		return;
	}
	CXmlDocumentWrapper xmlDoc;//存放XML包装类对象
	
	xmlDoc.LoadXML("<QUESTIONINFO></QUESTIONINFO>");
    
	CXmlNodeWrapper rootnode(xmlDoc.AsNode());   
    
	CXmlNodeWrapper node(rootnode.InsertNode(0,"BODY"));
	GetDlgItemText(IDC_EDITBODY,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node.SetText(strtext);
	CXmlNodeWrapper node_a(rootnode.InsertNode(1,"OPTIONA"));
	GetDlgItemText(IDC_EDIT1,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node_a.SetText(strtext);
	CXmlNodeWrapper node_b(rootnode.InsertNode(2,"OPTIONB"));
	GetDlgItemText(IDC_EDIT2,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node_b.SetText(strtext);
	CXmlNodeWrapper node_c(rootnode.InsertNode(3,"OPTIONC"));
	GetDlgItemText(IDC_EDIT3,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node_c.SetText(strtext);
	CXmlNodeWrapper node_d(rootnode.InsertNode(4,"OPTIOND"));
	GetDlgItemText(IDC_EDIT4,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node_d.SetText(strtext);
	CXmlNodeWrapper node_key(rootnode.InsertNode(5,"KEY"));
	GetDlgItemText(IDC_EDIT5,strtext);
	strtext.TrimLeft();
	strtext.TrimRight();
	node_key.SetText(strtext);
	if (DoMsgSend(MSG_MANAGE,MSG_QUESTION_SAVE,(char *)(LPCTSTR)xmlDoc.GetXML(),xmlDoc.GetXML().GetLength()+1) == false)
		AfxMessageBox("保存试题失败！");
	else
		AfxMessageBox("保存试题成功！");
}