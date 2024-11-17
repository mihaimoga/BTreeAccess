/* Copyright (C) 2014-2025 Stefan-Mihai MOGA
This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelHomeworkDlg.h : header file
//

#ifndef __SIATELHOMEWORKDLG__
#define __SIATELHOMEWORKDLG__

#pragma once

#include "stdafx.h"
#include "SiatelHomeworkExt.h"

// CSiatelHomeworkDlg dialog
class CSiatelHomeworkDlg : public CDialog
{
// Construction
public:
	CSiatelHomeworkDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIATELHOMEWORK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	CFont m_fontCourier;
	CString m_strFileList;
	CString m_strRootFolder;
	CString m_strInputFile;
	CString m_strOutputFile;
	CString m_strInputCode;
	CString m_strOutputCode;
	CString m_strDeleteCode;

	CEdit m_ctrlFolder;
	CEdit m_ctrlInputFile;
	CEdit m_ctrlOutputCode;
	CEdit m_ctrlOutputFile;
	CEdit m_ctrlInputCode;
	CEdit m_ctrlDeleteCode;
	CStatic m_ctrlMessage;
	CProgressCtrl m_ctrlProgress;
	CButton m_ctrlFileList;
	CButton m_ctrlUpload;
	CButton m_ctrlDownload;
	CButton m_ctrlDelete;

	CNetworkTree* m_treeNetwork;
	CNetworkHash* m_hashNetwork;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedInputFile();
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedOutputFile();
	afx_msg void OnBnClickedDownload();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedFileList();
	afx_msg void OnEnChangeInputCode();
	afx_msg void OnEnChangeDeleteCode();

	static DWORD CALLBACK ProgressFunc(
		LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred,
		LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred,
		DWORD dwStreamNumber,
		DWORD dwCallbackReason,
		HANDLE hSourceFile,
		HANDLE hDestinationFile,
		LPVOID lpData);

	LPPROGRESS_ROUTINE m_funcProgress;

	DECLARE_MESSAGE_MAP()
};

#endif // __SIATELHOMEWORKDLG__
