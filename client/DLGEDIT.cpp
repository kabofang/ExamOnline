// DLGEDIT.cpp : implementation file
//

#include "stdafx.h"
#include "client.h"
#include "DLGEDIT.h"
#include ".\dlgedit.h"
#include "./netlib/message.h"
#include "./xml/XmlNodeWrapper.h"


// CDLGEDIT dialog

IMPLEMENT_DYNAMIC(CDLGEDIT, CDialog)
CDLGEDIT::CDLGEDIT(CWnd* pParent /*=NULL*/)
	: CDialog(CDLGEDIT::IDD, pParent)
{
}

CDLGEDIT::~CDLGEDIT()
{
}

void CDLGEDIT::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDITBODY, m_rteditbody);
	DDX_Control(pDX, IDC_EDIT2, m_edb);
	DDX_Control(pDX, IDC_EDIT1, m_eda);
	DDX_Control(pDX, IDC_EDIT3, m_edc);
	DDX_Control(pDX, IDC_EDIT4, m_edd);
}


BEGIN_MESSAGE_MAP(CDLGEDIT, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
END_MESSAGE_MAP()


// CDLGEDIT message handlers

void CDLGEDIT::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_RICHEDITBODY,"");
	SetDlgItemText(IDC_EDIT1,"");
	SetDlgItemText(IDC_EDIT2,"");
	SetDlgItemText(IDC_EDIT3,"");
	SetDlgItemText(IDC_EDIT4,"");
}

void CDLGEDIT::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CXmlDocumentWrapper xmlDoc;//存放XML包装类对象
	CString strtext;
	xmlDoc.LoadXML("<QUESTIONINFO></QUESTIONINFO>");
    
	CXmlNodeWrapper rootnode(xmlDoc.AsNode());   
    
	CXmlNodeWrapper node(rootnode.InsertNode(0,"BODY"));
	GetDlgItemText(IDC_RICHEDITBODY,strtext);
	node.SetText(strtext.Trim());
	CXmlNodeWrapper node_a(rootnode.InsertNode(1,"OPTIONA"));
	GetDlgItemText(IDC_EDIT1,strtext);
	node_a.SetText(strtext.Trim());
	CXmlNodeWrapper node_b(rootnode.InsertNode(2,"OPTIONB"));
	GetDlgItemText(IDC_EDIT2,strtext);
	node_b.SetText(strtext.Trim());
	CXmlNodeWrapper node_c(rootnode.InsertNode(3,"OPTIONC"));
	GetDlgItemText(IDC_EDIT3,strtext);
	node_c.SetText(strtext.Trim());
	CXmlNodeWrapper node_d(rootnode.InsertNode(4,"OPTIOND"));
	GetDlgItemText(IDC_EDIT4,strtext);
	node_d.SetText(strtext.Trim());
	CXmlNodeWrapper node_key(rootnode.InsertNode(5,"KEY"));
	GetDlgItemText(IDC_EDIT5,strtext);
	node_key.SetText(strtext.Trim());
	DoMsgSend(MSG_MANAGE,MSG_QUESTION_SAVE,(char *)(LPCTSTR)xmlDoc.GetXML(),xmlDoc.GetXML().GetLength()+1);
}
BOOL CDLGEDIT::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
