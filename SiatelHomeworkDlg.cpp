/* This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelHomeworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SiatelHomework.h"
#include "SiatelHomeworkDlg.h"
#include "SiatelHomeworkExt.h"
#include "SiatelFileListDlg.h"
#include "FileDialogST.h"
#include "VersionInfo.h"
#include "HyperlinkStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();

protected:
	CStatic m_ctrlVersion;
	CEdit m_ctrlWarning;
	CVersionInfo m_pVersionInfo;
	CHyperlinkStatic m_ctrlWebsite;
	CHyperlinkStatic m_ctrlEmail;

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION, m_ctrlVersion);
	DDX_Control(pDX, IDC_WARNING, m_ctrlWarning);
	DDX_Control(pDX, IDC_WEBSITE, m_ctrlWebsite);
	DDX_Control(pDX, IDC_EMAIL, m_ctrlEmail);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_pVersionInfo.Load(AfxGetApp()->m_pszExeName))
	{
		CString strName = m_pVersionInfo.GetProductName();
		CString strVersion = m_pVersionInfo.GetProductVersionAsString();
		strVersion.Replace(_T(" "), _T(""));
		strVersion.Replace(_T(","), _T("."));
		const int nFirst = strVersion.Find(_T('.'));
		const int nSecond = strVersion.Find(_T('.'), nFirst + 1);
		strVersion.Truncate(nSecond);
		m_ctrlVersion.SetWindowText(strName + _T(" version ") + strVersion);
	}

	m_ctrlWarning.SetWindowText(_T("This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>."));

	m_ctrlWebsite.SetHyperlink(_T("http://www.emvs.site/"));
	m_ctrlEmail.SetHyperlink(_T("mailto:contact@emvs.site"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

// CSiatelHomeworkDlg dialog

CSiatelHomeworkDlg::CSiatelHomeworkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSiatelHomeworkDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_treeNetwork = NULL; m_hashNetwork = NULL;
}

void CSiatelHomeworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FOLDER, m_ctrlFolder);
	DDX_Control(pDX, IDC_INPUT_EDIT, m_ctrlInputFile);
	DDX_Control(pDX, IDC_OUTPUT_EDIT, m_ctrlOutputFile);
	DDX_Control(pDX, IDC_INPUT_CODE, m_ctrlInputCode);
	DDX_Control(pDX, IDC_OUTPUT_CODE, m_ctrlOutputCode);
	DDX_Control(pDX, IDC_DELETE_CODE, m_ctrlDeleteCode);
	DDX_Control(pDX, IDC_STATUS, m_ctrlMessage);
	DDX_Control(pDX, IDC_PROGRESS, m_ctrlProgress);
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Control(pDX, IDC_UPLOAD, m_ctrlUpload);
	DDX_Control(pDX, IDC_DOWNLOAD, m_ctrlDownload);
	DDX_Control(pDX, IDC_DELETE, m_ctrlDelete);
}

BEGIN_MESSAGE_MAP(CSiatelHomeworkDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BROWSE, &CSiatelHomeworkDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_INPUT_FILE, &CSiatelHomeworkDlg::OnBnClickedInputFile)
	ON_BN_CLICKED(IDC_OUTPUT_FILE, &CSiatelHomeworkDlg::OnBnClickedOutputFile)
	ON_BN_CLICKED(IDC_UPLOAD, &CSiatelHomeworkDlg::OnBnClickedUpload)
	ON_BN_CLICKED(IDC_DOWNLOAD, &CSiatelHomeworkDlg::OnBnClickedDownload)
	ON_BN_CLICKED(IDC_DELETE, &CSiatelHomeworkDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_FILE_LIST, &CSiatelHomeworkDlg::OnBnClickedFileList)
	ON_EN_CHANGE(IDC_INPUT_CODE, &CSiatelHomeworkDlg::OnEnChangeInputCode)
	ON_EN_CHANGE(IDC_DELETE_CODE, &CSiatelHomeworkDlg::OnEnChangeDeleteCode)
END_MESSAGE_MAP()


// CSiatelHomeworkDlg message handlers

BOOL CSiatelHomeworkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	RECT rectWindow;
	GetWindowRect( &rectWindow );
	TRACE(_T("CSiatelHomeworkDlg: Width = %d Height = %d\n"),
		rectWindow.right - rectWindow.left,
		rectWindow.bottom - rectWindow.top );

	// set up window's terminal fixed font
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	logfont.lfCharSet = OEM_CHARSET;
	logfont.lfPitchAndFamily = FIXED_PITCH;
	logfont.lfQuality = ANTIALIASED_QUALITY;
	logfont.lfOutPrecision = OUT_TT_PRECIS;
	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = 16;
	_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("Arial"));
	VERIFY(m_fontCourier.CreateFontIndirect(&logfont));

	m_ctrlFolder.SetFont(&m_fontCourier);
	m_ctrlInputFile.SetFont(&m_fontCourier);
	m_ctrlOutputFile.SetFont(&m_fontCourier);
	m_ctrlInputCode.SetFont(&m_fontCourier);
	m_ctrlOutputCode.SetFont(&m_fontCourier);
	m_ctrlDeleteCode.SetFont(&m_fontCourier);
	m_ctrlMessage.SetFont(&m_fontCourier);

	m_ctrlInputCode.SetLimitText(MAX_CODE_LENGTH);
	m_ctrlOutputCode.SetLimitText(MAX_CODE_LENGTH);
	m_ctrlDeleteCode.SetLimitText(MAX_CODE_LENGTH);

	m_ctrlFileList.EnableWindow(FALSE);
	m_ctrlUpload.EnableWindow(FALSE);
	m_ctrlDownload.EnableWindow(FALSE);
	m_ctrlDelete.EnableWindow(FALSE);

	m_hashNetwork = new CNetworkHash();
	m_treeNetwork = new CNetworkTree();

	m_ctrlProgress.ShowWindow(SW_HIDE);
	m_funcProgress = (LPPROGRESS_ROUTINE)(&CSiatelHomeworkDlg::ProgressFunc);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSiatelHomeworkDlg::OnDestroy()
{
	CDialog::OnDestroy();

	VERIFY(m_fontCourier.DeleteObject());

	if (USE_HASH_METHOD)
	{
		ASSERT(m_hashNetwork != NULL);
		m_hashNetwork->ExportData();
	}
	else
	{
		ASSERT(m_treeNetwork != NULL);
		m_treeNetwork->ExportData();
	}

	if (m_treeNetwork != NULL)
	{
		delete m_treeNetwork;
		m_treeNetwork = NULL;
	}

	if (m_hashNetwork != NULL)
	{
		delete m_hashNetwork;
		m_hashNetwork = NULL;
	}
}

void CSiatelHomeworkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSiatelHomeworkDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSiatelHomeworkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSiatelHomeworkDlg::OnBnClickedBrowse()
{
	const UINT nFolderFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_VALIDATE;

	CFileDialogST dlgFileDialog;
	if (dlgFileDialog.SelectFolder(NULL, m_strRootFolder, nFolderFlags, this) == IDOK)
	{
		m_strRootFolder = dlgFileDialog.GetSelectedFolder();
		const UINT lenRootFolder = m_strRootFolder.GetLength();
		if (lenRootFolder > 0)
		{
			if (m_strRootFolder.GetAt(lenRootFolder - 1) != _T('\\'))
				m_strRootFolder += _T("\\");
			m_ctrlFolder.SetWindowText(m_strRootFolder);
			m_strFileList.Format(_T("%sfilelist.txt"), static_cast<LPCWSTR>(m_strRootFolder));
			if (USE_HASH_METHOD)
			{
				ASSERT(m_hashNetwork != NULL);
				m_hashNetwork->RemoveAll();
				m_hashNetwork->SetFileList(m_strFileList);
				m_hashNetwork->SetRootFolder(m_strRootFolder);
				m_hashNetwork->ImportData();
			}
			else
			{
				ASSERT(m_treeNetwork != NULL);
				m_treeNetwork->DeleteTree();
				m_treeNetwork->SetFileList(m_strFileList);
				m_treeNetwork->SetRootFolder(m_strRootFolder);
				m_treeNetwork->ImportData();
			}
			m_ctrlMessage.SetWindowText(READY_MESSAGE);
			m_ctrlFileList.EnableWindow(TRUE);
		}
	}
}

void CSiatelHomeworkDlg::OnBnClickedFileList()
{
	CSiatelFileListDlg dlgSiatelFileList(this);
	dlgSiatelFileList.SetRootFolder(m_strRootFolder);
	dlgSiatelFileList.SetNetworkTree(m_treeNetwork);
	dlgSiatelFileList.SetNetworkHash(m_hashNetwork);
	dlgSiatelFileList.DoModal();
}

void CSiatelHomeworkDlg::OnBnClickedInputFile()
{
	CFileDialogST pFileDialog(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
		_T("All files\0*.*\0"), this);
	if (pFileDialog.DoModal() == IDOK)
	{
		m_strInputFile = pFileDialog.GetPathName();
		m_ctrlInputFile.SetWindowText(m_strInputFile);
		m_ctrlUpload.EnableWindow(!m_strInputFile.IsEmpty());
	}
}

void CSiatelHomeworkDlg::OnBnClickedUpload()
{
	BOOL bCancelOperation = FALSE;
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;
	m_ctrlFolder.GetWindowText(m_strRootFolder);
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK);
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != NULL);
			int nFileCode = m_hashNetwork->GenerateID();
			int nPathCode = m_hashNetwork->GetSize();
			CString strFileCode = m_hashNetwork->EncodeNetworkID(nFileCode);
			CString strFilePath = m_hashNetwork->GetFilePath(nPathCode, arrNetworkPath);
			VERIFY(m_hashNetwork->CreateNetworkPath(m_strRootFolder, arrNetworkPath));
			strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath));
			m_ctrlProgress.SetRange32(0, PROGRESS_RANGE);
			m_ctrlProgress.SetPos(0);
			m_ctrlProgress.ShowWindow(SW_SHOW);
			if (!CopyFileEx(m_strInputFile, strFilename, m_funcProgress, &m_ctrlProgress, &bCancelOperation, 0 ))
			{
				m_ctrlProgress.ShowWindow(SW_HIDE);
				const DWORD dwLastError = GetLastError();
				m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError));
			}
			else
			{
				VERIFY(m_hashNetwork->InsertItem(nFileCode, nPathCode, strFilePath));
				strMessage.Format(_T("Done writing file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(strFileCode));
				m_ctrlMessage.SetWindowText(strMessage);
				m_ctrlOutputCode.SetWindowText(strFileCode);
				m_ctrlProgress.SetPos(PROGRESS_RANGE-1);
				m_ctrlProgress.UpdateWindow();
				Sleep(SLEEP_INTERVAL);
				m_ctrlProgress.ShowWindow(SW_HIDE);
			}
		}
		else // USE B-TREE
		{
		}
	}
}

void CSiatelHomeworkDlg::OnBnClickedOutputFile()
{
	CFileDialogST pFileDialog(FALSE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_LONGNAMES | OFN_PATHMUSTEXIST,
		_T("All files\0*.*\0"), this);
	if (pFileDialog.DoModal() == IDOK)
	{
		m_strOutputFile = pFileDialog.GetPathName();
		m_ctrlOutputFile.SetWindowText(m_strOutputFile);
		m_ctrlDownload.EnableWindow(!m_strOutputFile.IsEmpty() && !m_strInputCode.IsEmpty());
	}
}

void CSiatelHomeworkDlg::OnBnClickedDownload()
{
	int nLevel = 0;
	BOOL bCancelOperation = FALSE;
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;
	m_ctrlFolder.GetWindowText(m_strRootFolder);
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK);
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != NULL);
			if (!m_hashNetwork->IsValidCode(m_strInputCode))
			{
				m_ctrlMessage.SetWindowText(INVALID_CHARS);
			}
			else
			{
				CString strFilePath; // relative path to file 
				int nFileCode = m_hashNetwork->DecodeNetworkID(m_strInputCode);
				if (!m_hashNetwork->SearchItem(nFileCode, nLevel, strFilePath))
				{
					m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS);
				}
				else
				{
					strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath));
					m_ctrlProgress.SetRange32(0, PROGRESS_RANGE);
					m_ctrlProgress.SetPos(0);
					m_ctrlProgress.ShowWindow(SW_SHOW);
					if (!CopyFileEx(strFilename, m_strOutputFile, m_funcProgress, &m_ctrlProgress, &bCancelOperation, 0 ))
					{
						m_ctrlProgress.ShowWindow(SW_HIDE);
						const DWORD dwLastError = GetLastError();
						if (!m_hashNetwork->FileExists(strFilename))
							m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS);
						else
							m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError));
					}
					else
					{
						strMessage.Format(_T("Done reading file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(m_strInputCode));
						m_ctrlMessage.SetWindowText(strMessage);
						m_ctrlProgress.SetPos(PROGRESS_RANGE-1);
						m_ctrlProgress.UpdateWindow();
						Sleep(SLEEP_INTERVAL);
						m_ctrlProgress.ShowWindow(SW_HIDE);
					}
				}
			}
		}
		else // USE B-TREE
		{
		}
	}
}

void CSiatelHomeworkDlg::OnBnClickedDelete()
{
	int nLevel = 0;
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;
	m_ctrlFolder.GetWindowText(m_strRootFolder);
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK);
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != NULL);
			if (!m_hashNetwork->IsValidCode(m_strDeleteCode))
			{
				m_ctrlMessage.SetWindowText(INVALID_CHARS);
			}
			else
			{
				CString strFilePath;
				int nFileCode = m_hashNetwork->DecodeNetworkID(m_strDeleteCode);
				if (!m_hashNetwork->SearchItem(nFileCode, nLevel, strFilePath))
				{
					m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS);
				}
				else
				{
					strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath));
					if (!m_hashNetwork->FileExists(strFilename))
					{
						m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS);
					}
					else
					{
						if (!DeleteFile(strFilename))
						{
							const DWORD dwLastError = GetLastError();
							m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError));
						}
						else
						{
							VERIFY(m_hashNetwork->DeleteItem(nFileCode));
							strMessage.Format(_T("File %s [%s] has been deleted ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(m_strDeleteCode));
							m_ctrlMessage.SetWindowText(strMessage);
						}
					}
				}
			}
		}
		else // USE B-TREE
		{
		}
	}
}

void CSiatelHomeworkDlg::OnEnChangeInputCode()
{
	m_ctrlInputCode.GetWindowText(m_strInputCode);
	m_ctrlDownload.EnableWindow(!m_strOutputFile.IsEmpty() && !m_strInputCode.IsEmpty());
}

void CSiatelHomeworkDlg::OnEnChangeDeleteCode()
{
	m_ctrlDeleteCode.GetWindowText(m_strDeleteCode);
	m_ctrlDelete.EnableWindow(!m_strDeleteCode.IsEmpty());
}

DWORD CALLBACK CSiatelHomeworkDlg::ProgressFunc(
		LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred,
		LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred,
		DWORD dwStreamNumber,
		DWORD dwCallbackReason,
		HANDLE hSourceFile,
		HANDLE hDestinationFile,
		LPVOID lpData)
{
	UNREFERENCED_PARAMETER(StreamSize);
	UNREFERENCED_PARAMETER(StreamBytesTransferred);
	UNREFERENCED_PARAMETER(dwStreamNumber);
	UNREFERENCED_PARAMETER(dwCallbackReason);
	UNREFERENCED_PARAMETER(hSourceFile);
	UNREFERENCED_PARAMETER(hDestinationFile);

	ASSERT(lpData != NULL);
	float nFileSize = (float)TotalFileSize.QuadPart;
	float nTransferred = (float)TotalBytesTransferred.QuadPart;
	if (TotalFileSize.QuadPart != 0)
	{
		int nPosition = (int)(nTransferred / nFileSize * (float)PROGRESS_RANGE);
		((CProgressCtrl*)lpData)->SetPos(nPosition);
		((CProgressCtrl*)lpData)->UpdateWindow();
	}
	return PROGRESS_CONTINUE;
}
