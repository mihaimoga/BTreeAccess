/* This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelHomework.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SiatelHomework.h"
#include "SiatelHomeworkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSiatelHomeworkApp

BEGIN_MESSAGE_MAP(CSiatelHomeworkApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CSiatelHomeworkApp construction

CSiatelHomeworkApp::CSiatelHomeworkApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CSiatelHomeworkApp object

CSiatelHomeworkApp theApp;

// CSiatelHomeworkApp initialization

BOOL CSiatelHomeworkApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("MihaiMoga"));

	delete AfxGetApp()->m_pszExeName;
	AfxGetApp()->m_pszExeName = new TCHAR[_MAX_PATH];

	// delete AfxGetApp()->m_pszProfileName;
	// AfxGetApp()->m_pszProfileName = new TCHAR[_MAX_PATH];

	TCHAR lpszDrive[_MAX_DRIVE];
	TCHAR lpszDirectory[_MAX_DIR];
	TCHAR lpszFilename[_MAX_FNAME];
	TCHAR lpszExtension[_MAX_EXT];

	LPTSTR lpszHelpPath = (LPTSTR) AfxGetApp()->m_pszHelpFilePath;
	LPTSTR lpszExePath = (LPTSTR) AfxGetApp()->m_pszExeName;
	// LPTSTR lpszIniPath = (LPTSTR) AfxGetApp()->m_pszProfileName;

	VERIFY(0 == _tsplitpath_s(lpszHelpPath, lpszDrive, _MAX_DRIVE, lpszDirectory, _MAX_DIR, lpszFilename, _MAX_FNAME, lpszExtension, _MAX_EXT));
	VERIFY(0 == _tmakepath_s(lpszExePath, _MAX_PATH, lpszDrive, lpszDirectory, lpszFilename, _T(".exe")));
	// VERIFY(0 == _tmakepath_s(lpszIniPath, _MAX_PATH, lpszDrive, lpszDirectory, lpszFilename, _T(".ini")));

	srand((unsigned)time(NULL));

	CSiatelHomeworkDlg dlgSiatelHomework;
	m_pMainWnd = &dlgSiatelHomework;
	INT_PTR nResponse = dlgSiatelHomework.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}