/* This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelHomeworkExt.cpp : implementation file
//

#include "stdafx.h"
#include <direct.h>
#include "SiatelHomework.h"
#include "SiatelHomeworkExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CNetworkBase, CObject)
IMPLEMENT_DYNAMIC(CNetworkTree, CNetworkBase)
IMPLEMENT_DYNAMIC(CNetworkHash, CNetworkBase)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkBase ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CNetworkBase::CNetworkBase()
{
}

CNetworkBase::~CNetworkBase()
{
}

CString CNetworkBase::FormatLastError(DWORD dwLastError)
{
	LPVOID lpMsgBuf = NULL;
	CString	strLastError;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL);
	strLastError = (LPCTSTR)lpMsgBuf; 
	//Trim CR/LF from the error message.
	strLastError.TrimRight();

	// free alocated buffer by FormatMessage
	LocalFree(lpMsgBuf);

	//Display the last error.
	TRACE(_T("%s\n"), static_cast<LPCWSTR>(strLastError));
	return strLastError;
}

BOOL CNetworkBase::FileExists(CString strFilename)
{
	const UINT nBinaryFileFlags = CFile::modeRead | CFile::typeBinary;
	try {
		CFile pBinaryFile(strFilename, nBinaryFileFlags);
		pBinaryFile.Close();
	} catch (CFileException *pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CNetworkBase::IsValidCode(CString strFileCode)
{
	const int nLength = strFileCode.GetLength();
	for (int nIndex = 0; nIndex < nLength; nIndex++)
	{
		TCHAR chDigit = strFileCode.GetAt(nIndex);
		if (-1 == strNetworkDigitString.Find(chDigit))
			return FALSE;
	}
	return TRUE;
}

CString CNetworkBase::EncodeNetworkID(int nKey)
{
	CString strKey;

	int nModulo = nKey % intNetworkDigitLength;
	nKey /= intNetworkDigitLength;

	while (nKey != 0)
	{
		strKey.Insert(0, strNetworkDigitString.GetAt(nModulo));
		nModulo = nKey % intNetworkDigitLength;
		nKey /= intNetworkDigitLength;
	}
	strKey.Insert(0, strNetworkDigitString.GetAt(nModulo));

	while (strKey.GetLength() < MAX_CODE_LENGTH)
		strKey.Insert(0, _T('0'));

	return strKey;
}

int CNetworkBase::DecodeNetworkID(CString strKey)
{
	int nKey = 0;

	const int nLength = strKey.GetLength();
	for (int nIndex = 0; nIndex < nLength; nIndex++)
	{
		nKey *= intNetworkDigitLength;
		TCHAR chDigit = strKey.GetAt(nIndex);
		int nDigit = strNetworkDigitString.Find(chDigit);
		ASSERT(nDigit >= 0);
		nKey += nDigit;
	}

	return nKey;
}

int CNetworkBase::GetNextID()
{
	return 1 + (int)((double)rand() / (1.0 + RAND_MAX) * MAX_RANDOM_DATA);
}

CString CNetworkBase::FormatFolder(int nKey)
{
	CString strFolder;
	strFolder.Format(_T("%s"), static_cast<LPCWSTR>(EncodeNetworkID(nKey)));
	return strFolder;
}

CString CNetworkBase::FormatFilename(int nKey)
{
	CString strFilename;
	strFilename.Format(_T("%s.dat"), static_cast<LPCWSTR>(EncodeNetworkID(nKey)));
	return strFilename;
}

CString CNetworkBase::GetFilePath(int nKey, CStringArray & arrFilePath)
{
	BOOL bFirstTime = TRUE;
	CString strFilePath;
	arrFilePath.RemoveAll();
	int folder_no = nKey % BALANCED_TREE_ORDER;
	int file_name = nKey - folder_no;
	while (file_name != 0)
	{
		if (bFirstTime)
		{
			strFilePath = FormatFilename(folder_no);
			bFirstTime = FALSE;
		}
		else
		{
			CString strFolder = FormatFolder(folder_no);
			arrFilePath.InsertAt(0, strFolder);
			strFolder += _T("\\");
			strFilePath.Insert(0, strFolder);
		}
		nKey /= BALANCED_TREE_ORDER;
		folder_no = nKey % BALANCED_TREE_ORDER;
		file_name = nKey - folder_no;
	}
	if (bFirstTime)
	{
		strFilePath = FormatFilename(folder_no);
	}
	else
	{
		CString strFolder = FormatFolder(folder_no);
		arrFilePath.InsertAt(0, strFolder);
		strFolder += _T("\\");
		strFilePath.Insert(0, strFolder);
	}

	return strFilePath;
}

BOOL CNetworkBase::CreateNetworkPath(CString strRootFolder, CStringArray & strSubFolders)
{
	CString strNetworkPath = strRootFolder;
	if (!SetCurrentDirectory(strNetworkPath))
		return FALSE;

	const int nLength = (int)strSubFolders.GetCount();
	for (int nIndex = 0; nIndex < nLength; nIndex++)
	{
		strNetworkPath.AppendFormat(_T("\\%s"), static_cast<LPCWSTR>(strSubFolders.GetAt(nIndex)));
		if (!SetCurrentDirectory(strNetworkPath))
		{
			if (!CreateDirectory(strNetworkPath, NULL))
				return FALSE;
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkTree ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CNetworkTree::CNetworkTree()
{
	VERIFY(CreateTree());
}

CNetworkTree::~CNetworkTree()
{
	VERIFY(DeleteTree());
}

BOOL CNetworkTree::ImportData()
{
	return TRUE;
}

BOOL CNetworkTree::ExportData()
{
	return TRUE;
}

int CNetworkTree::GenerateID()
{
	CString strFilePath;
	int nKey = GetNextID();
	while (SearchNode(nKey, strFilePath))
		nKey = GetNextID();
	return nKey;
}

BOOL CNetworkTree::CreateTree()
{
	for (int i = 0; i <= BALANCED_TREE_ORDER; i++)
	{
		m_ptrLink[i] = NULL;
	}
	SetLeaf(TRUE);
	SetRoot(this);
	SetSize(0);
	return TRUE;
}

BOOL CNetworkTree::DeleteTree()
{
	for (int i = 0; i <= BALANCED_TREE_ORDER; i++)
	{
		if (m_ptrLink[i] != NULL)
		{
			delete m_ptrLink[i];
			m_ptrLink[i] = NULL;
		}
	}
	return TRUE;
}

BOOL CNetworkTree::SearchNode(int nKey, CString & strFilePath)
{
	ASSERT(GetSize() == (UINT)m_listKey.GetCount());
	ASSERT(GetSize() == (UINT)m_listCode.GetCount());

	UINT i = 0;
	while ((i < GetSize()) && (nKey > m_listKey.GetAt(i)))
		i++;

	if ((i < GetSize()) && (nKey == m_listKey.GetAt(i)))
	{
		strFilePath = m_listCode.GetAt(i);
		return TRUE;
	}

	if (GetLeaf())
		return FALSE;

	ASSERT(m_ptrLink[i] != NULL);
	return m_ptrLink[i]->SearchNode(nKey, strFilePath);
}

/*BOOL CNetworkTree::UpdateNode(int nKey, CString strFilePath)
{
	ASSERT(GetSize() == (UINT)m_listKey.GetCount());
	ASSERT(GetSize() == (UINT)m_listCode.GetCount());

	UINT i = 0;
	while ((i < GetSize()) && (nKey > m_listKey.GetAt(i)))
		i++;

	if ((i < GetSize()) && (nKey == m_listKey.GetAt(i)))
	{
		m_listCode.SetAt(i, strFilePath);
		return TRUE;
	}

	if (GetLeaf())
		return FALSE;

	ASSERT(m_ptrLink[i] != NULL);
	return m_ptrLink[i]->SearchNode(nKey, strFilePath);
}

BOOL CNetworkTree::InsertNode(int nKey, CString strFilePath)
{
	CNetworkTree * r = GetRoot();
	if (r->GetSize() < BALANCED_TREE_ORDER)
	{
		VERIFY(r->InsertEmpty(nKey, strFilePath));
	}
	else
	{
		CNetworkTree * s = new CNetworkTree();
		SetRoot(s);
		s->SetLeaf(FALSE);
		s->SetSize(0);
		s->SetLink(0, r);
		VERIFY(DivideNode(s, 0, r));
		VERIFY(s->InsertEmpty(nKey, strFilePath));
	}
	return TRUE;
}

BOOL CNetworkTree::InsertEmpty(int nKey, CString strFilePath)
{
	if (GetLeaf())
	{
		m_listKey.SetSize(GetSize()+1);
		m_listCode.SetSize(GetSize()+1);
		if (GetSize() == 0)
		{
			m_listKey.SetAt(0, nKey);
			m_listCode.SetAt(0, strFilePath);
			SetSize(1);
		}
		else
		{
			int i = GetSize()-1;
			while ((i >= 0) && (nKey < m_listKey.GetAt(i)))
			{
				m_listKey.SetAt(i+1, m_listKey.GetAt(i));
				m_listCode.SetAt(i+1, m_listCode.GetAt(i));
				i--;
			}
			m_listKey.SetAt(i+1, nKey);
			m_listCode.SetAt(i+1, strFilePath);
			SetSize(GetSize()+1);
		}
	}
	else
	{
		//int i = GetSize()-1;
	}
	return TRUE;
}*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkHash ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CNetworkHash::CNetworkHash()
{
	m_mapFileCode.InitHashTable(MAX_RANDOM_DATA);
	m_mapFilePath.InitHashTable(MAX_RANDOM_DATA);
}

CNetworkHash::~CNetworkHash()
{
}

void CNetworkHash::RemoveAll()
{
	m_listKey.RemoveAll();
	m_mapFileCode.RemoveAll();
	m_mapFilePath.RemoveAll();
}

BOOL CNetworkHash::ImportData()
{
	int nLevel = 0, nFileCode = 0;
	CString strFileLine, strFilePath;
	const UINT nTextFileFlags = CFile::modeRead | CFile::typeText;
	try {
		CStdioFile pTextFile(GetFileList(), nTextFileFlags);
		while (pTextFile.ReadString(strFileLine))
		{
			int nFirstNoLength = strFileLine.Find(_T(' '), 0);
			int nSecondNoLength = strFileLine.Find(_T(' '), nFirstNoLength + 1);
			ASSERT((nFirstNoLength >= 0) && (nSecondNoLength >= 0));
			nFileCode = _tstoi(strFileLine.Mid(0, nFirstNoLength));
			nLevel = _tstoi(strFileLine.Mid(nFirstNoLength + 1, nSecondNoLength));
			strFilePath = strFileLine.Mid(nSecondNoLength + 1);
			VERIFY(InsertItem(nFileCode, nLevel, strFilePath));
			TRACE(_T("Importing data %d %d %s ...\n"), nFileCode, nLevel, static_cast<LPCWSTR>(strFilePath));
		}
		pTextFile.Close();
	} catch (CFileException *pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return FALSE;
	}
	return TRUE;
}

BOOL CNetworkHash::ExportData()
{
	int nLevel = 0;
	CString strFileLine, strFilePath;
	const UINT nTextFileFlags = CFile::modeCreate | CFile::modeWrite | CFile::typeText;
	try {
		CStdioFile pTextFile(GetFileList(), nTextFileFlags);
		const int nLength = GetSize();
		for (int nIndex = 0; nIndex < nLength; nIndex++)
		{
			int nFileCode = GetKey(nIndex);
			VERIFY(SearchItem(nFileCode, nLevel, strFilePath));
			strFileLine.Format(_T("%d %d %s\n"), nFileCode, nLevel, static_cast<LPCWSTR>(strFilePath));
			pTextFile.WriteString(strFileLine);
			TRACE(_T("Exporting data %d %d %s ...\n"), nFileCode, nLevel, static_cast<LPCWSTR>(strFilePath));
		}
		pTextFile.Close();
	} catch (CFileException *pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return FALSE;
	}
	return TRUE;
}

int CNetworkHash::GenerateID()
{
	int nLevel = 0;
	CString strFilePath;
	int nKey = GetNextID();
	while (SearchItem(nKey, nLevel, strFilePath))
		nKey = GetNextID();
	return nKey;
}

BOOL CNetworkHash::SearchItem(int nKey, int & nLevel, CString & strFilePath)
{
	BOOL bFileCode = m_mapFileCode.Lookup(nKey, nLevel);
	BOOL bFilePath = m_mapFilePath.Lookup(nKey, strFilePath);
	return (bFileCode && bFilePath);
}

BOOL CNetworkHash::UpdateItem(int nKey, int nLevel, CString strFilePath)
{
	m_mapFileCode.SetAt(nKey, nLevel);
	m_mapFilePath.SetAt(nKey, strFilePath);
	return TRUE;
}

BOOL CNetworkHash::InsertItem(int nKey, int nLevel, CString strFilePath)
{
	m_listKey.Add(nKey);
	m_mapFileCode.SetAt(nKey, nLevel);
	m_mapFilePath.SetAt(nKey, strFilePath);
	return TRUE;
}

BOOL CNetworkHash::DeleteItem(int nKey)
{
	int nOldLevel = 0, nNewLevel = 0;
	CString strOldFilePath, strNewFilePath;
	const int nLength = GetSize();
	if (nLength > 0)
	{
		if (SearchItem(nKey, nNewLevel, strNewFilePath))
		{
			for (int nIndex = 0; nIndex < nLength; nIndex++)
			{
				if (nKey == m_listKey.GetAt(nIndex))
				{
					m_listKey.RemoveAt(nIndex);
					break;
				}
			}

			for (int nIndex = 0; nIndex < GetSize(); nIndex++)
			{
				const int nCurrentKey = m_listKey.GetAt(nIndex);
				if (SearchItem(nCurrentKey, nOldLevel, strOldFilePath))
				{
					if (nOldLevel == GetSize())
					{
						CString strFileName = strNewFilePath;
						// strNewFilePath = the file just deleted
						// strOldFilePath = the file in the old location
						strOldFilePath.Insert(0, GetRootFolder());
						strFileName.Insert(0, GetRootFolder());
						if (!MoveFile(strOldFilePath, strFileName))
						{
							const DWORD dwLastError = GetLastError();
							CString strError = FormatLastError(dwLastError);
							AfxMessageBox(strError, MB_OK | MB_ICONSTOP);
						}
						else
						{
							VERIFY(UpdateItem(nCurrentKey, nNewLevel, strNewFilePath));
						}
						break;
					}
				}
			}
		}
	}
	BOOL bFileCode = m_mapFileCode.RemoveKey(nKey);
	BOOL bFilePath = m_mapFilePath.RemoveKey(nKey);
	return (bFileCode && bFilePath);
}
