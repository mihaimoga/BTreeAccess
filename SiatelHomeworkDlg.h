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

// SiatelHomeworkDlg.h : header file
//

#ifndef __SIATELHOMEWORKDLG__
#define __SIATELHOMEWORKDLG__

#pragma once

#include "stdafx.h"
#include "SiatelHomeworkExt.h"

/**
 * @class CSiatelHomeworkDlg
 * @brief Main application dialog for the BTreeAccess homework application.
 *
 * Provides the primary UI for uploading, downloading, and deleting files
 * within a managed network folder structure using either a B-Tree or hash-based
 * storage method.
 */
class CSiatelHomeworkDlg : public CDialog
{
// Construction
public:
	/**
	 * @brief Standard constructor.
	 * @param pParent Pointer to the parent window. Defaults to NULL.
	 */
	CSiatelHomeworkDlg(CWnd* pParent = NULL);

// Dialog Data
	enum { IDD = IDD_SIATELHOMEWORK_DIALOG };

	protected:
	/**
	 * @brief Performs data exchange and validation between dialog controls and member variables.
	 * @param pDX Pointer to the CDataExchange object used for DDX/DDV.
	 */
	virtual void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	/** @brief Application icon handle. */
	HICON m_hIcon;
	/** @brief Fixed-pitch font used for text controls. */
	CFont m_fontCourier;
	/** @brief Path to the file list index file. */
	CString m_strFileList;
	/** @brief Path to the root network folder. */
	CString m_strRootFolder;
	/** @brief Path to the selected input (source) file for upload. */
	CString m_strInputFile;
	/** @brief Path to the selected output (destination) file for download. */
	CString m_strOutputFile;
	/** @brief Encoded network ID used for download lookup. */
	CString m_strInputCode;
	/** @brief Encoded network ID generated after a successful upload. */
	CString m_strOutputCode;
	/** @brief Encoded network ID used for file deletion. */
	CString m_strDeleteCode;

	/** @brief Edit control displaying the root folder path. */
	CEdit m_ctrlFolder;
	/** @brief Edit control displaying the input file path. */
	CEdit m_ctrlInputFile;
	/** @brief Edit control displaying the output (download) network code. */
	CEdit m_ctrlOutputCode;
	/** @brief Edit control displaying the output file path. */
	CEdit m_ctrlOutputFile;
	/** @brief Edit control for entering the download network code. */
	CEdit m_ctrlInputCode;
	/** @brief Edit control for entering the delete network code. */
	CEdit m_ctrlDeleteCode;
	/** @brief Static label used for status messages. */
	CStatic m_ctrlMessage;
	/** @brief Progress bar control for file copy operations. */
	CProgressCtrl m_ctrlProgress;
	/** @brief Button to open the file list dialog. */
	CButton m_ctrlFileList;
	/** @brief Button to trigger a file upload. */
	CButton m_ctrlUpload;
	/** @brief Button to trigger a file download. */
	CButton m_ctrlDownload;
	/** @brief Button to trigger a file deletion. */
	CButton m_ctrlDelete;

	/** @brief Pointer to the B-Tree network storage structure. */
	CNetworkTree* m_treeNetwork;
	/** @brief Pointer to the hash-based network storage structure. */
	CNetworkHash* m_hashNetwork;

	// Generated message map functions
	/** @brief Called when the dialog is initialized; sets up controls and data structures. */
	virtual BOOL OnInitDialog();
	/** @brief Handles WM_DESTROY; exports data and cleans up allocated resources. */
	afx_msg void OnDestroy();
	/**
	 * @brief Handles WM_SYSCOMMAND messages, including About box and social media links.
	 * @param nID System command ID.
	 * @param lParam Additional message parameter.
	 */
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	/** @brief Handles WM_PAINT; draws the icon when the window is minimized. */
	afx_msg void OnPaint();
	/** @brief Returns the cursor to display while the user drags the minimized window. */
	afx_msg HCURSOR OnQueryDragIcon();

	/** @brief Handles Browse button click; opens a folder picker dialog. */
	afx_msg void OnBnClickedBrowse();
	/** @brief Handles Input File button click; opens a file open dialog. */
	afx_msg void OnBnClickedInputFile();
	/** @brief Handles Upload button click; copies the selected file into the network store. */
	afx_msg void OnBnClickedUpload();
	/** @brief Handles Output File button click; opens a file save dialog. */
	afx_msg void OnBnClickedOutputFile();
	/** @brief Handles Download button click; retrieves a file from the network store. */
	afx_msg void OnBnClickedDownload();
	/** @brief Handles Delete button click; removes a file from the network store. */
	afx_msg void OnBnClickedDelete();
	/** @brief Handles File List button click; opens the file list dialog. */
	afx_msg void OnBnClickedFileList();
	/** @brief Handles EN_CHANGE for the input code edit control; updates download button state. */
	afx_msg void OnEnChangeInputCode();
	/** @brief Handles EN_CHANGE for the delete code edit control; updates delete button state. */
	afx_msg void OnEnChangeDeleteCode();

	/**
	 * @brief CopyFileEx progress callback; updates the progress bar during file copy.
	 * @param TotalFileSize Total size of the file being copied.
	 * @param TotalBytesTransferred Number of bytes transferred so far.
	 * @param StreamSize Size of the current stream.
	 * @param StreamBytesTransferred Bytes transferred for the current stream.
	 * @param dwStreamNumber Current stream number.
	 * @param dwCallbackReason Reason the callback was invoked.
	 * @param hSourceFile Handle to the source file.
	 * @param hDestinationFile Handle to the destination file.
	 * @param lpData Application-defined data; cast to CProgressCtrl*.
	 * @return PROGRESS_CONTINUE to continue the copy operation.
	 */
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

	/** @brief Pointer to ProgressFunc, passed to CopyFileEx as the progress callback. */
	LPPROGRESS_ROUTINE m_funcProgress = nullptr;

	DECLARE_MESSAGE_MAP()
};

#endif // __SIATELHOMEWORKDLG__
