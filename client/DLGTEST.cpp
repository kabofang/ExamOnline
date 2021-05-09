// DLGTEST.cpp : implementation file
//

#include "stdafx.h"
#include "client.h"
#include "DLGTEST.h"
#include ".\dlgtest.h"
#include "msgdispatcher.h"
#include "./xml/XmlNodeWrapper.h"


// CDLGTEST dialog

IMPLEMENT_DYNAMIC(CDLGTEST, CDialog)
CDLGTEST::CDLGTEST(CWnd* pParent /*=NULL*/)
	: CDialog(CDLGTEST::IDD, pParent)
{
	pqhead = NULL;
}

CDLGTEST::~CDLGTEST()
{
}

void CDLGTEST::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDLGTEST, CDialog)
ON_MESSAGE(WM_LOADTEST, OnLoadTest)
ON_BN_CLICKED(IDC_BUTTONUP, OnBnClickedButtonup)
ON_BN_CLICKED(IDC_BUTTONDOWN, OnBnClickedButtondown)
ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()

void CDLGTEST::ParseNode(IDispatch *pNode,QUESTION **pqhead)
{
	CXmlNodeWrapper root(pNode);
	CXmlNodeWrapper node_question;
	CXmlNodeWrapper node;
	QUESTION *pitem;
	CString str;
	CString str_text;
		
	for (int i = 0; i < root.NumNodes(); i++)
	{
		node_question = root.GetNode(i);
		pitem = new QUESTION;
		pitem->id = atoi((char *)(LPCTSTR)node_question.GetValue("id"));
		for (int j = 0;j< node_question.NumNodes();j++)
		{
			node = node_question.GetNode(j);
			str = node.Name();
			str_text = node.GetText();
			if (str == "BODY")
			{
				pitem->body = new char[str_text.GetLength()+1];
				memset(pitem->body,0,str_text.GetLength()+1);
				memcpy(pitem->body,(char *)(LPCTSTR)str_text,strlen(str_text));
			}
			if (str == "OPTIONA")
			{
				pitem->optiona = new char[str_text.GetLength()+1];
				memset(pitem->optiona,0,str_text.GetLength()+1);
				memcpy(pitem->optiona,(char *)(LPCTSTR)str_text,strlen(str_text));
			}
			if (str == "OPTIONB")
			{
				pitem->optionb = new char[str_text.GetLength()+1];
				memset(pitem->optionb,0,str_text.GetLength()+1);
				memcpy(pitem->optionb,(char *)(LPCTSTR)str_text,strlen(str_text));
			}
			if (str == "OPTIONC")
			{
				pitem->optionc = new char[str_text.GetLength()+1];
				memset(pitem->optionc,0,str_text.GetLength()+1);
				memcpy(pitem->optionc,(char *)(LPCTSTR)str_text,strlen(str_text));
			}
			if (str == "OPTIOND")
			{
				pitem->optiond = new char[str_text.GetLength()+1];
				memset(pitem->optiond,0,str_text.GetLength()+1);
				memcpy(pitem->optiond,(char *)(LPCTSTR)str_text,strlen(str_text));
			}
		}

		pitem->next = *pqhead;
		*pqhead = pitem;
		
	}
}

LRESULT CDLGTEST::OnLoadTest(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
	  return 0;
	char *pstrxml = (char *)wParam;
    CXmlDocumentWrapper xmlDoc;//生成XML文档处理对象
		
	if (xmlDoc.LoadXML(pstrxml) == false)
	{
		if (pstrxml)
			delete pstrxml;
		return false;
	}
	
	ParseNode(xmlDoc.AsNode(),&pqhead);	

	CString strnum;
	strnum.Format("%d",pqhead->id);
	SetDlgItemText(IDC_STATIC_NUM,strnum);
	SetDlgItemText(IDC_EDITBODY,pqhead->body);
	SetDlgItemText(IDC_EDITA,pqhead->optiona);
	SetDlgItemText(IDC_EDITB,pqhead->optionb);
	SetDlgItemText(IDC_EDITC,pqhead->optionc);
	SetDlgItemText(IDC_EDITD,pqhead->optiond);
	return 0;
}

// CDLGTEST message handlers

