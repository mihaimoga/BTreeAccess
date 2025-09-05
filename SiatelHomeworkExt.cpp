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

// SiatelHomeworkExt.cpp : implementation file
//
// This file implements the core classes for network-based file management
// in the BTreeAccess application. It provides base functionality for file
// existence checks, encoding/decoding network IDs, and path formatting,
// as well as two derived classes for tree-based and hash-based network
// structures.
//
// Classes implemented:
//   - CNetworkBase:   Base class with utility methods for file and ID handling.
//   - CNetworkTree:   Implements a B-Tree-like structure for managing network nodes.
//   - CNetworkHash:   Implements a hash table for fast access to network items.
//
// Copyright (C) 2014-2025 Stefan-Mihai MOGA
///////////////////////////////////////////////////////////////////////////////

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
// CNetworkBase
///////////////////////////////////////////////////////////////////////////////

/**
 * CNetworkBase
 * ------------
 * Base class providing utility functions for file operations, network ID
 * encoding/decoding, and path formatting.
 */
	CNetworkBase::CNetworkBase()
{
}

CNetworkBase::~CNetworkBase()
{
}

/**
 * FormatLastError
 * ---------------
 * Converts a Windows error code to a human-readable string.
 *
 * @param dwLastError The error code.
 * @return CString    The formatted error message.
 */
CString CNetworkBase::FormatLastError(DWORD dwLastError)
{
	LPVOID lpMsgBuf = nullptr;
	CString	strLastError;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		nullptr);
	strLastError = (LPCTSTR)lpMsgBuf;
	//Trim CR/LF from the error message.
	strLastError.TrimRight();

	// free alocated buffer by FormatMessage
	LocalFree(lpMsgBuf);

	//Display the last error.
	TRACE(_T("%s\n"), static_cast<LPCWSTR>(strLastError));
	return strLastError;
}

/**
 * FileExists
 * ----------
 * Checks if a file exists at the given path.
 *
 * @param strFilename The file path.
 * @return bool       True if the file exists, false otherwise.
 */
bool CNetworkBase::FileExists(CString strFilename)
{
	const UINT nBinaryFileFlags = CFile::modeRead | CFile::typeBinary;
	try {
		CFile pBinaryFile(strFilename, nBinaryFileFlags);
		pBinaryFile.Close();
	}
	catch (CFileException* pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return false;
	}
	return true;
}

/**
 * IsValidCode
 * -----------
 * Validates if the given string contains only allowed network digits.
 *
 * @param strFileCode The code string to validate.
 * @return bool       True if valid, false otherwise.
 */
bool CNetworkBase::IsValidCode(CString strFileCode)
{
	const int nLength = strFileCode.GetLength();
	for (int nIndex = 0; nIndex < nLength; nIndex++)
	{
		TCHAR chDigit = strFileCode.GetAt(nIndex);
		if (-1 == strNetworkDigitString.Find(chDigit))
			return false;
	}
	return true;
}

/**
 * EncodeNetworkID
 * ---------------
 * Encodes an integer key into a string using the network digit set.
 *
 * @param nKey    The integer key.
 * @return CString The encoded string.
 */
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

/**
 * DecodeNetworkID
 * ---------------
 * Decodes a string network ID back to its integer representation.
 *
 * @param strKey  The encoded string.
 * @return int    The decoded integer key.
 */
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

/**
 * GetNextID
 * ---------
 * Generates a pseudo-random ID for new network entries.
 *
 * @return int    The generated ID.
 */
int CNetworkBase::GetNextID()
{
	return 1 + (int)((double)rand() / (1.0 + RAND_MAX) * MAX_RANDOM_DATA);
}

/**
 * FormatFolder
 * ------------
 * Formats a folder name based on the network key.
 *
 * @param nKey    The network key.
 * @return CString The folder name.
 */
CString CNetworkBase::FormatFolder(int nKey)
{
	CString strFolder;
	strFolder.Format(_T("%s"), static_cast<LPCWSTR>(EncodeNetworkID(nKey)));
	return strFolder;
}

/**
 * FormatFilename
 * --------------
 * Formats a filename (with .dat extension) based on the network key.
 *
 * @param nKey    The network key.
 * @return CString The filename.
 */
CString CNetworkBase::FormatFilename(int nKey)
{
	CString strFilename;
	strFilename.Format(_T("%s.dat"), static_cast<LPCWSTR>(EncodeNetworkID(nKey)));
	return strFilename;
}

