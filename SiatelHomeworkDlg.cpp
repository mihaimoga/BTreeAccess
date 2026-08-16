/* Copyright (C) 2014-2026 Stefan-Mihai MOGA
This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

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

#include "VersionInfo.h"
#include "HLinkCtrl.h"
#include "FolderDlg.h"

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
	CHLinkCtrl m_ctrlWebsite;
	CHLinkCtrl m_ctrlEmail;

	DECLARE_MESSAGE_MAP()
};

/**
 * @brief Constructs the About dialog.
 */
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

/**
 * @brief Performs DDX/DDV data exchange for the About dialog controls.
 * @param pDX Pointer to the CDataExchange object.
 */
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION, m_ctrlVersion); // version label
	DDX_Control(pDX, IDC_WARNING, m_ctrlWarning); // read-only GPL license text
	DDX_Control(pDX, IDC_WEBSITE, m_ctrlWebsite); // clickable website hyperlink
	DDX_Control(pDX, IDC_EMAIL, m_ctrlEmail);     // clickable e-mail hyperlink
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/**
 * @brief Retrieves the fully qualified path of the current executable module.
 * @param pdwLastError Optional pointer that receives the last Win32 error code.
 *        Set to ERROR_SUCCESS on success, or the failing error code on failure.
 * @return The full path of the module as a CString, or an empty CString on failure.
 */
CString GetModuleFileName(_Inout_opt_ DWORD* pdwLastError = nullptr)
{
	CString strModuleFileName;
	DWORD dwSize{ _MAX_PATH }; // start with a typical path-length buffer
	while (true)
	{
		TCHAR* pszModuleFileName{ strModuleFileName.GetBuffer(dwSize) }; // allocate/expand the internal buffer
		const DWORD dwResult{ ::GetModuleFileName(nullptr, pszModuleFileName, dwSize) }; // query the exe path
		if (dwResult == 0)
		{
			// Win32 call failed entirely; propagate the error code and return empty
			if (pdwLastError != nullptr)
				*pdwLastError = GetLastError();
			strModuleFileName.ReleaseBuffer(0);
			return CString{};
		}
		else if (dwResult < dwSize)
		{
			// Success: the path fit inside the buffer; commit the exact character count
			if (pdwLastError != nullptr)
				*pdwLastError = ERROR_SUCCESS;
			strModuleFileName.ReleaseBuffer(dwResult);
			return strModuleFileName;
		}
		else if (dwResult == dwSize)
		{
			// Buffer was too small; discard contents and double the size before retrying
			strModuleFileName.ReleaseBuffer(0);
			dwSize *= 2;
		}
	}
}

/**
 * @brief Initializes the About dialog; populates the version label and sets hyperlinks.
 * @return TRUE to set input focus to the first control, FALSE otherwise.
 */
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strFullPath{ GetModuleFileName() }; // resolve the full exe path for version-info lookup
	if (strFullPath.IsEmpty())               // bail out if the path could not be determined
#pragma warning(suppress: 26487)
		return FALSE;

	if (m_pVersionInfo.Load(strFullPath.GetString())) // parse the VERSIONINFO resource from the exe
	{
		CString strName = m_pVersionInfo.GetProductName().c_str();
		CString strVersion = m_pVersionInfo.GetProductVersionAsString().c_str();
		strVersion.Replace(_T(" "), _T(""));   // strip spaces introduced by the version-string formatter
		strVersion.Replace(_T(","), _T("."));  // normalise comma separators to dots (e.g. "1,2" -> "1.2")
		const int nFirst = strVersion.Find(_T('.'));           // position of the first dot (major/minor boundary)
		const int nSecond = strVersion.Find(_T('.'), nFirst + 1); // position of the second dot (minor/patch boundary)
		strVersion.Truncate(nSecond);           // keep only major.minor, discard patch and build components
		if (nSecond == (nFirst + 2))            // single-digit minor version (e.g. "1.9") needs zero-padding
			strVersion.Insert(nFirst + 1, _T("0")); // pad to two digits (e.g. "1.9" -> "1.09")
#if _WIN32 || _WIN64
#if _WIN64
		m_ctrlVersion.SetWindowText(strName + _T(" version ") + strVersion + _T(" (64-bit)"));
#else
		m_ctrlVersion.SetWindowText(strName + _T(" version ") + strVersion + _T(" (32-bit)"));
#endif
#endif
	}

	// populate the license notice text box with the standard GPL-3.0 boilerplate
	m_ctrlWarning.SetWindowText(_T("This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>."));

	m_ctrlWebsite.SetHyperLink(_T("https://www.moga.doctor/")); // author's personal website
	m_ctrlEmail.SetHyperLink(_T("mailto:stefan-mihai@moga.doctor")); // author's contact e-mail

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**
 * @brief Handles WM_DESTROY for the About dialog.
 */
void CAboutDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

// CSiatelHomeworkDlg dialog

/**
 * @brief Constructs the main application dialog.
 * @param pParent Pointer to the parent window. Defaults to NULL.
 */
CSiatelHomeworkDlg::CSiatelHomeworkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSiatelHomeworkDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // load the application icon for use in OnPaint / OnQueryDragIcon
	m_treeNetwork = nullptr; m_hashNetwork = nullptr; // both backends start uninitialised; allocated in OnInitDialog
}

/**
 * @brief Performs DDX/DDV data exchange between dialog controls and member variables.
 * @param pDX Pointer to the CDataExchange object.
 */
void CSiatelHomeworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FOLDER,      m_ctrlFolder);     // root network folder path display
	DDX_Control(pDX, IDC_INPUT_EDIT,  m_ctrlInputFile);  // source file path for upload
	DDX_Control(pDX, IDC_OUTPUT_EDIT, m_ctrlOutputFile); // destination file path for download
	DDX_Control(pDX, IDC_INPUT_CODE,  m_ctrlInputCode);  // network code entered by user for download
	DDX_Control(pDX, IDC_OUTPUT_CODE, m_ctrlOutputCode); // network code generated after a successful upload
	DDX_Control(pDX, IDC_DELETE_CODE, m_ctrlDeleteCode); // network code entered by user for deletion
	DDX_Control(pDX, IDC_STATUS,      m_ctrlMessage);    // status / error message label
	DDX_Control(pDX, IDC_PROGRESS,    m_ctrlProgress);   // progress bar shown during file copy
	DDX_Control(pDX, IDC_FILE_LIST,   m_ctrlFileList);   // button to open the file list dialog
	DDX_Control(pDX, IDC_UPLOAD,      m_ctrlUpload);     // button to upload a file
	DDX_Control(pDX, IDC_DOWNLOAD,    m_ctrlDownload);   // button to download a file
	DDX_Control(pDX, IDC_DELETE,      m_ctrlDelete);     // button to delete a file
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

/**
 * @brief Initializes the main dialog; sets up fonts, controls, and network data structures.
 * @return TRUE to set input focus to the first control, FALSE otherwise.
 */
