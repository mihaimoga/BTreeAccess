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

// SiatelHomeworkExt.h : header file
//

#ifndef __SIATELHOMEWORKEXT__
#define __SIATELHOMEWORKEXT__

#pragma once

#include "stdafx.h"

//------------------------------------------------------------------------------
// Constants and Macros
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Type Definitions
//------------------------------------------------------------------------------

typedef CArray<int, int> CIntegerArray;                ///< Array of integers.
typedef CMap<int, int, int, int> CFileCodeMap;         ///< Map from int to int (file code to level).
typedef CMap<int, int, CString, CString> CFilePathMap; ///< Map from int to CString (file code to file path).

const static CString strNetworkDigitString = _T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"); ///< Allowed digits for network codes.
const static int intNetworkDigitLength = (int)_tcslen(strNetworkDigitString);           ///< Length of the digit string.

//------------------------------------------------------------------------------
// CNetworkBase
//------------------------------------------------------------------------------

/**
 * @class CNetworkBase
 * @brief Base class providing utility functions for file and network code management.
 *
 * Handles file existence checks, code validation, encoding/decoding of network IDs,
 * and path formatting for networked file storage.
 */
class CNetworkBase : public CObject {
	DECLARE_DYNAMIC(CNetworkBase)

public:
	CNetworkBase();
	~CNetworkBase();

private:
	CString m_strFileList;   ///< Path to the file list.
	CString m_strRootFolder; ///< Root folder for network files.

public:
	/// Gets the file list path.
	inline CString GetFileList() { return m_strFileList; };
	/// Sets the file list path.
	inline void SetFileList(CString strFileList) { m_strFileList = strFileList; };

	/// Gets the root folder path.
	inline CString GetRootFolder() { return m_strRootFolder; };
	/// Sets the root folder path.
	inline void SetRootFolder(CString strRootFolder) { m_strRootFolder = strRootFolder; };

	/**
	 * @brief Formats a Windows error code as a string.
	 * @param dwLastError The error code.
	 * @return Formatted error message.
	 */
	CString FormatLastError(DWORD dwLastError);

	/**
	 * @brief Checks if a file exists at the given path.
	 * @param strFilename File path.
	 * @return True if file exists, false otherwise.
	 */
	bool FileExists(CString strFilename);

	/**
	 * @brief Validates if the code contains only allowed network digits.
	 * @param strFileCode Code string to validate.
	 * @return True if valid, false otherwise.
	 */
	bool IsValidCode(CString strFileCode);

	/**
	 * @brief Encodes an integer key as a network code string.
	 * @param nKey Integer key.
	 * @return Encoded string.
	 */
	CString EncodeNetworkID(int nKey);

	/**
	 * @brief Decodes a network code string to its integer key.
	 * @param strKey Encoded string.
	 * @return Decoded integer key.
	 */
	int DecodeNetworkID(CString strKey);

	/**
	 * @brief Generates a pseudo-random ID for new network entries.
	 * @return Generated ID.
	 */
	int GetNextID();

	/**
	 * @brief Formats a folder name based on the network key.
	 * @param nKey Network key.
	 * @return Folder name.
	 */
	CString FormatFolder(int nKey);

	/**
	 * @brief Formats a filename (with .dat extension) based on the network key.
	 * @param nKey Network key.
	 * @return Filename.
	 */
	CString FormatFilename(int nKey);

	/**
	 * @brief Constructs the full file path for a given key, updating the folder array.
	 * @param nKey Network key.
	 * @param arrFilePath Array to receive folder path components.
	 * @return Full file path.
	 */
	CString GetFilePath(int nKey, CStringArray& arrFilePath);

	/**
	 * @brief Ensures the directory structure exists for the given root and subfolders.
	 * @param strRootFolder Root directory.
	 * @param strSubFolders Array of subfolder names.
	 * @return True if the path exists or was created, false otherwise.
	 */
	bool CreateNetworkPath(CString strRootFolder, CStringArray& strSubFolders);
};

//------------------------------------------------------------------------------
// CNetworkTree
//------------------------------------------------------------------------------

/**
 * @class CNetworkTree
 * @brief Implements a B-Tree-like structure for managing network nodes.
 *
 * Stores file keys and paths in a balanced tree for efficient search and management.
 */
class CNetworkTree : public CNetworkBase {
	DECLARE_DYNAMIC(CNetworkTree)

public:
	CNetworkTree();
	~CNetworkTree();

protected:
	bool m_boolLeaf;                                 ///< True if this node is a leaf.
	UINT m_uintSize;                                 ///< Number of keys in this node.
	CNetworkTree* m_ptrRoot;                        ///< Pointer to the root node.
	CIntegerArray m_listKey;                         ///< Array of keys (random integers).
	CStringArray m_listCode;                         ///< Array of file paths.
	CNetworkTree* m_ptrLink[BALANCED_TREE_ORDER + 1]; ///< Child pointers.

public:
	/// Gets the child link at the specified index.
	inline CNetworkTree* GetLink(int nIndex) { return m_ptrLink[nIndex]; }
	/// Sets the child link at the specified index.
	inline void SetLink(int nIndex, CNetworkTree* pTree) { m_ptrLink[nIndex] = pTree; }

