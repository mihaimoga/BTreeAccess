/* This file is part of BTreeAccess application developed by Stefan-Mihai MOGA.

BTreeAccess is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Open
Source Initiative, either version 3 of the License, or any later version.

BTreeAccess is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BTreeAccess. If not, see <http://www.opensource.org/licenses/gpl-3.0.html>*/

// SiatelHomeworkExt.h : header file
//

#ifndef __SIATELHOMEWORKEXT__
#define __SIATELHOMEWORKEXT__

#pragma once

#include "stdafx.h"

#define USE_HASH_METHOD TRUE /* TRUE = CNetworkHash, FALSE = CNetworkTree */

#define SLEEP_INTERVAL 0x0400
#define PROGRESS_RANGE 0x1000
#define BALANCED_TREE_ERROR -1
#define BALANCED_TREE_ORDER 4 // Should be 1000!
#define MAX_CODE_LENGTH 0x000004 /* Generated Code Length */
#define MAX_RANDOM_DATA 0x010001 /* Max Random ID */

#define EMPTY_ROOT_FOLDER _T("Please select root folder in order to have access to upload, download and delete ...")
#define READY_MESSAGE _T("Ready to process your requests ...")
#define INVALID_CHARS _T("The provided code has invalid characters ...")
#define FILE_DOES_NOT_EXISTS _T("There is no file on disk associated with the code you provided ...")

typedef CArray<int, int> CIntegerArray;
typedef CMap<int, int, int, int> CFileCodeMap;
typedef CMap<int, int, CString, CString> CFilePathMap;

const static CString strNetworkDigitString = _T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
const static int intNetworkDigitLength = _tcslen(strNetworkDigitString);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkBase ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CNetworkBase : public CObject {
	DECLARE_DYNAMIC(CNetworkBase)

public:
	CNetworkBase();
	~CNetworkBase();

private:
	CString m_strFileList;
	CString m_strRootFolder;

public:
	inline CString GetFileList() { return m_strFileList; };
	inline void SetFileList(CString strFileList) { m_strFileList = strFileList; };

	inline CString GetRootFolder() { return m_strRootFolder; };
	inline void SetRootFolder(CString strRootFolder) { m_strRootFolder = strRootFolder; };

	CString FormatLastError(DWORD dwLastError);

	BOOL FileExists(CString strFilename);

	BOOL IsValidCode(CString strFileCode);
	CString EncodeNetworkID(int nKey);
	int DecodeNetworkID(CString strKey);

	int GetNextID();

	CString FormatFolder(int nKey);
	CString FormatFilename(int nKey);

	CString GetFilePath(int nKey, CStringArray & arrFilePath);
	BOOL CreateNetworkPath(CString strRootFolder, CStringArray & strSubFolders);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkTree ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CNetworkTree : public CNetworkBase {
	DECLARE_DYNAMIC(CNetworkTree)

public:
	CNetworkTree();
	~CNetworkTree();

protected:
	BOOL m_boolLeaf;
	UINT m_uintSize;
	CNetworkTree * m_ptrRoot;
	CIntegerArray m_listKey; // random integer
	CStringArray m_listCode; // file path on disk
	CNetworkTree * m_ptrLink[BALANCED_TREE_ORDER+1];

public:
	inline CNetworkTree * GetLink(int nIndex) { return m_ptrLink[nIndex]; }
	inline void SetLink(int nIndex, CNetworkTree * pTree) { m_ptrLink[nIndex] = pTree; }

	inline BOOL GetLeaf() { return m_boolLeaf; } 
	inline void SetLeaf(BOOL bLeaf) { m_boolLeaf = bLeaf; }

	inline UINT GetSize() { return m_uintSize; } 
	inline void SetSize(UINT nSize) { m_uintSize = nSize; }

	inline CNetworkTree * GetRoot() { return m_ptrRoot; } 
	inline void SetRoot(CNetworkTree * pRoot) { m_ptrRoot = pRoot; }

	BOOL ImportData();
	BOOL ExportData();

	int GenerateID();

	BOOL CreateTree();
	BOOL DeleteTree();

	BOOL SearchNode(int nKey, CString & strFilePath);
	/*BOOL UpdateNode(int nKey, CString strFilePath);

	BOOL InsertNode(int nKey, CString strFilePath);
	BOOL InsertEmpty(int nKey, CString strFilePath);*/

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CNetworkHash ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CNetworkHash : public CNetworkBase {
	DECLARE_DYNAMIC(CNetworkHash)

public:
	CNetworkHash();
	~CNetworkHash();

protected:
	CIntegerArray m_listKey; // unique integer array
	CFileCodeMap m_mapFileCode;
	CFilePathMap m_mapFilePath;

public:
	inline int GetSize() { return m_listKey.GetCount(); } 
	inline int GetKey(int nIndex) { return m_listKey.GetAt(nIndex); };

	void RemoveAll();

	BOOL ImportData();
	BOOL ExportData();

	int GenerateID();

	BOOL SearchItem(int nKey, int & nLevel, CString & strFilePath);
	BOOL UpdateItem(int nKey, int nLevel, CString strFilePath);

	BOOL InsertItem(int nKey, int nLevel, CString strFilePath);
	BOOL DeleteItem(int nKey);
};

#endif // __SIATELHOMEWORKEXT__