BOOL CSiatelHomeworkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE); // retrieve a copy of the system (title-bar) menu
	if (pSysMenu != nullptr)
	{
		bool bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX); // load the "About" string from the resource table
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);                    // visual divider before About item
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu); // "About BTreeAccess..."
		}
		pSysMenu->AppendMenu(MF_SEPARATOR);                        // divider before social-media links
		pSysMenu->AppendMenu(MF_STRING, IDM_TWITTER,     _T("Twitter"));
		pSysMenu->AppendMenu(MF_STRING, IDM_LINKEDIN,    _T("LinkedIn"));
		pSysMenu->AppendMenu(MF_STRING, IDM_FACEBOOK,    _T("Facebook"));
		pSysMenu->AppendMenu(MF_STRING, IDM_INSTAGRAM,   _T("Instagram"));
		pSysMenu->AppendMenu(MF_SEPARATOR);                        // divider before GitHub project links
		pSysMenu->AppendMenu(MF_STRING, IDM_ISSUES,      _T("Issues"));
		pSysMenu->AppendMenu(MF_STRING, IDM_DISCUSSIONS, _T("Discussions"));
		pSysMenu->AppendMenu(MF_STRING, IDM_WIKI,        _T("Wiki"));
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
	memset(&logfont, 0, sizeof(logfont)); // zero-initialise all fields before setting individual ones
	logfont.lfCharSet = OEM_CHARSET;
	logfont.lfPitchAndFamily = FIXED_PITCH;        // request a fixed-pitch (monospaced) face
	logfont.lfQuality = ANTIALIASED_QUALITY;       // smooth rendering on-screen
	logfont.lfOutPrecision = OUT_TT_PRECIS;        // prefer TrueType outlines
	logfont.lfWeight = FW_NORMAL;
	logfont.lfHeight = 16;                         // logical units; matches a comfortable reading size
	_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("Arial"));
	VERIFY(m_fontCourier.CreateFontIndirect(&logfont)); // create the GDI font object from the descriptor

	// apply the uniform font to every text-input and status control
	m_ctrlFolder.SetFont(&m_fontCourier);
	m_ctrlInputFile.SetFont(&m_fontCourier);
	m_ctrlOutputFile.SetFont(&m_fontCourier);
	m_ctrlInputCode.SetFont(&m_fontCourier);
	m_ctrlOutputCode.SetFont(&m_fontCourier);
	m_ctrlDeleteCode.SetFont(&m_fontCourier);
	m_ctrlMessage.SetFont(&m_fontCourier);

	// restrict code edit controls to the defined maximum code length
	m_ctrlInputCode.SetLimitText(MAX_CODE_LENGTH);
	m_ctrlOutputCode.SetLimitText(MAX_CODE_LENGTH);
	m_ctrlDeleteCode.SetLimitText(MAX_CODE_LENGTH);

	// disable action buttons until the user picks a root folder
	m_ctrlFileList.EnableWindow(FALSE);
	m_ctrlUpload.EnableWindow(FALSE);
	m_ctrlDownload.EnableWindow(FALSE);
	m_ctrlDelete.EnableWindow(FALSE);

	m_hashNetwork = new CNetworkHash(); // allocate the hash-based storage backend
	m_treeNetwork = new CNetworkTree(); // allocate the B-Tree storage backend

	m_ctrlProgress.ShowWindow(SW_HIDE); // hide progress bar until a copy operation starts
	m_funcProgress = (LPPROGRESS_ROUTINE)(&CSiatelHomeworkDlg::ProgressFunc); // store callback pointer for CopyFileEx

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**
 * @brief Handles WM_DESTROY; exports network data and frees allocated tree/hash structures.
 */
void CSiatelHomeworkDlg::OnDestroy()
{
	CDialog::OnDestroy();

	VERIFY(m_fontCourier.DeleteObject()); // release the GDI font resource

	// persist the active backend's file index to disk before the window is destroyed
	if (USE_HASH_METHOD)
	{
		ASSERT(m_hashNetwork != nullptr);
		m_hashNetwork->ExportData(); // write hash table contents to filelist.txt
	}
	else
	{
		ASSERT(m_treeNetwork != nullptr);
		m_treeNetwork->ExportData(); // write B-Tree contents to filelist.txt
	}

	// free the B-Tree backend and nullify the pointer to avoid dangling references
	if (m_treeNetwork != nullptr)
	{
		delete m_treeNetwork;
		m_treeNetwork = nullptr;
	}

	// free the hash-table backend and nullify the pointer to avoid dangling references
	if (m_hashNetwork != nullptr)
	{
		delete m_hashNetwork;
		m_hashNetwork = nullptr;
	}
}

/**
 * @brief Handles WM_SYSCOMMAND messages.
 *        Dispatches the About box and external social/project URL commands;
 *        delegates all other system commands to the base class.
 * @param nID System command identifier.
 * @param lParam Additional message-dependent data.
 */
void CSiatelHomeworkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) // mask off low nibble Windows uses for internal sizing/moving state
	{
		CAboutDlg dlgAbout; // show the About dialog modally
		dlgAbout.DoModal();
	}
	else
	{
		if (nID == IDM_TWITTER) // open the author's Twitter/X profile in the default browser
		{
			::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://x.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
		}
		else
		{
			if (nID == IDM_LINKEDIN) // open the author's LinkedIn profile
			{
				::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.linkedin.com/in/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
			}
			else
			{
				if (nID == IDM_FACEBOOK) // open the author's Facebook profile
				{
					::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.facebook.com/stefanmihaimoga"), nullptr, nullptr, SW_SHOW);
				}
				else
				{
					if (nID == IDM_INSTAGRAM) // open the author's Instagram profile
					{
						::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://www.instagram.com/stefanmihaimoga/"), nullptr, nullptr, SW_SHOW);
					}
					else
					{
						if (nID == IDM_ISSUES) // open the GitHub Issues page for bug reports
						{
							::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/BTreeAccess/issues"), nullptr, nullptr, SW_SHOW);
						}
						else
						{
							if (nID == IDM_DISCUSSIONS) // open the GitHub Discussions page
							{
								::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/BTreeAccess/discussions"), nullptr, nullptr, SW_SHOW);
							}
							else
							{
								if (nID == IDM_WIKI) // open the GitHub Wiki documentation page
								{
									::ShellExecute(GetSafeHwnd(), _T("open"), _T("https://github.com/mihaimoga/BTreeAccess/wiki"), nullptr, nullptr, SW_SHOW);
								}
								else
								{
									CDialog::OnSysCommand(nID, lParam); // pass unrecognised commands to the base class
								}
							}
						}
					}

				}
			}
		}
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/**
 * @brief Handles WM_PAINT; draws the application icon centered in the client area
 *        when the window is in its minimized (iconic) state.
 */
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
/**
 * @brief Returns the cursor to display while the user drags the minimized window.
 * @return Handle to the application icon cast as an HCURSOR.
 */
HCURSOR CSiatelHomeworkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**
 * @brief Handles the Browse button click.
 *        Opens a folder browser dialog and, on confirmation, initialises the
 *        network data structure from the selected root folder.
 */
void CSiatelHomeworkDlg::OnBnClickedBrowse()
{
	// folder-picker flags: filesystem paths only, new Explorer-style UI, validate the selection
	const UINT nFolderFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_VALIDATE;

	CFolderDialog dlgFileDialog(nullptr, m_strRootFolder, this, nFolderFlags);
	if (dlgFileDialog.DoModal() == IDOK)
	{
		m_strRootFolder = dlgFileDialog.GetFolderPath(); // retrieve the chosen path
		const UINT lenRootFolder = m_strRootFolder.GetLength();
		if (lenRootFolder > 0)
		{
			// ensure the root folder path always ends with a backslash for safe concatenation
			if (m_strRootFolder.GetAt(lenRootFolder - 1) != _T('\\'))
				m_strRootFolder += _T("\\");
			m_ctrlFolder.SetWindowText(m_strRootFolder); // display the normalised path in the UI
			m_strFileList.Format(_T("%sfilelist.txt"), static_cast<LPCWSTR>(m_strRootFolder)); // derive the index file path
			if (USE_HASH_METHOD)
			{
				ASSERT(m_hashNetwork != nullptr);
				m_hashNetwork->RemoveAll();                       // discard any previously loaded data
				m_hashNetwork->SetFileList(m_strFileList);        // point the backend at the new index file
				m_hashNetwork->SetRootFolder(m_strRootFolder);   // set the base path for all relative lookups
				m_hashNetwork->ImportData();                      // load the index from disk into the hash table
			}
			else
			{
				ASSERT(m_treeNetwork != nullptr);
				m_treeNetwork->DeleteTree();                      // discard any previously loaded data
				m_treeNetwork->SetFileList(m_strFileList);        // point the backend at the new index file
				m_treeNetwork->SetRootFolder(m_strRootFolder);   // set the base path for all relative lookups
				m_treeNetwork->ImportData();                      // load the index from disk into the B-Tree
			}
			m_ctrlMessage.SetWindowText(READY_MESSAGE); // signal to the user that the folder is ready
			m_ctrlFileList.EnableWindow(TRUE);          // allow the user to browse the loaded file list
		}
	}
}

/**
 * @brief Handles the File List button click.
 *        Opens a modal dialog showing all files currently stored in the network.
 */
void CSiatelHomeworkDlg::OnBnClickedFileList()
{
	CSiatelFileListDlg dlgSiatelFileList(this);
	dlgSiatelFileList.SetRootFolder(m_strRootFolder);    // pass the root folder so the list dialog can build absolute paths
	dlgSiatelFileList.SetNetworkTree(m_treeNetwork);     // share the B-Tree backend reference (may be unused if hash mode is active)
	dlgSiatelFileList.SetNetworkHash(m_hashNetwork);     // share the hash-table backend reference
	dlgSiatelFileList.DoModal();                         // display the dialog; blocks until the user closes it
}

/**
 * @brief Handles the Input File button click.
 *        Opens a file-open dialog so the user can select the source file to upload.
 */
void CSiatelHomeworkDlg::OnBnClickedInputFile()
{
	// open-file dialog: the selected file must already exist; hide the read-only checkbox
	CFileDialog pFileDialog(TRUE, nullptr, nullptr,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
		_T("All files\0*.*\0"), this);
	if (pFileDialog.DoModal() == IDOK)
	{
		m_strInputFile = pFileDialog.GetPathName();          // store the full path of the selected source file
		m_ctrlInputFile.SetWindowText(m_strInputFile);       // reflect the selection in the UI
		m_ctrlUpload.EnableWindow(!m_strInputFile.IsEmpty()); // enable Upload only when a valid file is chosen
	}
}

/**
 * @brief Handles the Upload button click.
 *        Generates a network ID, determines a storage path, and copies the
 *        selected input file into the managed network folder.
 */
void CSiatelHomeworkDlg::OnBnClickedUpload()
{
	BOOL bCancelOperation = FALSE; // flag that can be set to TRUE to abort CopyFileEx mid-transfer
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;   // receives the individual folder components of the storage path
	m_ctrlFolder.GetWindowText(m_strRootFolder); // read the current root folder from the UI
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK); // cannot proceed without a root folder
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != nullptr);
			int nFileCode = m_hashNetwork->GenerateID();                          // generate a unique integer ID for this file
			int nPathCode = m_hashNetwork->GetSize();                             // current item count doubles as the path index
			CString strFileCode = m_hashNetwork->EncodeNetworkID(nFileCode);      // encode the ID as a human-readable network code
			CString strFilePath = m_hashNetwork->GetFilePath(nPathCode, arrNetworkPath); // resolve the relative storage path
			VERIFY(m_hashNetwork->CreateNetworkPath(m_strRootFolder, arrNetworkPath));   // create any missing subdirectories
			strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath)); // absolute destination path
			m_ctrlProgress.SetRange32(0, PROGRESS_RANGE); // configure the progress bar range
			m_ctrlProgress.SetPos(0);                     // reset the indicator to the start
			m_ctrlProgress.ShowWindow(SW_SHOW);           // make the progress bar visible
			if (!CopyFileEx(m_strInputFile, strFilename, m_funcProgress, &m_ctrlProgress, &bCancelOperation, 0 ))
			{
				// copy failed: hide the progress bar and display the Win32 error description
				m_ctrlProgress.ShowWindow(SW_HIDE);
				const DWORD dwLastError = GetLastError();
				m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError));
			}
			else
			{
				VERIFY(m_hashNetwork->InsertItem(nFileCode, nPathCode, strFilePath)); // register the new file in the hash index
				strMessage.Format(_T("Done writing file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(strFileCode));
				m_ctrlMessage.SetWindowText(strMessage);         // show the success message with the destination path
				m_ctrlOutputCode.SetWindowText(strFileCode);     // display the generated network code for the user to note
				m_ctrlProgress.SetPos(PROGRESS_RANGE-1);         // advance bar to near-complete
				m_ctrlProgress.UpdateWindow();                   // force an immediate repaint
				Sleep(SLEEP_INTERVAL);                           // brief pause so the user can see the completed bar
				m_ctrlProgress.ShowWindow(SW_HIDE);              // hide the progress bar
			}
		}
		else // USE B-TREE
		{
		}
	}
}

