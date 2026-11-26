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

// TestHelpers.h : helpers shared by the BTreeAccess unit tests
//

#ifndef __TESTHELPERS__
#define __TESTHELPERS__

#pragma once

#include "../stdafx.h"
#include "CppUnitTest.h"
#include "../SiatelHomeworkExt.h"

#include <map>
#include <string>
#include <vector>

namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			/// Lets Assert::AreEqual print CString values in failure messages.
			template<> inline std::wstring ToString<CString>(const CString& strValue)
			{
				return std::wstring(static_cast<LPCWSTR>(strValue));
			}
		}
	}
}

namespace TestHelpers
{
	/**
	 * @class CTempFolder
	 * @brief Creates a unique folder under %TEMP% and removes it again on destruction.
	 *
	 * CNetworkBase::CreateNetworkPath changes the current directory of the process, which
	 * would keep the folder locked, so the destructor restores the original directory first.
	 */
	class CTempFolder
	{
	public:
		CTempFolder();
		~CTempFolder();

		CTempFolder(const CTempFolder&) = delete;
		CTempFolder& operator=(const CTempFolder&) = delete;

		/// Gets the folder path, always terminated by a backslash.
		CString GetPath() const;
		/// Gets the full path of a (not necessarily existing) entry directly inside the folder.
		CString GetEntry(LPCTSTR lpszName) const;

	private:
		std::filesystem::path m_pathFolder;
		std::filesystem::path m_pathSavedCurrent;
	};

	/// Creates (or truncates) a binary file with the given content.
	void WriteFile(const CString& strPath, const std::string& strContent);
	/// Reads a whole file; an unreadable file yields an empty string.
	std::string ReadFile(const CString& strPath);
	/// Reads a text file as lines; handles both LF and CRLF terminators.
	std::vector<CString> ReadLines(const CString& strPath);

	/// Checks whether a file or directory exists, independently of the code under test.
	bool PathExists(const CString& strPath);
	/// Checks whether the path is an existing directory.
	bool IsDirectory(const CString& strPath);
	/// Counts the *.dat files below a folder (recursively).
	size_t CountDataFiles(const CString& strFolder);

	/// Joins the elements of an array using a separator.
	CString Join(const CStringArray& arrItems, LPCTSTR lpszSeparator);
	/// Deterministic content that is different for every key, used to prove the right file is returned.
	std::string MakeContent(int nKey);

	/**
	 * Stores a file the way the Upload button of the main dialog does: the current item count is the
	 * next free level and selects the path, the missing folders are created, the content of the key
	 * is written and the item is registered. The root folder of the hash must be set and end with a backslash.
	 * @return The level of the new item.
	 */
	int StoreFile(CNetworkHash& hash, int nKey);
	/// Deletes a file the way the Delete button does: the physical file goes first, then the index entry.
	bool RemoveStoredFile(CNetworkHash& hash, int nKey);
	/**
	 * Fails the current test unless the index and the disk agree with each other: every key is listed once and
	 * found, the levels are a permutation of 0..size-1, each path is the one GetFilePath gives for the level,
	 * each file holds the content of its key and there are no other data files below the root folder.
	 */
	void VerifyNetwork(CNetworkHash& hash);
	/**
	 * Same checks, but for files whose content is not derived from the key: the indexed keys must be exactly
	 * the keys of the map and every file must hold the text the map gives for its key.
	 */
	void VerifyNetwork(CNetworkHash& hash, const std::map<int, std::string>& mapContent);

	/**
	 * @class CAssertionMonitor
	 * @brief Turns CRT/MFC assertions of the code under test into test failures.
	 *
	 * In Debug builds a failed ASSERT/ATLASSERT opens a modal dialog, which would block an
	 * unattended test run forever. The monitor swallows the report and counts it instead,
	 * and VerifyNone() fails the current test when an assertion fired. It does nothing in
	 * Release builds, where those assertions are compiled out.
	 */
	class CAssertionMonitor
	{
	public:
		static void Install();
		static void Uninstall();
		static void Reset();
		static int GetCount();
		static CString GetFirstMessage();
		static void VerifyNone();
	};

	/**
	 * @class CMessageBoxMonitor
	 * @brief Turns the modal message boxes of the code under test into recorded events.
	 *
	 * A message box that nobody answers blocks a test run forever; CNetworkHash::DeleteItem shows one when it
	 * cannot move a file. While a thread is watched, every message box of that thread is closed with IDOK as soon
	 * as it is activated, and its caption and text are recorded. VerifyNone() fails the current test when a message
	 * box was shown, so a test that provokes one on purpose reads the records and then calls Clear().
	 */
	class CMessageBoxMonitor
	{
	public:
		/// Starts watching the calling thread and forgets the earlier records.
		static void Begin();
		/// Stops watching.
		static void End();
		/// Forgets the recorded message boxes.
		static void Clear();
		static int GetCount();
		static CString GetFirstText();
		static CString GetFirstCaption();
		static void VerifyNone();
	};
}

/// Place inside a TEST_CLASS to fail every test of that class that triggers an unexpected assertion or message box.
#define GUARD_AGAINST_UNEXPECTED_POPUPS() \
	TEST_METHOD_INITIALIZE(PopupGuardSetup) \
	{ \
		TestHelpers::CAssertionMonitor::Reset(); \
		TestHelpers::CMessageBoxMonitor::Begin(); \
	} \
	TEST_METHOD_CLEANUP(PopupGuardTeardown) \
	{ \
		TestHelpers::CMessageBoxMonitor::End(); \
		TestHelpers::CMessageBoxMonitor::VerifyNone(); \
		TestHelpers::CAssertionMonitor::VerifyNone(); \
	}

#endif // __TESTHELPERS__
