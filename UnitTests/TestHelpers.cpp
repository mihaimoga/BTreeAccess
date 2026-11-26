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

// TestHelpers.cpp : implementation of the helpers shared by the BTreeAccess unit tests
//

#include "stdafx.h"
#include "TestHelpers.h"

#include <atomic>
#include <fstream>
#include <functional>
#include <map>
#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestHelpers
{
	namespace
	{
		std::atomic<unsigned int> g_nFolderCounter(0);
	}

	///////////////////////////////////////////////////////////////////////////
	// CTempFolder
	///////////////////////////////////////////////////////////////////////////

	CTempFolder::CTempFolder()
	{
		std::error_code ecError;
		m_pathSavedCurrent = std::filesystem::current_path(ecError);
		const std::filesystem::path pathTemp = std::filesystem::temp_directory_path(ecError);
		Assert::IsFalse(static_cast<bool>(ecError), L"Unable to locate the temporary directory");

		// The name is unique per process and per instance; retry in case a stale folder already uses it.
		for (int nAttempt = 0; nAttempt < 100; nAttempt++)
		{
			std::wostringstream strName;
			strName << L"BTreeAccessTests_" << GetCurrentProcessId() << L'_' << g_nFolderCounter++ << L'_' << GetTickCount64();
			const std::filesystem::path pathCandidate = pathTemp / strName.str();
			ecError.clear();
			if (std::filesystem::create_directory(pathCandidate, ecError))
			{
				m_pathFolder = pathCandidate;
				return;
			}
		}
		Assert::Fail(L"Unable to create a temporary folder");
	}

	CTempFolder::~CTempFolder()
	{
		std::error_code ecError;
		// CreateNetworkPath leaves the process inside the folder, which would block its deletion.
		std::filesystem::current_path(m_pathSavedCurrent, ecError);
		std::filesystem::remove_all(m_pathFolder, ecError);
	}

	CString CTempFolder::GetPath() const
	{
		CString strPath(m_pathFolder.c_str());
		strPath += _T('\\');
		return strPath;
	}

	CString CTempFolder::GetEntry(LPCTSTR lpszName) const
	{
		return GetPath() + lpszName;
	}

	///////////////////////////////////////////////////////////////////////////
	// File helpers
	///////////////////////////////////////////////////////////////////////////

	void WriteFile(const CString& strPath, const std::string& strContent)
	{
		std::ofstream streamFile(std::filesystem::path(static_cast<LPCWSTR>(strPath)), std::ios::binary | std::ios::trunc);
		if (!streamFile.is_open())
			Assert::Fail((std::wstring(L"Unable to write the test file ") + static_cast<LPCWSTR>(strPath)).c_str());
		streamFile.write(strContent.data(), static_cast<std::streamsize>(strContent.size()));
		streamFile.close();
		Assert::IsFalse(streamFile.fail(), L"Unable to write the content of a test file");
	}

	std::string ReadFile(const CString& strPath)
	{
		std::ifstream streamFile(std::filesystem::path(static_cast<LPCWSTR>(strPath)), std::ios::binary);
		std::ostringstream streamContent;
		if (streamFile.is_open())
			streamContent << streamFile.rdbuf();
		return streamContent.str();
	}

	std::vector<CString> ReadLines(const CString& strPath)
	{
		std::vector<CString> arrLines;
		const std::string strContent = ReadFile(strPath);
		size_t nStart = 0;
		while (nStart < strContent.size())
		{
			size_t nEnd = strContent.find('\n', nStart);
			if (nEnd == std::string::npos)
				nEnd = strContent.size();
			size_t nLength = nEnd - nStart;
			if ((nLength > 0) && (strContent[nStart + nLength - 1] == '\r'))
				nLength--;
			arrLines.push_back(CString(strContent.substr(nStart, nLength).c_str()));
			nStart = nEnd + 1;
		}
		return arrLines;
	}

	bool PathExists(const CString& strPath)
	{
		std::error_code ecError;
		return std::filesystem::exists(std::filesystem::path(static_cast<LPCWSTR>(strPath)), ecError);
	}

	bool IsDirectory(const CString& strPath)
	{
		std::error_code ecError;
		return std::filesystem::is_directory(std::filesystem::path(static_cast<LPCWSTR>(strPath)), ecError);
	}

	size_t CountDataFiles(const CString& strFolder)
	{
		size_t nCount = 0;
		std::error_code ecError;
		const std::filesystem::recursive_directory_iterator itEnd;
		for (std::filesystem::recursive_directory_iterator itEntry(std::filesystem::path(static_cast<LPCWSTR>(strFolder)), ecError);
			(!ecError) && (itEntry != itEnd); itEntry.increment(ecError))
		{
			if (itEntry->is_regular_file(ecError) && (itEntry->path().extension() == L".dat"))
				nCount++;
		}
		return nCount;
	}

	CString Join(const CStringArray& arrItems, LPCTSTR lpszSeparator)
	{
		CString strJoined;
		const INT_PTR nCount = arrItems.GetCount();
		for (INT_PTR nIndex = 0; nIndex < nCount; nIndex++)
		{
			if (nIndex > 0)
				strJoined += lpszSeparator;
			strJoined += arrItems.GetAt(nIndex);
		}
		return strJoined;
	}

	std::string MakeContent(int nKey)
	{
		std::string strContent = "BTreeAccess test payload for key " + std::to_string(nKey) + ":";
		// The size varies with the key, so a file moved onto the wrong slot cannot go unnoticed.
		for (int nRepeat = 0; nRepeat <= nKey % 5; nRepeat++)
			strContent += " data-" + std::to_string(nKey);
		return strContent;
	}

	///////////////////////////////////////////////////////////////////////////
	// Network helpers
	///////////////////////////////////////////////////////////////////////////

	int StoreFile(CNetworkHash& hash, int nKey)
	{
		CStringArray arrFolders;
		const int nLevel = hash.GetSize();
		const CString strFilePath = hash.GetFilePath(nLevel, arrFolders);
		Assert::IsTrue(hash.CreateNetworkPath(hash.GetRootFolder(), arrFolders), L"Unable to create the folders of a stored file");
		WriteFile(hash.GetRootFolder() + strFilePath, MakeContent(nKey));
		Assert::IsTrue(hash.InsertItem(nKey, nLevel, strFilePath), L"Unable to register a stored file");
		return nLevel;
	}

	bool RemoveStoredFile(CNetworkHash& hash, int nKey)
	{
		int nLevel = 0;
		CString strFilePath;
		if (hash.SearchItem(nKey, nLevel, strFilePath))
		{
			// DeleteItem moves another file into the freed slot, so the deleted file must already be gone.
			Assert::IsTrue(DeleteFile(hash.GetRootFolder() + strFilePath) != FALSE, L"Unable to delete the physical file");
		}
		return hash.DeleteItem(nKey);
	}

	namespace
	{
		// The callback gives the content a key must have on disk.
		void VerifyNetworkImpl(CNetworkHash& hash, const std::function<std::string(int)>& fnExpectedContent)
		{
			const CString strRoot = hash.GetRootFolder();
			const int nSize = hash.GetSize();
			std::vector<bool> arrLevelUsed(static_cast<size_t>(nSize), false);
			std::set<int> setKeys;

			for (int nIndex = 0; nIndex < nSize; nIndex++)
			{
				const int nKey = hash.GetKey(nIndex);
				int nLevel = -1;
				CString strFilePath;
				const auto fnCheck = [&](bool bCondition, LPCTSTR lpszProblem)
				{
					if (!bCondition)
					{
						CString strMessage;
						strMessage.Format(_T("Key %d (listed at index %d of %d) %s"), nKey, nIndex, nSize, lpszProblem);
						Assert::Fail(strMessage);
					}
				};

				fnCheck(setKeys.insert(nKey).second, _T("is listed more than once"));
				fnCheck(hash.SearchItem(nKey, nLevel, strFilePath), _T("is listed but cannot be found"));
				fnCheck((nLevel >= 0) && (nLevel < nSize), _T("has a level outside 0..size-1"));
				fnCheck(!arrLevelUsed[static_cast<size_t>(nLevel)], _T("shares its level with another key"));
				arrLevelUsed[static_cast<size_t>(nLevel)] = true;

				CStringArray arrFolders;
				fnCheck(hash.GetFilePath(nLevel, arrFolders) == strFilePath, _T("is not stored at the path of its level"));
				fnCheck(PathExists(strRoot + strFilePath), _T("has no file on disk"));
				fnCheck(ReadFile(strRoot + strFilePath) == fnExpectedContent(nKey), _T("has the content of another file"));
			}

			Assert::AreEqual(static_cast<size_t>(nSize), CountDataFiles(strRoot), L"The number of data files on disk differs from the number of indexed items");
		}
	}

	void VerifyNetwork(CNetworkHash& hash)
	{
		VerifyNetworkImpl(hash, [](int nKey) { return MakeContent(nKey); });
	}

	void VerifyNetwork(CNetworkHash& hash, const std::map<int, std::string>& mapContent)
	{
		const int nSize = hash.GetSize();
		std::set<int> setIndexed;
		for (int nIndex = 0; nIndex < nSize; nIndex++)
			setIndexed.insert(hash.GetKey(nIndex));

		for (const auto& pairEntry : mapContent)
		{
			if (setIndexed.count(pairEntry.first) == 0)
			{
				CString strMessage;
				strMessage.Format(_T("Key %d is expected in the index but is missing"), pairEntry.first);
				Assert::Fail(strMessage);
			}
		}
		for (const int nKey : setIndexed)
		{
			if (mapContent.count(nKey) == 0)
			{
				CString strMessage;
				strMessage.Format(_T("Key %d is in the index but was not expected"), nKey);
				Assert::Fail(strMessage);
			}
		}

		VerifyNetworkImpl(hash, [&mapContent](int nKey) { return mapContent.at(nKey); });
	}

	///////////////////////////////////////////////////////////////////////////
	// CAssertionMonitor
	///////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	namespace
	{
		std::atomic<int> g_nAssertionCount(0);
		wchar_t g_szFirstMessage[512] = L"";
		SRWLOCK g_lockFirstMessage = SRWLOCK_INIT;
		int g_nSavedAssertMode = 0;
		int g_nSavedErrorMode = 0;

		void RecordAssertion(const wchar_t* lpszMessage)
		{
			if (g_nAssertionCount.fetch_add(1) == 0)
			{
				AcquireSRWLockExclusive(&g_lockFirstMessage);
				wcsncpy_s(g_szFirstMessage, lpszMessage, _TRUNCATE);
				ReleaseSRWLockExclusive(&g_lockFirstMessage);
			}
		}

		// The hooks must neither throw nor allocate, because the CRT calls them while holding its report lock.
		int __cdecl ReportHookA(int nReportType, char* lpszMessage, int* pnReturnValue)
		{
			if ((nReportType != _CRT_ASSERT) && (nReportType != _CRT_ERROR))
				return FALSE;

			wchar_t szMessage[512];
			size_t nIndex = 0;
			while ((lpszMessage != nullptr) && (lpszMessage[nIndex] != '\0') && (nIndex < _countof(szMessage) - 1))
			{
				szMessage[nIndex] = static_cast<unsigned char>(lpszMessage[nIndex]);
				nIndex++;
			}
			szMessage[nIndex] = L'\0';
			RecordAssertion(szMessage);

			if (pnReturnValue != nullptr)
				*pnReturnValue = 0; // do not break into the debugger
			return TRUE;
		}

		int __cdecl ReportHookW(int nReportType, wchar_t* lpszMessage, int* pnReturnValue)
		{
			if ((nReportType != _CRT_ASSERT) && (nReportType != _CRT_ERROR))
				return FALSE;

			RecordAssertion((lpszMessage != nullptr) ? lpszMessage : L"");

			if (pnReturnValue != nullptr)
				*pnReturnValue = 0; // do not break into the debugger
			return TRUE;
		}
	}
#endif

	void CAssertionMonitor::Install()
	{
#ifdef _DEBUG
		// MFC asserts go through the ANSI report function, ATL asserts through the wide one.
		const int nHooksA = _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, ReportHookA);
		const int nHooksW = _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, ReportHookW);
		Assert::IsTrue((nHooksA != -1) && (nHooksW != -1), L"Unable to install the assertion monitor");

		// Anything the hooks do not handle must still never open the modal assertion dialog.
		g_nSavedAssertMode = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
		g_nSavedErrorMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		Reset();
