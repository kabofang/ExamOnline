// experiment.cpp : 定义应用程序的类行为。
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
S_MSGLST g_msglst;//消息管理结构全局变量
// CexperimentApp 构造

bool LoadFromDb()
{
	CDATABASE db(STR_DB_PATH,STR_DB_ADMIN,STR_DB_PASSWD,"");//数据库
		
	return true;
}

void init_threads()//初始化所有线程
{
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)recvmsg,
								&cfg,0,NULL);//创建消息接收线程
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)msgdispatcher,
								NULL,0,NULL);//创建消息分发线程
}

void exit_threads()//退出所有线程
{
	//终止线程recvmsg，确保线程正常退出
	cfg.SetEndflg(true);//置为真，等待接收线程复位该标志
	bool is_wait = true;
	while (is_wait)
	{
		if (cfg.GetEndflg() == false)//接收线程已复位
			is_wait = false;
	}
	
	is_wait = true;
	g_msglst.set_endflg(true);
	while (is_wait)
	{
		if (!g_msglst.get_endflg())//等待接收端对end_flg复位
			is_wait = false;
	}

}//
CexperimentApp::CexperimentApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CexperimentApp 对象

CexperimentApp theApp;


// CexperimentApp 初始化

BOOL CexperimentApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	CoInitialize(NULL);
	LoadFromDb();//数据库初始化
	init_threads();//初始化线程
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CExperimentDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用“确定”来关闭
		//对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用“取消”来关闭
		//对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	// 而不是启动应用程序的消息泵。
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