BOOL CDLGTEST::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CMSGDISPATCHER::h_dlgtest = GetSafeHwnd();
	DoMsgSend(MSG_MANAGE,MSG_TEST_LOAD,NULL,0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDLGTEST::OnBnClickedButtonup()
{
	// TODO: Add your control notification handler code here
	CString strnum;
	CString stranswer;
	int num;
	GetDlgItemText(IDC_STATIC_NUM,strnum);
	num = atoi((char *)(LPCTSTR)strnum);
	QUESTION *pitem = pqhead;
	QUESTION *pprev = pitem;
	while(pitem != NULL)
	{
		if (pitem->id == num)
			break;
		pprev = pitem;
		pitem = pitem->next;
	}
		
	if (pitem != NULL)
	{
		strnum.Format("%d",pprev->id);
		SetDlgItemText(IDC_STATIC_NUM,strnum);
		SetDlgItemText(IDC_EDITBODY,pprev->body);
		SetDlgItemText(IDC_EDITA,pprev->optiona);
		SetDlgItemText(IDC_EDITB,pprev->optionb);
		SetDlgItemText(IDC_EDITC,pprev->optionc);
		SetDlgItemText(IDC_EDITD,pprev->optiond);
		GetDlgItemText(IDC_EDITKEY,stranswer);
		stranswer.TrimLeft();
		stranswer.TrimRight();
		stranswer.MakeUpper();
		if (stranswer != "")
		{
			if (stranswer == "A")
				pitem->key = 1;
			if (stranswer == "B")
				pitem->key = 2;
			if (stranswer == "C")
				pitem->key = 3;
			if (stranswer == "D")
				pitem->key = 4;
		}
		
	}
}

void CDLGTEST::OnBnClickedButtondown()
{
	// TODO: Add your control notification handler code here
	CString strnum;
	CString stranswer;
	int num;
	GetDlgItemText(IDC_STATIC_NUM,strnum);
	num = atoi((char *)(LPCTSTR)strnum);
	QUESTION *pitem = pqhead;
	QUESTION *pnext = NULL;
	while(pitem != NULL)
	{
		if (pitem->id == num)
			break;
		pitem = pitem->next;
	}
		
	if (pitem != NULL)
	{
		pnext = pitem->next;
		GetDlgItemText(IDC_EDITKEY,stranswer);
		stranswer.TrimLeft();
		stranswer.TrimRight();
		stranswer.MakeUpper();
		if (stranswer == "A")
			pitem->key = 1;
		if (stranswer == "B")
			pitem->key = 2;
		if (stranswer == "C")
			pitem->key = 3;
		if (stranswer == "D")
			pitem->key = 4;
	}
	
	if (pnext != NULL)
	{
		strnum.Format("%d",pnext->id);
		SetDlgItemText(IDC_STATIC_NUM,strnum);
		SetDlgItemText(IDC_EDITBODY,pnext->body);
		SetDlgItemText(IDC_EDITA,pnext->optiona);
		SetDlgItemText(IDC_EDITB,pnext->optionb);
		SetDlgItemText(IDC_EDITC,pnext->optionc);
		SetDlgItemText(IDC_EDITD,pnext->optiond);
	}
}
void CDLGTEST::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	QUESTION *pitem = pqhead;
	ANSWER *panswer = NULL;
	int count = 0;
	CString strnum;
	CString stranswer;
	int num;
	GetDlgItemText(IDC_STATIC_NUM,strnum);
	num = atoi((char *)(LPCTSTR)strnum);
	while (pitem)
	{
		if (pitem->id == num)
		{
			GetDlgItemText(IDC_EDITKEY,stranswer);
			stranswer.TrimLeft();
			stranswer.TrimRight();
			stranswer.MakeUpper();
			if (stranswer == "A")
				pitem->key = 1;
			if (stranswer == "B")
				pitem->key = 2;
			if (stranswer == "C")
				pitem->key = 3;
			if (stranswer == "D")
				pitem->key = 4;
		}
		count++;
		pitem = pitem->next;
	}
	panswer = new ANSWER[count];
	memset(panswer,0,sizeof(ANSWER)*count);
	pitem = pqhead;
	int index = 0;
	while (pitem)
	{
		panswer[index].no = pitem->id;
		panswer[index].key = pitem->key;
		index++;
		pitem = pitem->next;
	}

	DoMsgSend(MSG_MANAGE,MSG_TEST_COMMIT,(char *)panswer,sizeof(ANSWER)*count);
	delete []panswer;
}
