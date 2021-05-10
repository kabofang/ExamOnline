// experiment.cpp : ����Ӧ�ó��������Ϊ��
//
extern "C"
{
#include <openssl/applink.c>
};
#include "stdafx.h"
#include "experiment.h"
#include "experimentDlg.h"
#include "./netlib/message.h"
#include "msgdispatcher.h"
#include "./experiment.h"
#include "./database/database.h"
#include "tempvar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CexperimentApp

BEGIN_MESSAGE_MAP(CexperimentApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

Config cfg(NULL,recv_proc);
S_MSGLST g_msglst;//��Ϣ����ṹȫ�ֱ���
// CexperimentApp ����

bool LoadFromDb()
{
	CDATABASE db(STR_DB_PATH,STR_DB_ADMIN,STR_DB_PASSWD,"");//���ݿ�
		
	return true;
}

void init_threads()//��ʼ�������߳�
{
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)recvmsg,
								&cfg,0,NULL);//������Ϣ�����߳�
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)msgdispatcher,
								NULL,0,NULL);//������Ϣ�ַ��߳�
}

void exit_threads()//�˳������߳�
{
	//��ֹ�߳�recvmsg��ȷ���߳������˳�
	cfg.SetEndflg(true);//��Ϊ�棬�ȴ������̸߳�λ�ñ�־
	bool is_wait = true;
	while (is_wait)
	{
		if (cfg.GetEndflg() == false)//�����߳��Ѹ�λ
			is_wait = false;
	}
	
	is_wait = true;
	g_msglst.set_endflg(true);
	while (is_wait)
	{
		if (!g_msglst.get_endflg())//�ȴ����ն˶�end_flg��λ
			is_wait = false;
	}

}//
CexperimentApp::CexperimentApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CexperimentApp ����

CexperimentApp theApp;


// CexperimentApp ��ʼ��

BOOL CexperimentApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControls()�����򣬽��޷��������ڡ�
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	CoInitialize(NULL);
	LoadFromDb();//���ݿ��ʼ��
	init_threads();//��ʼ���߳�
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CExperimentDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȷ�������ر�
		//�Ի���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȡ�������ر�
		//�Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	// ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

int CexperimentApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	CoUninitialize();
	exit_threads();
	delete Key;
	return CWinApp::ExitInstance();
}