/**
 * GetFilePath
 * -----------
 * Constructs the full file path for a given key, updating the folder array.
 *
 * @param nKey         The network key.
 * @param arrFilePath  Array to receive the folder path components.
 * @return CString     The full file path.
 */
CString CNetworkBase::GetFilePath(int nKey, CStringArray& arrFilePath)
{
	bool bFirstTime = true;
	CString strFilePath;
	arrFilePath.RemoveAll();
	int folder_no = nKey % BALANCED_TREE_ORDER;
	int file_name = nKey - folder_no;
	while (file_name != 0)
	{
		if (bFirstTime)
		{
			strFilePath = FormatFilename(folder_no);
			bFirstTime = false;
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

/**
 * CreateNetworkPath
 * -----------------
 * Ensures the directory structure exists for the given root and subfolders.
 *
 * @param strRootFolder The root directory.
 * @param strSubFolders Array of subfolder names.
 * @return bool         True if the path exists or was created, false otherwise.
 */
bool CNetworkBase::CreateNetworkPath(CString strRootFolder, CStringArray& strSubFolders)
{
	CString strNetworkPath = strRootFolder;
	if (!SetCurrentDirectory(strNetworkPath))
		return false;

	const int nLength = (int)strSubFolders.GetCount();
	for (int nIndex = 0; nIndex < nLength; nIndex++)
	{
		strNetworkPath.AppendFormat(_T("\\%s"), static_cast<LPCWSTR>(strSubFolders.GetAt(nIndex)));
		if (!SetCurrentDirectory(strNetworkPath))
		{
			if (!CreateDirectory(strNetworkPath, nullptr))
				return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// CNetworkTree
///////////////////////////////////////////////////////////////////////////////

/**
 * CNetworkTree
 * ------------
 * Implements a B-Tree-like structure for managing network nodes.
 * Inherits from CNetworkBase.
 */
CNetworkTree::CNetworkTree()
{
	VERIFY(CreateTree());
}

CNetworkTree::~CNetworkTree()
{
	VERIFY(DeleteTree());
}

/**
 * ImportData
 * ----------
 * Placeholder for importing data into the tree.
 *
 * @return bool Always true (not implemented).
 */
bool CNetworkTree::ImportData()
{
	return true;
}

/**
 * ExportData
 * ----------
 * Placeholder for exporting data from the tree.
 *
 * @return bool Always true (not implemented).
 */
bool CNetworkTree::ExportData()
{
	return true;
}

/**
 * GenerateID
 * ----------
 * Generates a unique ID not present in the tree.
 *
 * @return int The generated unique ID.
 */
int CNetworkTree::GenerateID()
{
	CString strFilePath;
	int nKey = GetNextID();
	while (SearchNode(nKey, strFilePath))
		nKey = GetNextID();
	return nKey;
}

/**
 * CreateTree
 * ----------
 * Initializes the tree structure.
 *
 * @return bool True if successful.
 */
bool CNetworkTree::CreateTree()
{
	for (int i = 0; i <= BALANCED_TREE_ORDER; i++)
	{
		m_ptrLink[i] = nullptr;
	}
	SetLeaf(true);
	SetRoot(this);
	SetSize(0);
	return true;
}

/**
 * DeleteTree
 * ----------
 * Recursively deletes all nodes in the tree.
 *
 * @return bool True if successful.
 */
bool CNetworkTree::DeleteTree()
{
	for (int i = 0; i <= BALANCED_TREE_ORDER; i++)
	{
		if (m_ptrLink[i] != nullptr)
		{
			delete m_ptrLink[i];
			m_ptrLink[i] = nullptr;
		}
	}
	return true;
}

/**
 * SearchNode
 * ----------
 * Searches for a node by key and retrieves its file path.
 *
 * @param nKey        The key to search for.
 * @param strFilePath Output parameter for the file path.
 * @return bool       True if found, false otherwise.
 */
bool CNetworkTree::SearchNode(int nKey, CString& strFilePath)
{
	ASSERT(GetSize() == (UINT)m_listKey.GetCount());
	ASSERT(GetSize() == (UINT)m_listCode.GetCount());

	UINT i = 0;
	while ((i < GetSize()) && (nKey > m_listKey.GetAt(i)))
		i++;

	if ((i < GetSize()) && (nKey == m_listKey.GetAt(i)))
	{
		strFilePath = m_listCode.GetAt(i);
		return true;
	}

	if (GetLeaf())
		return false;

	ASSERT(m_ptrLink[i] != nullptr);
	return m_ptrLink[i]->SearchNode(nKey, strFilePath);
}

///////////////////////////////////////////////////////////////////////////////
// CNetworkHash
///////////////////////////////////////////////////////////////////////////////

/**
 * CNetworkHash
 * ------------
 * Implements a hash table for fast access to network items.
 * Inherits from CNetworkBase.
 */
CNetworkHash::CNetworkHash()
{
	m_mapFileCode.InitHashTable(MAX_RANDOM_DATA);
	m_mapFilePath.InitHashTable(MAX_RANDOM_DATA);
}

CNetworkHash::~CNetworkHash()
{
}

/**
 * RemoveAll
 * ---------
 * Clears all keys and hash tables.
 */
void CNetworkHash::RemoveAll()
{
	m_listKey.RemoveAll();
	m_mapFileCode.RemoveAll();
	m_mapFilePath.RemoveAll();
}

/**
 * ImportData
 * ----------
 * Imports data from a file list into the hash table.
 *
 * @return bool True if successful, false otherwise.
 */
bool CNetworkHash::ImportData()
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
	}
	catch (CFileException* pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return false;
	}
	return true;
}

/**
 * ExportData
 * ----------
 * Exports hash table data to a file list.
 *
 * @return bool True if successful, false otherwise.
 */
bool CNetworkHash::ExportData()
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
	}
	catch (CFileException* pFileException) {
		TCHAR lpszFileException[0x1000];
		VERIFY(pFileException->GetErrorMessage(lpszFileException, 0x1000));
		TRACE(_T("%s\n"), lpszFileException);
		pFileException->Delete();
		return false;
	}
	return true;
}

/**
 * GenerateID
 * ----------
 * Generates a unique ID not present in the hash table.
 *
 * @return int The generated unique ID.
 */
int CNetworkHash::GenerateID()
{
	int nLevel = 0;
	CString strFilePath;
	int nKey = GetNextID();
	while (SearchItem(nKey, nLevel, strFilePath))
		nKey = GetNextID();
	return nKey;
}

/**
 * SearchItem
 * ----------
 * Searches for an item by key and retrieves its level and file path.
 *
 * @param nKey        The key to search for.
 * @param nLevel      Output parameter for the item's level.
 * @param strFilePath Output parameter for the file path.
 * @return bool       True if found, false otherwise.
 */
bool CNetworkHash::SearchItem(int nKey, int& nLevel, CString& strFilePath)
{
	bool bFileCode = m_mapFileCode.Lookup(nKey, nLevel);
	bool bFilePath = m_mapFilePath.Lookup(nKey, strFilePath);
	return (bFileCode && bFilePath);
}

/**
 * UpdateItem
 * ----------
 * Updates the level and file path for a given key.
 *
 * @param nKey        The key to update.
 * @param nLevel      The new level.
 * @param strFilePath The new file path.
 * @return bool       True if successful.
 */
bool CNetworkHash::UpdateItem(int nKey, int nLevel, CString strFilePath)
{
	m_mapFileCode.SetAt(nKey, nLevel);
	m_mapFilePath.SetAt(nKey, strFilePath);
	return true;
}

/**
 * InsertItem
 * ----------
 * Inserts a new item into the hash table.
 *
 * @param nKey        The key to insert.
 * @param nLevel      The item's level.
 * @param strFilePath The file path.
 * @return bool       True if successful.
 */
bool CNetworkHash::InsertItem(int nKey, int nLevel, CString strFilePath)
{
	m_listKey.Add(nKey);
	m_mapFileCode.SetAt(nKey, nLevel);
	m_mapFilePath.SetAt(nKey, strFilePath);
	return true;
}

/**
 * DeleteItem
 * ----------
 * Deletes an item from the hash table and updates file paths as needed.
 *
 * @param nKey The key to delete.
 * @return bool True if successful.
 */
bool CNetworkHash::DeleteItem(int nKey)
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
	bool bFileCode = m_mapFileCode.RemoveKey(nKey);
	bool bFilePath = m_mapFilePath.RemoveKey(nKey);
	return (bFileCode && bFilePath);
}
