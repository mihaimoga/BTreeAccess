/* Copyright (C) 2014-2024 Stefan-Mihai MOGA
This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelFileListDlg.h : header file
//

#ifndef __SIATELFILELISTDLG__
#define __SIATELFILELISTDLG__

#pragma once

#include "stdafx.h"
#include "SiatelHomeworkExt.h"
#include "afxcmn.h"

// CSiatelFileListDlg dialog

class CSiatelFileListDlg : public CDialog
{
	DECLARE_DYNAMIC(CSiatelFileListDlg)

public:
	CSiatelFileListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSiatelFileListDlg();

	inline void SetRootFolder(CString strFolder) { m_strRootFolder = strFolder; };
	inline void SetNetworkTree(CNetworkTree * pTree) { m_treeNetwork = pTree; };
	inline void SetNetworkHash(CNetworkHash * pHash) { m_hashNetwork = pHash; };

// Dialog Data
	enum { IDD = IDD_SIATELFILELISTDLG };

protected:
	CString m_strRootFolder;

	CNetworkTree * m_treeNetwork;
	CNetworkHash * m_hashNetwork;

	CListCtrl m_ctrlFileList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

#endif // __SIATELFILELISTDLG__