/**
 * @brief Handles the Output File button click.
 *        Opens a file-save dialog so the user can specify the download destination path.
 */
void CSiatelHomeworkDlg::OnBnClickedOutputFile()
{
	// save-file dialog: prompt before overwriting an existing file; the directory must already exist
	CFileDialog pFileDialog(FALSE, nullptr, nullptr,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_LONGNAMES | OFN_PATHMUSTEXIST,
		_T("All files\0*.*\0"), this);
	if (pFileDialog.DoModal() == IDOK)
	{
		m_strOutputFile = pFileDialog.GetPathName();     // store the full destination path
		m_ctrlOutputFile.SetWindowText(m_strOutputFile); // reflect the selection in the UI
		// enable Download only when both an output path and an input code are available
		m_ctrlDownload.EnableWindow(!m_strOutputFile.IsEmpty() && !m_strInputCode.IsEmpty());
	}
}

/**
 * @brief Handles the Download button click.
 *        Decodes the input network code, locates the corresponding file in the
 *        network store, and copies it to the selected output path.
 */
void CSiatelHomeworkDlg::OnBnClickedDownload()
{
	int nLevel = 0;                // receives the tree depth from SearchItem (required by the API, not used here)
	BOOL bCancelOperation = FALSE; // flag that can be set to TRUE to abort CopyFileEx mid-transfer
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;
	m_ctrlFolder.GetWindowText(m_strRootFolder); // read the current root folder from the UI
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK); // cannot proceed without a root folder
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != nullptr);
			if (!m_hashNetwork->IsValidCode(m_strInputCode)) // reject codes containing illegal characters
			{
				m_ctrlMessage.SetWindowText(INVALID_CHARS);
			}
			else
			{
				CString strFilePath; // relative path to file 
				int nFileCode = m_hashNetwork->DecodeNetworkID(m_strInputCode); // convert the alphanumeric code back to an integer key
				if (!m_hashNetwork->SearchItem(nFileCode, nLevel, strFilePath)) // look up the file's relative path in the index
				{
					m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS); // code not found in the index
				}
				else
				{
					strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath)); // build the absolute source path
					m_ctrlProgress.SetRange32(0, PROGRESS_RANGE); // configure the progress bar range
					m_ctrlProgress.SetPos(0);                     // reset the indicator to the start
					m_ctrlProgress.ShowWindow(SW_SHOW);           // make the progress bar visible
					if (!CopyFileEx(strFilename, m_strOutputFile, m_funcProgress, &m_ctrlProgress, &bCancelOperation, 0 ))
					{
						// copy failed: hide the bar, then diagnose whether the file is missing or another OS error occurred
						m_ctrlProgress.ShowWindow(SW_HIDE);
						const DWORD dwLastError = GetLastError();
						if (!m_hashNetwork->FileExists(strFilename))
							m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS); // file in index but not on disk
						else
							m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError)); // other Win32 error
					}
					else
					{
						strMessage.Format(_T("Done reading file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(m_strInputCode));
						m_ctrlMessage.SetWindowText(strMessage);     // show success message with source path and code
						m_ctrlProgress.SetPos(PROGRESS_RANGE-1);     // advance bar to near-complete
						m_ctrlProgress.UpdateWindow();               // force an immediate repaint
						Sleep(SLEEP_INTERVAL);                       // brief pause so the user can see the completed bar
						m_ctrlProgress.ShowWindow(SW_HIDE);          // hide the progress bar
					}
				}
			}
		}
		else // USE B-TREE
		{
		}
	}
}