	/// Gets whether this node is a leaf.
	inline bool GetLeaf() { return m_boolLeaf; }
	/// Sets whether this node is a leaf.
	inline void SetLeaf(bool bLeaf) { m_boolLeaf = bLeaf; }

	/// Gets the number of keys in this node.
	inline UINT GetSize() { return m_uintSize; }
	/// Sets the number of keys in this node.
	inline void SetSize(UINT nSize) { m_uintSize = nSize; }

	/// Gets the root node pointer.
	inline CNetworkTree* GetRoot() { return m_ptrRoot; }
	/// Sets the root node pointer.
	inline void SetRoot(CNetworkTree* pRoot) { m_ptrRoot = pRoot; }

	/**
	 * @brief Imports data into the tree structure.
	 * @return True if successful.
	 */
	bool ImportData();

	/**
	 * @brief Exports data from the tree structure.
	 * @return True if successful.
	 */
	bool ExportData();

	/**
	 * @brief Generates a unique ID not present in the tree.
	 * @return Generated unique ID.
	 */
	int GenerateID();

	/**
	 * @brief Initializes the tree structure.
	 * @return True if successful.
	 */
	bool CreateTree();

	/**
	 * @brief Recursively deletes all nodes in the tree.
	 * @return True if successful.
	 */
	bool DeleteTree();

	/**
	 * @brief Searches for a node by key and retrieves its file path.
	 * @param nKey Key to search for.
	 * @param strFilePath Output parameter for the file path.
	 * @return True if found, false otherwise.
	 */
	bool SearchNode(int nKey, CString& strFilePath);

	/* Uncommented methods for future use:
	bool UpdateNode(int nKey, CString strFilePath);
	bool InsertNode(int nKey, CString strFilePath);
	bool InsertEmpty(int nKey, CString strFilePath);
	*/
};

//------------------------------------------------------------------------------
// CNetworkHash
//------------------------------------------------------------------------------

/**
 * @class CNetworkHash
 * @brief Implements a hash table for fast access to network items.
 *
 * Stores file keys, levels, and paths in hash maps for efficient lookup and management.
 */
class CNetworkHash : public CNetworkBase {
	DECLARE_DYNAMIC(CNetworkHash)

public:
	CNetworkHash();
	~CNetworkHash();

protected:
	CIntegerArray m_listKey; ///< Array of unique keys.
	CFileCodeMap m_mapFileCode; ///< Map from key to level.
	CFilePathMap m_mapFilePath; ///< Map from key to file path.

public:
	/// Gets the number of items in the hash.
	inline int GetSize() { return (int)m_listKey.GetCount(); }
	/// Gets the key at the specified index.
	inline int GetKey(int nIndex) { return m_listKey.GetAt(nIndex); }

	/**
	 * @brief Removes all items from the hash.
	 */
	void RemoveAll();

	/**
	 * @brief Imports data into the hash structure.
	 * @return True if successful.
	 */
	bool ImportData();

	/**
	 * @brief Exports data from the hash structure.
	 * @return True if successful.
	 */
	bool ExportData();

	/**
	 * @brief Generates a unique ID not present in the hash.
	 * @return Generated unique ID.
	 */
	int GenerateID();

	/**
	 * @brief Searches for an item by key and retrieves its level and file path.
	 * @param nKey Key to search for.
	 * @param nLevel Output parameter for the item's level.
	 * @param strFilePath Output parameter for the file path.
	 * @return True if found, false otherwise.
	 */
	bool SearchItem(int nKey, int& nLevel, CString& strFilePath);

	/**
	 * @brief Updates the level and file path for a given key.
	 * @param nKey Key to update.
	 * @param nLevel New level.
	 * @param strFilePath New file path.
	 * @return True if successful.
	 */
	bool UpdateItem(int nKey, int nLevel, CString strFilePath);

	/**
	 * @brief Inserts a new item into the hash structure.
	 * @param nKey Key to insert.
	 * @param nLevel Item's level.
	 * @param strFilePath File path.
	 * @return True if successful.
	 */
	bool InsertItem(int nKey, int nLevel, CString strFilePath);

	/**
	 * @brief Deletes an item from the hash structure.
	 * @param nKey Key to delete.
	 * @return True if successful.
	 */
	bool DeleteItem(int nKey);
};

#endif // __SIATELHOMEWORKEXT__