#endif
	}

	void CAssertionMonitor::Uninstall()
	{
#ifdef _DEBUG
		_CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, ReportHookA);
		_CrtSetReportHookW2(_CRT_RPTHOOK_REMOVE, ReportHookW);
		_CrtSetReportMode(_CRT_ASSERT, g_nSavedAssertMode);
		_CrtSetReportMode(_CRT_ERROR, g_nSavedErrorMode);
#endif
	}

	void CAssertionMonitor::Reset()
	{
#ifdef _DEBUG
		g_nAssertionCount = 0;
		AcquireSRWLockExclusive(&g_lockFirstMessage);
		g_szFirstMessage[0] = L'\0';
		ReleaseSRWLockExclusive(&g_lockFirstMessage);
#endif
	}

	int CAssertionMonitor::GetCount()
	{
#ifdef _DEBUG
		return g_nAssertionCount;
#else
		return 0;
#endif
	}

	CString CAssertionMonitor::GetFirstMessage()
	{
		CString strMessage;
#ifdef _DEBUG
		AcquireSRWLockShared(&g_lockFirstMessage);
		strMessage = g_szFirstMessage;
		ReleaseSRWLockShared(&g_lockFirstMessage);
#endif
		return strMessage;
	}

	void CAssertionMonitor::VerifyNone()
	{
		const int nCount = GetCount();
		if (nCount != 0)
		{
			std::wostringstream strMessage;
			strMessage << nCount << L" unexpected assertion(s) fired in the code under test; first: " << static_cast<LPCWSTR>(GetFirstMessage());
			Reset(); // keep the failure from leaking into the next test
			Assert::Fail(strMessage.str().c_str());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// CMessageBoxMonitor
	///////////////////////////////////////////////////////////////////////////

	namespace
	{
		HHOOK g_hMessageBoxHook = nullptr;
		int g_nMessageBoxCount = 0;
		std::wstring g_strFirstCaption;
		std::wstring g_strFirstText;

		// Runs on the watched thread just before one of its windows is activated.
		LRESULT CALLBACK MessageBoxHookProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			if (nCode == HCBT_ACTIVATE)
			{
				const HWND hWnd = reinterpret_cast<HWND>(wParam);
				wchar_t szClass[16] = L"";
				GetClassNameW(hWnd, szClass, _countof(szClass));
				if (wcscmp(szClass, L"#32770") == 0) // the window class of the system dialogs, message boxes included
				{
					wchar_t szCaption[256] = L"";
					wchar_t szText[1024] = L"";
					GetWindowTextW(hWnd, szCaption, _countof(szCaption));
					GetDlgItemTextW(hWnd, 0xFFFF, szText, _countof(szText)); // the static control that holds the message
					if (g_nMessageBoxCount++ == 0)
					{
						g_strFirstCaption = szCaption;
						g_strFirstText = szText;
					}
					EndDialog(hWnd, IDOK);
					return 0;
				}
			}
			return CallNextHookEx(g_hMessageBoxHook, nCode, wParam, lParam);
		}
	}

	void CMessageBoxMonitor::Begin()
	{
		End(); // a hook left behind by an earlier test must not stay around
		Clear();
		g_hMessageBoxHook = SetWindowsHookExW(WH_CBT, MessageBoxHookProc, nullptr, GetCurrentThreadId());
		Assert::IsTrue(g_hMessageBoxHook != nullptr, L"Unable to install the message box monitor");
	}

	void CMessageBoxMonitor::End()
	{
		if (g_hMessageBoxHook != nullptr)
		{
			UnhookWindowsHookEx(g_hMessageBoxHook);
			g_hMessageBoxHook = nullptr;
		}
	}

	void CMessageBoxMonitor::Clear()
	{
		g_nMessageBoxCount = 0;
		g_strFirstCaption.clear();
		g_strFirstText.clear();
	}

	int CMessageBoxMonitor::GetCount()
	{
		return g_nMessageBoxCount;
	}

	CString CMessageBoxMonitor::GetFirstText()
	{
		return CString(g_strFirstText.c_str());
	}

	CString CMessageBoxMonitor::GetFirstCaption()
	{
		return CString(g_strFirstCaption.c_str());
	}

	void CMessageBoxMonitor::VerifyNone()
	{
		if (g_nMessageBoxCount != 0)
		{
			std::wostringstream strMessage;
			strMessage << g_nMessageBoxCount << L" unexpected message box(es) shown by the code under test; first: \"" << g_strFirstText << L"\"";
			Clear(); // keep the failure from leaking into the next test
			Assert::Fail(strMessage.str().c_str());
		}
	}
}
