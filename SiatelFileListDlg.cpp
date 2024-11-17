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

// SiatelFileListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SiatelHomework.h"
#include "SiatelFileListDlg.h"
#include "SiatelHomeworkExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSiatelFileListDlg dialog

IMPLEMENT_DYNAMIC(CSiatelFileListDlg, CDialog)

CSiatelFileListDlg::CSiatelFileListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSiatelFileListDlg::IDD, pParent)
{
	m_treeNetwork = nullptr;
	m_hashNetwork = nullptr;
}

CSiatelFileListDlg::~CSiatelFileListDlg()
{
}

void CSiatelFileListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
}

BEGIN_MESSAGE_MAP(CSiatelFileListDlg, CDialog)
END_MESSAGE_MAP()

// CSiatelFileListDlg message handlers

BOOL CSiatelFileListDlg::OnInitDialog()
{
	int nLevel = 0;
	CString strIndex;
	CString strFilePath;
	CString strFileCode;

	CDialog::OnInitDialog();

	m_ctrlFileList.SetExtendedStyle(m_ctrlFileList.GetExtendedStyle()
		| LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_ctrlFileList.InsertColumn(0, _T("#"), LVCFMT_LEFT, 50);
	m_ctrlFileList.InsertColumn(1, _T("Code"), LVCFMT_LEFT, 50);
	m_ctrlFileList.InsertColumn(2, _T("Path"), LVCFMT_LEFT, 330);

	if (USE_HASH_METHOD)
	{
		ASSERT(m_hashNetwork != nullptr);
		const int nLength = (int)m_hashNetwork->GetSize();
		for (int nIndex = 0; nIndex < nLength; nIndex++)
		{
			strIndex.Format(_T("%d"), nIndex);
			int nFileCode = m_hashNetwork->GetKey(nIndex);
			strFileCode = m_hashNetwork->EncodeNetworkID(nFileCode);
			VERIFY(m_hashNetwork->SearchItem(nFileCode, nLevel, strFilePath));
			strFilePath.Insert(0, m_strRootFolder);
			m_ctrlFileList.InsertItem(nIndex, strIndex);
			m_ctrlFileList.SetItemText(nIndex, 1, strFileCode);
			m_ctrlFileList.SetItemText(nIndex, 2, strFilePath);
		}
	}
	else // USE B-TREE
	{
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