/**
 * @brief Handles the Delete button click.
 *        Decodes the delete network code, locates the file on disk, removes it,
 *        and updates the network index accordingly.
 */
void CSiatelHomeworkDlg::OnBnClickedDelete()
{
	int nLevel = 0;                // receives the tree depth from SearchItem (required by the API, not used here)
	CString strMessage, strFilename;
	CStringArray arrNetworkPath;
	m_ctrlFolder.GetWindowText(m_strRootFolder); // read the current root folder from the UI
	if (m_strRootFolder.IsEmpty())
	{
		MessageBox(EMPTY_ROOT_FOLDER, _T("Error"), MB_OK); // cannot proceed without a root folder
	}
	else
	{
		if (USE_HASH_METHOD)
		{
			ASSERT(m_hashNetwork != nullptr);
			if (!m_hashNetwork->IsValidCode(m_strDeleteCode)) // reject codes containing illegal characters
			{
				m_ctrlMessage.SetWindowText(INVALID_CHARS);
			}
			else
			{
				CString strFilePath; // relative path resolved from the index
				int nFileCode = m_hashNetwork->DecodeNetworkID(m_strDeleteCode); // convert the alphanumeric code back to an integer key
				if (!m_hashNetwork->SearchItem(nFileCode, nLevel, strFilePath))  // look up the file in the index
				{
					m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS); // code not found in the index
				}
				else
				{
					strFilename.Format(_T("%s%s"), static_cast<LPCWSTR>(m_strRootFolder), static_cast<LPCWSTR>(strFilePath)); // build the absolute path
					if (!m_hashNetwork->FileExists(strFilename)) // confirm the file is actually present on disk
					{
						m_ctrlMessage.SetWindowText(FILE_DOES_NOT_EXISTS); // index entry exists but the file is missing
					}
					else
					{
						if (!DeleteFile(strFilename)) // attempt the Win32 file deletion
						{
							// deletion failed (e.g. file locked); report the OS error
							const DWORD dwLastError = GetLastError();
							m_ctrlMessage.SetWindowText(m_hashNetwork->FormatLastError(dwLastError));
						}
						else
						{
							VERIFY(m_hashNetwork->DeleteItem(nFileCode)); // remove the entry from the hash index
							strMessage.Format(_T("File %s [%s] has been deleted ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(m_strDeleteCode));
							m_ctrlMessage.SetWindowText(strMessage);      // confirm the deletion to the user
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

/**
 * @brief Handles EN_CHANGE for the input code edit control.
 *        Reads the current text and enables or disables the Download button
 *        based on whether both an output file and an input code are present.
 */
void CSiatelHomeworkDlg::OnEnChangeInputCode()
{
	m_ctrlInputCode.GetWindowText(m_strInputCode); // sync the member variable with what the user has typed
	// enable Download only when both the network code and the output file path are non-empty
	m_ctrlDownload.EnableWindow(!m_strOutputFile.IsEmpty() && !m_strInputCode.IsEmpty());
}

/**
 * @brief Handles EN_CHANGE for the delete code edit control.
 *        Reads the current text and enables or disables the Delete button
 *        based on whether a delete code has been entered.
 */
void CSiatelHomeworkDlg::OnEnChangeDeleteCode()
{
	m_ctrlDeleteCode.GetWindowText(m_strDeleteCode); // sync the member variable with what the user has typed
	m_ctrlDelete.EnableWindow(!m_strDeleteCode.IsEmpty()); // enable Delete only when a code has been entered
}

/**
 * @brief CopyFileEx progress callback; updates the progress bar control during a file copy.
 * @param TotalFileSize Total size of the file being copied.
 * @param TotalBytesTransferred Total number of bytes transferred so far.
 * @param StreamSize Size of the current stream (unused).
 * @param StreamBytesTransferred Bytes transferred in the current stream (unused).
 * @param dwStreamNumber Current stream number (unused).
 * @param dwCallbackReason Reason the callback was invoked (unused).
 * @param hSourceFile Handle to the source file (unused).
 * @param hDestinationFile Handle to the destination file (unused).
 * @param lpData Application-defined pointer; expected to be a CProgressCtrl*.
 * @return PROGRESS_CONTINUE to allow the copy to proceed.
 */
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
	// suppress warnings for parameters provided by the CopyFileEx API that we don't need
	UNREFERENCED_PARAMETER(StreamSize);
	UNREFERENCED_PARAMETER(StreamBytesTransferred);
	UNREFERENCED_PARAMETER(dwStreamNumber);
	UNREFERENCED_PARAMETER(dwCallbackReason);
	UNREFERENCED_PARAMETER(hSourceFile);
	UNREFERENCED_PARAMETER(hDestinationFile);

	ASSERT(lpData != nullptr); // lpData must point to the CProgressCtrl passed via CopyFileEx
	float nFileSize    = (float)TotalFileSize.QuadPart;           // cast to float for percentage arithmetic
	float nTransferred = (float)TotalBytesTransferred.QuadPart;
	if (TotalFileSize.QuadPart != 0) // guard against division by zero for empty files
	{
		int nPosition = (int)(nTransferred / nFileSize * (float)PROGRESS_RANGE); // scale transferred bytes to bar range
		((CProgressCtrl*)lpData)->SetPos(nPosition);   // update the progress bar position
		((CProgressCtrl*)lpData)->UpdateWindow();       // force an immediate repaint so the UI stays responsive
	}
	return PROGRESS_CONTINUE; // tell CopyFileEx to keep going
}
