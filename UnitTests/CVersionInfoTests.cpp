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

// CVersionInfoTests.cpp : unit tests for CVersionInfo
//
// The version resource of the test DLL itself (see UnitTests.rc) is the fixture of most tests,
// so every expected value below has to be kept in sync with that resource script.
//

#include "stdafx.h"
#include "TestHelpers.h"
#include "../VersionInfo.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

extern "C" IMAGE_DOS_HEADER __ImageBase; // The base address of this DLL, which is also its module handle.

namespace UnitTests
{
	namespace
	{
		/// Builds a 64-bit version number out of its four 16-bit parts.
		unsigned __int64 MakeVersion(WORD wMajor, WORD wMinor, WORD wBuild, WORD wRevision)
		{
			return (static_cast<unsigned __int64>(wMajor) << 48) | (static_cast<unsigned __int64>(wMinor) << 32) |
				(static_cast<unsigned __int64>(wBuild) << 16) | static_cast<unsigned __int64>(wRevision);
		}

		/// Converts a value returned by CVersionInfo to the string type that the assertions understand.
		CString Str(const CVersionInfo::String& strValue)
		{
			return CString(strValue.c_str());
		}

		/// The full path of this DLL.
		CString GetModulePath()
		{
			TCHAR szPath[MAX_PATH * 2] = {};
			const DWORD dwLength = ::GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), szPath, static_cast<DWORD>(_countof(szPath)));
			Assert::IsTrue((dwLength > 0) && (dwLength < _countof(szPath)), L"Unable to get the path of the test DLL");
			return CString(szPath);
		}

		/// The full path of a library of the Windows system folder.
		CString GetSystemLibraryPath(LPCTSTR lpszName)
		{
			TCHAR szFolder[MAX_PATH] = {};
			const UINT nLength = ::GetSystemDirectory(szFolder, static_cast<UINT>(_countof(szFolder)));
			Assert::IsTrue((nLength > 0) && (nLength < _countof(szFolder)), L"Unable to get the system folder");
			CString strPath(szFolder);
			strPath += _T('\\');
			strPath += lpszName;
			return strPath;
		}

		/// Loads a file into the object and tells whether version information was found in it.
		bool Loads(CVersionInfo& info, const CString& strPath)
		{
			return info.Load(strPath) != FALSE;
		}

		/// Loads the version resource of the test DLL, which holds the fixture values.
		void LoadFixture(CVersionInfo& info)
		{
			Assert::IsTrue(Loads(info, GetModulePath()), L"Unable to read the version resource of the test DLL");
		}

		using Bytes = std::vector<BYTE>;

		/// Appends a 16-bit value to a byte buffer, least significant byte first.
		void AppendWord(Bytes& arrData, WORD wValue)
		{
			arrData.push_back(static_cast<BYTE>(wValue & 0xFF));
			arrData.push_back(static_cast<BYTE>(wValue >> 8));
		}

		/// Appends a 32-bit value to a byte buffer, least significant byte first.
		void AppendDword(Bytes& arrData, DWORD dwValue)
		{
			AppendWord(arrData, static_cast<WORD>(dwValue & 0xFFFF));
			AppendWord(arrData, static_cast<WORD>(dwValue >> 16));
		}

		/// Appends a text with its terminator and returns the number of characters appended, terminator included.
		WORD AppendString(Bytes& arrData, const std::wstring& strValue)
		{
			for (const wchar_t chChar : strValue)
				AppendWord(arrData, chChar);
			AppendWord(arrData, 0);
			return static_cast<WORD>(strValue.size() + 1);
		}

		/// Pads a byte buffer with zeros up to the next 32-bit boundary.
		void AlignToDword(Bytes& arrData)
		{
			while ((arrData.size() % sizeof(DWORD)) != 0)
				arrData.push_back(0);
		}

		/// Builds one block of a version resource: the header, the key, the value and the child blocks, each starting on a 32-bit boundary.
		Bytes MakeVersionBlock(const std::wstring& strKey, WORD wType, WORD wValueLength, const Bytes& arrValue, const std::vector<Bytes>& arrChildren)
		{
			Bytes arrBlock;
			AppendWord(arrBlock, 0); // the length of the block is known only at the end
			AppendWord(arrBlock, wValueLength);
			AppendWord(arrBlock, wType);
			AppendString(arrBlock, strKey);
			AlignToDword(arrBlock);
			arrBlock.insert(arrBlock.end(), arrValue.begin(), arrValue.end());
			for (const Bytes& arrChild : arrChildren)
			{
				AlignToDword(arrBlock);
				arrBlock.insert(arrBlock.end(), arrChild.begin(), arrChild.end());
			}
			const WORD wLength = static_cast<WORD>(arrBlock.size());
			arrBlock[0] = static_cast<BYTE>(wLength & 0xFF);
			arrBlock[1] = static_cast<BYTE>(wLength >> 8);
			return arrBlock;
		}

		/// One entry of a string table; its value length counts characters.
		Bytes MakeStringBlock(const std::wstring& strName, const std::wstring& strValue)
		{
			Bytes arrValue;
			const WORD wValueLength = AppendString(arrValue, strValue);
			return MakeVersionBlock(strName, 1, wValueLength, arrValue, {});
		}

		/// A version resource whose only string table is the one for the neutral language and the ANSI code page, so it has no translation table.
		Bytes MakeVersionResourceWithoutTranslationTable()
		{
			Bytes arrFixedFileInfo;
			AppendDword(arrFixedFileInfo, VS_FFI_SIGNATURE);
			AppendDword(arrFixedFileInfo, VS_FFI_STRUCVERSION);
			AppendDword(arrFixedFileInfo, 0x00090008); // file version 9.8.7.6
			AppendDword(arrFixedFileInfo, 0x00070006);
			AppendDword(arrFixedFileInfo, 0x000D000C); // product version 13.12.11.10
			AppendDword(arrFixedFileInfo, 0x000B000A);
			AppendDword(arrFixedFileInfo, 0); // file flags mask
			AppendDword(arrFixedFileInfo, 0); // file flags
			AppendDword(arrFixedFileInfo, VOS_NT_WINDOWS32);
			AppendDword(arrFixedFileInfo, VFT_DLL);
			AppendDword(arrFixedFileInfo, 0); // file subtype
			AppendDword(arrFixedFileInfo, 0); // file date, high part
			AppendDword(arrFixedFileInfo, 0); // file date, low part
			Assert::AreEqual<size_t>(sizeof(VS_FIXEDFILEINFO), arrFixedFileInfo.size());

			const Bytes arrStringTable = MakeVersionBlock(L"000004E4", 1, 0, {},
				{ MakeStringBlock(L"ProductName", L"No Translation Fixture"), MakeStringBlock(L"CompanyName", L"BTreeAccess Test Company") });
			const Bytes arrStringFileInfo = MakeVersionBlock(L"StringFileInfo", 1, 0, {}, { arrStringTable });
			return MakeVersionBlock(L"VS_VERSION_INFO", 0, static_cast<WORD>(sizeof(VS_FIXEDFILEINFO)), arrFixedFileInfo, { arrStringFileInfo });
		}

		/// Stores a copy of the test DLL whose version resource is replaced by the given one.
		void StoreDllWithVersionResource(const CString& strPath, Bytes arrResource)
		{
			Assert::IsTrue(::CopyFile(GetModulePath(), strPath, FALSE) != FALSE, L"Unable to copy the test DLL");
			::SetFileAttributes(strPath, FILE_ATTRIBUTE_NORMAL); // the copy must be writable even if the original is not

			const HANDLE hUpdate = ::BeginUpdateResource(strPath, FALSE);
			Assert::IsTrue(hUpdate != nullptr, L"Unable to open the copy of the test DLL for resource updates");
			const BOOL bUpdated = ::UpdateResource(hUpdate, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO),
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), arrResource.data(), static_cast<DWORD>(arrResource.size()));
			const BOOL bClosed = ::EndUpdateResource(hUpdate, !bUpdated);
			Assert::IsTrue((bUpdated != FALSE) && (bClosed != FALSE), L"Unable to replace the version resource of the copy");
		}
	}

	TEST_CLASS(CVersionInfoTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		//----------------------------------------------------------------------
		// Before anything is loaded
		//----------------------------------------------------------------------

		TEST_METHOD(Constructor_NewObject_HasNoVersionData)
		{
			CVersionInfo info;

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
			Assert::AreEqual(0, info.GetNumberOfTranslations());
		}

		//----------------------------------------------------------------------
		// Load
		//----------------------------------------------------------------------

		TEST_METHOD(Load_ModuleWithVersionResource_ReturnsTrue)
		{
			CVersionInfo info;

			Assert::IsTrue(Loads(info, GetModulePath()));

			Assert::IsTrue(info.GetFixedFileInfo() != nullptr);
		}

		TEST_METHOD(Load_MissingFile_ReturnsFalseAndLoadsNothing)
		{
			TestHelpers::CTempFolder folder;
			CVersionInfo info;

			Assert::IsFalse(Loads(info, folder.GetEntry(_T("missing.dll"))));

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
			Assert::AreEqual(0, info.GetNumberOfTranslations());
		}

		TEST_METHOD(Load_EmptyFileName_ReturnsFalse)
		{
			CVersionInfo info;

			Assert::IsFalse(Loads(info, CString()));

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
		}

		TEST_METHOD(Load_TextFile_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			TestHelpers::WriteFile(folder.GetEntry(_T("notes.txt")), "This file has no version resource.\r\n");
			CVersionInfo info;

			Assert::IsFalse(Loads(info, folder.GetEntry(_T("notes.txt"))));

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
		}

		TEST_METHOD(Load_EmptyFile_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			TestHelpers::WriteFile(folder.GetEntry(_T("empty.dll")), "");
			CVersionInfo info;

			Assert::IsFalse(Loads(info, folder.GetEntry(_T("empty.dll"))));

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
		}

		TEST_METHOD(Load_Folder_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CVersionInfo info;

			Assert::IsFalse(Loads(info, folder.GetPath()));
		}

		TEST_METHOD(Load_FailingLoadAfterASuccessfulOne_DiscardsThePreviousData)
		{
			TestHelpers::CTempFolder folder;
			CVersionInfo info;
			LoadFixture(info);

			Assert::IsFalse(Loads(info, folder.GetEntry(_T("missing.dll"))));

			Assert::IsTrue(info.GetFixedFileInfo() == nullptr);
			Assert::AreEqual(0, info.GetNumberOfTranslations());
		}

		TEST_METHOD(Load_SameFileTwice_GivesTheSameValues)
		{
			CVersionInfo info;
			LoadFixture(info);

			LoadFixture(info);

			Assert::AreEqual<unsigned __int64>(MakeVersion(1, 2, 3, 4), info.GetFileVersion());
			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
			Assert::AreEqual(2, info.GetNumberOfTranslations());
		}

		TEST_METHOD(Load_AnotherFile_ReplacesThePreviousValues)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::IsTrue(Loads(info, GetSystemLibraryPath(_T("kernel32.dll"))));

			Assert::AreNotEqual<unsigned __int64>(MakeVersion(1, 2, 3, 4), info.GetFileVersion());
			Assert::AreNotEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));

			LoadFixture(info);

			Assert::AreEqual<unsigned __int64>(MakeVersion(1, 2, 3, 4), info.GetFileVersion());
			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
		}

		TEST_METHOD(Load_AfterSetTranslation_SelectsTheFirstLanguageAgain)
		{
			CVersionInfo info;
			LoadFixture(info);
			info.SetTranslation(1);
			Assert::AreEqual(CString(_T("BTreeAccess Testfirma")), Str(info.GetCompanyName()));

			LoadFixture(info);

			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
		}

		TEST_METHOD(Load_ResourceWithoutTranslationTable_ReadsTheNeutralLanguageAndTheAnsiCodePage)
		{
			TestHelpers::CTempFolder folder;
			const CString strPath = folder.GetEntry(_T("NoTranslation.dll"));
			StoreDllWithVersionResource(strPath, MakeVersionResourceWithoutTranslationTable());
			CVersionInfo info;

			Assert::IsTrue(Loads(info, strPath));

			Assert::IsTrue(info.GetFixedFileInfo() != nullptr);
			Assert::AreEqual(0, info.GetNumberOfTranslations());
			Assert::AreEqual<unsigned __int64>(MakeVersion(9, 8, 7, 6), info.GetFileVersion());
			Assert::AreEqual(CString(_T("No Translation Fixture")), Str(info.GetProductName()));
			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
		}

		TEST_METHOD(Load_ResourceWithoutTranslationTable_AfterAnotherLanguage_ForgetsThatLanguage)
		{
			TestHelpers::CTempFolder folder;
			const CString strPath = folder.GetEntry(_T("NoTranslation.dll"));
			StoreDllWithVersionResource(strPath, MakeVersionResourceWithoutTranslationTable());
			CVersionInfo info;
			LoadFixture(info);
			info.SetTranslation(1);

			Assert::IsTrue(Loads(info, strPath));

			Assert::AreEqual(0, info.GetNumberOfTranslations());
			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
		}

		TEST_METHOD(Load_SystemLibrary_ReadsItsVersionInformation)
		{
			CVersionInfo info;

			Assert::IsTrue(Loads(info, GetSystemLibraryPath(_T("kernel32.dll"))));

			const VS_FIXEDFILEINFO* pInfo = info.GetFixedFileInfo();
			Assert::IsTrue(pInfo != nullptr);
			Assert::AreEqual<DWORD>(VS_FFI_SIGNATURE, pInfo->dwSignature);
			Assert::IsTrue(info.GetFileVersion() != 0);
			Assert::IsTrue(info.GetProductVersion() != 0);
			Assert::IsTrue(info.GetNumberOfTranslations() >= 1);
			Assert::IsFalse(info.GetCompanyName().empty());
			Assert::IsFalse(info.GetProductName().empty());
			Assert::IsFalse(info.GetProductVersionAsString().empty());
		}

		//----------------------------------------------------------------------
		// Fixed file information
		//----------------------------------------------------------------------

		TEST_METHOD(GetFixedFileInfo_Fixture_HasTheVersionInfoSignature)
		{
			CVersionInfo info;
			LoadFixture(info);

			const VS_FIXEDFILEINFO* pInfo = info.GetFixedFileInfo();

			Assert::IsTrue(pInfo != nullptr);
			Assert::AreEqual<DWORD>(VS_FFI_SIGNATURE, pInfo->dwSignature);
			Assert::AreEqual<DWORD>(VS_FFI_STRUCVERSION, pInfo->dwStrucVersion);
		}

		TEST_METHOD(GetFileVersion_Fixture_ReturnsTheFourPartsInOneNumber)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<unsigned __int64>(MakeVersion(1, 2, 3, 4), info.GetFileVersion());
		}

		TEST_METHOD(GetProductVersion_Fixture_ReturnsTheFourPartsInOneNumber)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<unsigned __int64>(MakeVersion(5, 6, 7, 8), info.GetProductVersion());
		}

		TEST_METHOD(GetFileVersion_Fixture_DiffersFromTheProductVersion)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreNotEqual(info.GetFileVersion(), info.GetProductVersion());
		}

		TEST_METHOD(GetFileFlagsMask_Fixture_ReturnsTheFlagsThatAreValid)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<DWORD>(VS_FFI_FILEFLAGSMASK, info.GetFileFlagsMask());
		}

		TEST_METHOD(GetFileFlags_Fixture_ReturnsThePreReleaseAndPrivateBuildFlags)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<DWORD>(VS_FF_PRERELEASE | VS_FF_PRIVATEBUILD, info.GetFileFlags());
		}

		TEST_METHOD(GetOS_Fixture_ReturnsNt32)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<DWORD>(VOS_NT_WINDOWS32, info.GetOS());
		}

		TEST_METHOD(GetFileType_Fixture_ReturnsDll)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<DWORD>(VFT_DLL, info.GetFileType());
		}

		TEST_METHOD(GetFileSubType_Fixture_ReturnsUnknown)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual<DWORD>(VFT2_UNKNOWN, info.GetFileSubType());
		}

		TEST_METHOD(GetCreationTime_Fixture_ReturnsAnEmptyTime)
		{
			CVersionInfo info;
			LoadFixture(info);

			const FILETIME creationTime = info.GetCreationTime();

			Assert::AreEqual<DWORD>(0, creationTime.dwHighDateTime);
			Assert::AreEqual<DWORD>(0, creationTime.dwLowDateTime);
		}

		//----------------------------------------------------------------------
		// String values of the first language
		//----------------------------------------------------------------------

		TEST_METHOD(GetComments_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("Fixture comments")), Str(info.GetComments()));
		}

		TEST_METHOD(GetCompanyName_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
		}

		TEST_METHOD(GetFileDescription_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("BTreeAccess unit test fixture")), Str(info.GetFileDescription()));
		}

		TEST_METHOD(GetFileVersionAsString_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("1.2.3.4")), Str(info.GetFileVersionAsString()));
		}

		TEST_METHOD(GetInternalName_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("UnitTests.dll")), Str(info.GetInternalName()));
		}

		TEST_METHOD(GetLegalCopyright_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("Copyright (C) 2014-2026 Stefan-Mihai MOGA")), Str(info.GetLegalCopyright()));
		}

		TEST_METHOD(GetLegalTrademarks_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("Fixture trademark")), Str(info.GetLegalTrademarks()));
		}

		TEST_METHOD(GetOriginalFilename_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("UnitTests.dll")), Str(info.GetOriginalFilename()));
		}

		TEST_METHOD(GetPrivateBuild_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("Fixture private build")), Str(info.GetPrivateBuild()));
		}

		TEST_METHOD(GetProductName_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("BTreeAccess Unit Tests")), Str(info.GetProductName()));
		}

		TEST_METHOD(GetProductVersionAsString_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("5.6.7.8")), Str(info.GetProductVersionAsString()));
		}

		TEST_METHOD(GetSpecialBuild_Fixture_ReturnsTheValueOfTheFirstLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("Fixture special build")), Str(info.GetSpecialBuild()));
		}

		TEST_METHOD(GetValue_ExistingKey_ReturnsTheValue)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetValue(_T("CompanyName"))));
			Assert::AreEqual(CString(_T("Fixture comments")), Str(info.GetValue(_T("Comments"))));
		}

		TEST_METHOD(GetValue_UnknownKey_ReturnsAnEmptyString)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::IsTrue(info.GetValue(_T("NoSuchKey")).empty());
		}

		//----------------------------------------------------------------------
		// Translations
		//----------------------------------------------------------------------

		TEST_METHOD(GetNumberOfTranslations_Fixture_ReturnsTheNumberOfLanguagesOfTheResource)
		{
			CVersionInfo info;
			LoadFixture(info);

			Assert::AreEqual(2, info.GetNumberOfTranslations());
		}

		TEST_METHOD(GetTranslation_Fixture_ReturnsTheLanguagesInTheOrderOfTheResource)
		{
			CVersionInfo info;
			LoadFixture(info);

			const CVersionInfo::TRANSLATION* pEnglish = info.GetTranslation(0);
			const CVersionInfo::TRANSLATION* pGerman = info.GetTranslation(1);

			Assert::IsTrue(pEnglish != nullptr);
			Assert::AreEqual<WORD>(0x0409, pEnglish->m_wLangID);
			Assert::AreEqual<WORD>(1200, pEnglish->m_wCodePage);
			Assert::IsTrue(pGerman != nullptr);
			Assert::AreEqual<WORD>(0x0407, pGerman->m_wLangID);
			Assert::AreEqual<WORD>(1200, pGerman->m_wCodePage);
		}

		TEST_METHOD(SetTranslation_German_SwitchesToTheStringsOfThatLanguage)
		{
			CVersionInfo info;
			LoadFixture(info);

			info.SetTranslation(1);

			Assert::AreEqual(CString(_T("BTreeAccess Testfirma")), Str(info.GetCompanyName()));
			Assert::AreEqual(CString(_T("BTreeAccess Testbeschreibung")), Str(info.GetFileDescription()));
		}

		TEST_METHOD(SetTranslation_German_ValuesThatLanguageLacksAreEmpty)
		{
			CVersionInfo info;
			LoadFixture(info);

			info.SetTranslation(1);

			Assert::IsTrue(info.GetComments().empty());
			Assert::IsTrue(info.GetProductName().empty());
			Assert::IsTrue(info.GetProductVersionAsString().empty());
			Assert::IsTrue(info.GetSpecialBuild().empty());
		}

		TEST_METHOD(SetTranslation_BackToTheFirstLanguage_RestoresTheEnglishStrings)
		{
			CVersionInfo info;
			LoadFixture(info);
			info.SetTranslation(1);

			info.SetTranslation(0);

			Assert::AreEqual(CString(_T("BTreeAccess Test Company")), Str(info.GetCompanyName()));
			Assert::AreEqual(CString(_T("BTreeAccess Unit Tests")), Str(info.GetProductName()));
		}

		TEST_METHOD(SetTranslation_German_KeepsTheFixedFileInformationAndTheTranslationList)
		{
			CVersionInfo info;
			LoadFixture(info);

			info.SetTranslation(1);

			Assert::AreEqual<unsigned __int64>(MakeVersion(1, 2, 3, 4), info.GetFileVersion());
			Assert::AreEqual<unsigned __int64>(MakeVersion(5, 6, 7, 8), info.GetProductVersion());
			Assert::AreEqual(2, info.GetNumberOfTranslations());
		}
	};
}
