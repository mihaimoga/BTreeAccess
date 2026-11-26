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

// CNetworkBaseTests.cpp : unit tests for CNetworkBase
//

#include "stdafx.h"
#include "TestHelpers.h"

#include <initializer_list>
#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	namespace
	{
		struct EncodingCase
		{
			int nKey;
			LPCTSTR lpszCode;
		};

		// Base-36 codes, left-padded with '0' to MAX_CODE_LENGTH characters but never truncated.
		const EncodingCase g_arrEncodingCases[] =
		{
			{ 0, _T("0000") },
			{ 1, _T("0001") },
			{ 9, _T("0009") },
			{ 10, _T("000A") },
			{ 35, _T("000Z") },
			{ 36, _T("0010") },
			{ 1295, _T("00ZZ") },
			{ 1296, _T("0100") },
			{ 46655, _T("0ZZZ") },
			{ 46656, _T("1000") },
			{ 65535, _T("1EKF") },
			{ 65537, _T("1EKH") },
			{ 1679615, _T("ZZZZ") },
			{ 1679616, _T("10000") },
			{ INT_MAX, _T("ZIK0ZJ") },
		};

		struct FilePathCase
		{
			int nKey;
			LPCTSTR lpszPath;
			int nFolders;
		};

		// Keys are split in base BALANCED_TREE_ORDER (4): every digit but the last becomes a folder.
		const FilePathCase g_arrFilePathCases[] =
		{
			{ 0, _T("0000.dat"), 0 },
			{ 1, _T("0001.dat"), 0 },
			{ 3, _T("0003.dat"), 0 },
			{ 4, _T("0001\\0000.dat"), 1 },
			{ 5, _T("0001\\0001.dat"), 1 },
			{ 8, _T("0002\\0000.dat"), 1 },
			{ 15, _T("0003\\0003.dat"), 1 },
			{ 16, _T("0001\\0000\\0000.dat"), 2 },
			{ 20, _T("0001\\0001\\0000.dat"), 2 },
			{ 63, _T("0003\\0003\\0003.dat"), 2 },
			{ 64, _T("0001\\0000\\0000\\0000.dat"), 3 },
			{ 65535, _T("0003\\0003\\0003\\0003\\0003\\0003\\0003\\0003.dat"), 7 },
		};

		/// A folder list that is filled in place, because CStringArray cannot be copied or returned by value.
		class CFolderList : public CStringArray
		{
		public:
			CFolderList(std::initializer_list<LPCTSTR> lstNames)
			{
				for (LPCTSTR lpszName : lstNames)
					Add(lpszName);
			}
		};

		std::vector<CString> SplitPath(const CString& strPath)
		{
			std::vector<CString> arrParts;
			int nPosition = 0;
			CString strPart = strPath.Tokenize(_T("\\"), nPosition);
			while (nPosition != -1)
			{
				arrParts.push_back(strPart);
				strPart = strPath.Tokenize(_T("\\"), nPosition);
			}
			return arrParts;
		}

		/// Seeds the C runtime's generator so that the next rand() call returns the wanted value.
		void SeedForNextRand(int nWanted)
		{
			for (unsigned int nSeed = 0; nSeed < 4000000; nSeed++)
			{
				srand(nSeed);
				if (rand() == nWanted)
				{
					srand(nSeed);
					return;
				}
			}
			Assert::Fail(L"No seed makes rand() return the wanted value");
		}
	}

	TEST_CLASS(CNetworkBaseTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		//----------------------------------------------------------------------
		// Accessors and run-time type information
		//----------------------------------------------------------------------

		TEST_METHOD(Constructor_NewObject_HasEmptyFileListAndRootFolder)
		{
			CNetworkBase network;

			Assert::IsTrue(network.GetFileList().IsEmpty());
			Assert::IsTrue(network.GetRootFolder().IsEmpty());
		}

		TEST_METHOD(SetFileList_Path_IsReturnedAndRootFolderIsUntouched)
		{
			CNetworkBase network;

			network.SetFileList(CString(_T("C:\\Lists\\files.txt")));

			Assert::AreEqual(CString(_T("C:\\Lists\\files.txt")), network.GetFileList());
			Assert::IsTrue(network.GetRootFolder().IsEmpty());
		}

		TEST_METHOD(SetRootFolder_Path_IsReturnedAndFileListIsUntouched)
		{
			CNetworkBase network;

			network.SetRootFolder(CString(_T("C:\\Root\\")));

			Assert::AreEqual(CString(_T("C:\\Root\\")), network.GetRootFolder());
			Assert::IsTrue(network.GetFileList().IsEmpty());
		}

		TEST_METHOD(SetRootFolder_CalledTwice_KeepsLastValue)
		{
			CNetworkBase network;

			network.SetRootFolder(CString(_T("C:\\First\\")));
			network.SetRootFolder(CString(_T("D:\\Second\\")));

			Assert::AreEqual(CString(_T("D:\\Second\\")), network.GetRootFolder());
		}

		TEST_METHOD(IsKindOf_NetworkBase_MatchesOnlyItsOwnClassAndBaseClass)
		{
			CNetworkBase network;

			Assert::IsTrue(network.IsKindOf(RUNTIME_CLASS(CNetworkBase)));
			Assert::IsTrue(network.IsKindOf(RUNTIME_CLASS(CObject)));
			Assert::IsFalse(network.IsKindOf(RUNTIME_CLASS(CNetworkHash)));
			Assert::IsFalse(network.IsKindOf(RUNTIME_CLASS(CNetworkTree)));
		}

		//----------------------------------------------------------------------
		// IsValidCode
		//----------------------------------------------------------------------

		TEST_METHOD(IsValidCode_EmptyString_ReturnsTrue)
		{
			CNetworkBase network;

			Assert::IsTrue(network.IsValidCode(CString()));
		}

		TEST_METHOD(IsValidCode_EveryAllowedDigit_ReturnsTrue)
		{
			CNetworkBase network;

			Assert::IsTrue(network.IsValidCode(CString(_T("0123456789"))));
			Assert::IsTrue(network.IsValidCode(CString(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))));
			Assert::IsTrue(network.IsValidCode(strNetworkDigitString));
		}

		TEST_METHOD(IsValidCode_LongValidCode_ReturnsTrue)
		{
			CNetworkBase network;
			CString strLongCode;
			for (int nIndex = 0; nIndex < 100; nIndex++)
				strLongCode += strNetworkDigitString;

			Assert::IsTrue(network.IsValidCode(strLongCode));
		}

		TEST_METHOD(IsValidCode_LowercaseLetters_ReturnsFalse)
		{
			CNetworkBase network;

			Assert::IsFalse(network.IsValidCode(CString(_T("abcd"))));
			Assert::IsFalse(network.IsValidCode(CString(_T("ABCd"))));
			Assert::IsFalse(network.IsValidCode(CString(_T("aBCD"))));
		}

		TEST_METHOD(IsValidCode_InvalidCharacterAtAnyPosition_ReturnsFalse)
		{
			CNetworkBase network;
			const CString strValid(_T("0A1B"));

			for (int nPosition = 0; nPosition < strValid.GetLength(); nPosition++)
			{
				CString strCode = strValid;
				strCode.SetAt(nPosition, _T('-'));

				Assert::IsFalse(network.IsValidCode(strCode), strCode);
			}
		}

		TEST_METHOD(IsValidCode_SeparatorsAndWhitespace_ReturnsFalse)
		{
			CNetworkBase network;
			const CString strInvalid(_T(" -_.,;:\\/*?\"<>|+=!@#$%^&()[]{}~\t\r\n"));

			for (int nIndex = 0; nIndex < strInvalid.GetLength(); nIndex++)
			{
				CString strCode(_T("AB"));
				strCode += strInvalid.GetAt(nIndex);
				strCode += _T("CD");

				CString strMessage;
				strMessage.Format(_T("Character 0x%02X was accepted"), static_cast<int>(strInvalid.GetAt(nIndex)));
				Assert::IsFalse(network.IsValidCode(strCode), strMessage);
			}
		}

		TEST_METHOD(IsValidCode_LeadingOrTrailingSpace_ReturnsFalse)
		{
			CNetworkBase network;

			Assert::IsFalse(network.IsValidCode(CString(_T("ABCD "))));
			Assert::IsFalse(network.IsValidCode(CString(_T(" ABCD"))));
		}

		TEST_METHOD(IsValidCode_NonAsciiCharacter_ReturnsFalse)
		{
			CNetworkBase network;
			CString strCode(_T("AB"));
			strCode += static_cast<TCHAR>(0x00C9); // E with acute accent
			strCode += _T("D");

			Assert::IsFalse(network.IsValidCode(strCode));
		}

		TEST_METHOD(IsValidCode_EncodedKey_ReturnsTrue)
		{
			CNetworkBase network;

			for (int nKey = 0; nKey < 5000; nKey++)
			{
				const CString strCode = network.EncodeNetworkID(nKey);
				Assert::IsTrue(network.IsValidCode(strCode), strCode);
			}
		}

		//----------------------------------------------------------------------
		// EncodeNetworkID / DecodeNetworkID
		//----------------------------------------------------------------------

		TEST_METHOD(EncodeNetworkID_KnownKeys_ReturnExpectedCodes)
		{
			CNetworkBase network;

			for (const EncodingCase& testCase : g_arrEncodingCases)
			{
				CString strMessage;
				strMessage.Format(_T("Encoding of key %d"), testCase.nKey);
				Assert::AreEqual(CString(testCase.lpszCode), network.EncodeNetworkID(testCase.nKey), strMessage);
			}
		}

		TEST_METHOD(EncodeNetworkID_KeyWithinFourDigits_ReturnsExactlyFourCharacters)
		{
			CNetworkBase network;

			Assert::AreEqual(MAX_CODE_LENGTH, network.EncodeNetworkID(0).GetLength());
			Assert::AreEqual(MAX_CODE_LENGTH, network.EncodeNetworkID(35).GetLength());
			Assert::AreEqual(MAX_CODE_LENGTH, network.EncodeNetworkID(46656).GetLength());
			Assert::AreEqual(MAX_CODE_LENGTH, network.EncodeNetworkID(1679615).GetLength());
		}

		TEST_METHOD(EncodeNetworkID_KeyBeyondFourDigits_IsNotTruncated)
		{
			CNetworkBase network;

			Assert::AreEqual(MAX_CODE_LENGTH + 1, network.EncodeNetworkID(1679616).GetLength());
			Assert::AreEqual(6, network.EncodeNetworkID(INT_MAX).GetLength());
		}

		TEST_METHOD(EncodeNetworkID_ManyKeys_ReturnDistinctValidCodes)
		{
			CNetworkBase network;
			std::set<std::wstring> setCodes;

			for (int nKey = 0; nKey <= 100000; nKey++)
			{
				const CString strCode = network.EncodeNetworkID(nKey);
				if (!network.IsValidCode(strCode) || (strCode.GetLength() < MAX_CODE_LENGTH))
				{
					CString strMessage;
					strMessage.Format(_T("Key %d produced the malformed code '%s'"), nKey, static_cast<LPCWSTR>(strCode));
					Assert::Fail(strMessage);
				}
				setCodes.insert(static_cast<LPCWSTR>(strCode));
			}

			Assert::AreEqual(static_cast<size_t>(100001), setCodes.size());
		}

		TEST_METHOD(DecodeNetworkID_EmptyString_ReturnsZero)
		{
			CNetworkBase network;

			Assert::AreEqual(0, network.DecodeNetworkID(CString()));
		}

		TEST_METHOD(DecodeNetworkID_KnownCodes_ReturnExpectedKeys)
		{
			CNetworkBase network;

			for (const EncodingCase& testCase : g_arrEncodingCases)
			{
				CString strMessage;
				strMessage.Format(_T("Decoding of '%s'"), testCase.lpszCode);
				Assert::AreEqual(testCase.nKey, network.DecodeNetworkID(CString(testCase.lpszCode)), strMessage);
			}
		}

		TEST_METHOD(DecodeNetworkID_SingleDigit_ReturnsItsPosition)
		{
			CNetworkBase network;

			for (int nDigit = 0; nDigit < intNetworkDigitLength; nDigit++)
			{
				const CString strCode(strNetworkDigitString.GetAt(nDigit));
				Assert::AreEqual(nDigit, network.DecodeNetworkID(strCode), strCode);
			}
		}

		TEST_METHOD(DecodeNetworkID_LeadingZeros_AreIgnored)
		{
			CNetworkBase network;

			Assert::AreEqual(0, network.DecodeNetworkID(CString(_T("0000"))));
			Assert::AreEqual(1, network.DecodeNetworkID(CString(_T("1"))));
			Assert::AreEqual(1, network.DecodeNetworkID(CString(_T("0001"))));
			Assert::AreEqual(1, network.DecodeNetworkID(CString(_T("00000001"))));
			Assert::AreEqual(35, network.DecodeNetworkID(CString(_T("00Z"))));
		}

		TEST_METHOD(DecodeNetworkID_OfEncodedKey_RestoresTheKey)
		{
			CNetworkBase network;

			for (int nKey = 0; nKey <= 200000; nKey++)
			{
				const int nDecoded = network.DecodeNetworkID(network.EncodeNetworkID(nKey));
				if (nDecoded != nKey)
				{
					CString strMessage;
					strMessage.Format(_T("Key %d came back as %d"), nKey, nDecoded);
					Assert::Fail(strMessage);
				}
			}
		}

		TEST_METHOD(DecodeNetworkID_OfEncodedLargeKey_RestoresTheKey)
		{
			CNetworkBase network;
			const int arrKeys[] = { 1679615, 1679616, 1679617, 100000000, INT_MAX - 1, INT_MAX };

			for (const int nKey : arrKeys)
			{
				const int nDecoded = network.DecodeNetworkID(network.EncodeNetworkID(nKey));
				Assert::AreEqual(nKey, nDecoded);
			}
		}

		TEST_METHOD(EncodeNetworkID_OfDecodedFourCharacterCode_RestoresTheCode)
		{
			CNetworkBase network;
			const int nCodeCount = intNetworkDigitLength * intNetworkDigitLength * intNetworkDigitLength * intNetworkDigitLength;

			// Walking the whole 1.6 million code space is too slow for a unit test, so use a prime stride.
			for (int nKey = 0; nKey < nCodeCount; nKey += 101)
			{
				const CString strCode = network.EncodeNetworkID(nKey);
				Assert::AreEqual(strCode, network.EncodeNetworkID(network.DecodeNetworkID(strCode)), strCode);
			}
		}

		//----------------------------------------------------------------------
		// GetNextID
		//----------------------------------------------------------------------

		TEST_METHOD(GetNextID_ManyCalls_StaysWithinTheRandomIdRange)
		{
			CNetworkBase network;
			srand(12345);

			for (int nCall = 0; nCall < 20000; nCall++)
			{
				const int nId = network.GetNextID();
				if ((nId < 1) || (nId > MAX_RANDOM_DATA))
				{
					CString strMessage;
					strMessage.Format(_T("Call %d returned %d"), nCall, nId);
					Assert::Fail(strMessage);
				}
			}
		}

		TEST_METHOD(GetNextID_SameSeed_ReturnsSameSequence)
		{
			CNetworkBase network;
			std::vector<int> arrFirst;
			std::vector<int> arrSecond;

			srand(2024);
			for (int nCall = 0; nCall < 50; nCall++)
				arrFirst.push_back(network.GetNextID());
			srand(2024);
			for (int nCall = 0; nCall < 50; nCall++)
				arrSecond.push_back(network.GetNextID());

			Assert::IsTrue(arrFirst == arrSecond);
		}

		TEST_METHOD(GetNextID_DifferentSeeds_ReturnDifferentSequences)
		{
			CNetworkBase network;
			std::vector<int> arrFirst;
			std::vector<int> arrSecond;

			srand(1);
			for (int nCall = 0; nCall < 50; nCall++)
				arrFirst.push_back(network.GetNextID());
			srand(2);
			for (int nCall = 0; nCall < 50; nCall++)
				arrSecond.push_back(network.GetNextID());

			Assert::IsFalse(arrFirst == arrSecond);
		}

		TEST_METHOD(GetNextID_ManyCalls_ProducesManyDistinctValues)
		{
			CNetworkBase network;
			std::set<int> setIds;
			srand(7);

			for (int nCall = 0; nCall < 2000; nCall++)
				setIds.insert(network.GetNextID());

			// 2000 draws out of rand()'s 32768 outputs collide rarely; a stuck generator would not pass.
			Assert::IsTrue(setIds.size() > 1500);
		}

		TEST_METHOD(GetNextID_LowestRandValue_ReturnsOne)
		{
			CNetworkBase network;
			SeedForNextRand(0);

			Assert::AreEqual(1, network.GetNextID());
		}

		TEST_METHOD(GetNextID_HighestRandValue_ReturnsTheLargestId)
		{
			CNetworkBase network;
			SeedForNextRand(RAND_MAX);

			// rand() has only 15 bits, so the ids stop at 65535 although MAX_RANDOM_DATA is 65537.
			Assert::AreEqual(65535, network.GetNextID());
		}

		//----------------------------------------------------------------------
		// FormatFolder / FormatFilename
		//----------------------------------------------------------------------

		TEST_METHOD(FormatFolder_Key_ReturnsTheEncodedCode)
		{
			CNetworkBase network;

			Assert::AreEqual(CString(_T("0000")), network.FormatFolder(0));
			Assert::AreEqual(CString(_T("0003")), network.FormatFolder(3));
			Assert::AreEqual(CString(_T("0010")), network.FormatFolder(36));
			Assert::AreEqual(CString(_T("1000")), network.FormatFolder(46656));
		}

		TEST_METHOD(FormatFilename_Key_AppendsTheDatExtension)
		{
			CNetworkBase network;

			Assert::AreEqual(CString(_T("0000.dat")), network.FormatFilename(0));
			Assert::AreEqual(CString(_T("0003.dat")), network.FormatFilename(3));
			Assert::AreEqual(CString(_T("0010.dat")), network.FormatFilename(36));
			Assert::AreEqual(CString(_T("1000.dat")), network.FormatFilename(46656));
		}

		TEST_METHOD(FormatFilename_AnyKey_IsTheFolderNameWithExtension)
		{
			CNetworkBase network;

			for (int nKey = 0; nKey < 3000; nKey++)
				Assert::AreEqual(network.FormatFolder(nKey) + _T(".dat"), network.FormatFilename(nKey));
		}

		//----------------------------------------------------------------------
		// GetFilePath
		//----------------------------------------------------------------------

		TEST_METHOD(GetFilePath_KnownKeys_ReturnExpectedPathAndFolderCount)
		{
			CNetworkBase network;

			for (const FilePathCase& testCase : g_arrFilePathCases)
			{
				CStringArray arrFolders;
				CString strMessage;
				strMessage.Format(_T("Path of key %d"), testCase.nKey);

				const CString strPath = network.GetFilePath(testCase.nKey, arrFolders);

				Assert::AreEqual(CString(testCase.lpszPath), strPath, strMessage);
				Assert::AreEqual(testCase.nFolders, static_cast<int>(arrFolders.GetCount()), strMessage);
			}
		}

		TEST_METHOD(GetFilePath_KeyWithoutFolders_ReturnsOnlyAFilename)
		{
			CNetworkBase network;
			CStringArray arrFolders;

			const CString strPath = network.GetFilePath(2, arrFolders);

			Assert::AreEqual(CString(_T("0002.dat")), strPath);
			Assert::AreEqual(0, static_cast<int>(arrFolders.GetCount()));
		}

		TEST_METHOD(GetFilePath_KeyWithNestedFolders_ListsFoldersFromTheTopDown)
		{
			CNetworkBase network;
			CStringArray arrFolders;

			// 27 is 123 in base 4: folder 0001, folder 0002, file 0003.
			const CString strPath = network.GetFilePath(27, arrFolders);

			Assert::AreEqual(CString(_T("0001\\0002\\0003.dat")), strPath);
			Assert::AreEqual(2, static_cast<int>(arrFolders.GetCount()));
			Assert::AreEqual(CString(_T("0001")), arrFolders.GetAt(0));
			Assert::AreEqual(CString(_T("0002")), arrFolders.GetAt(1));
		}

		TEST_METHOD(GetFilePath_ArrayWithPreviousContent_IsReplaced)
		{
			CNetworkBase network;
			CFolderList arrFolders{ _T("stale"), _T("entries"), _T("that"), _T("must"), _T("go") };

			network.GetFilePath(5, arrFolders);

			Assert::AreEqual(1, static_cast<int>(arrFolders.GetCount()));
			Assert::AreEqual(CString(_T("0001")), arrFolders.GetAt(0));
		}

		TEST_METHOD(GetFilePath_ArrayWithPreviousContentAndFolderlessKey_IsEmptied)
		{
			CNetworkBase network;
			CFolderList arrFolders{ _T("stale") };

			const CString strPath = network.GetFilePath(1, arrFolders);

			Assert::AreEqual(CString(_T("0001.dat")), strPath);
			Assert::AreEqual(0, static_cast<int>(arrFolders.GetCount()));
		}

		TEST_METHOD(GetFilePath_KeyThatIsAPowerOfTheOrder_AddsOneFolderPerPower)
		{
			CNetworkBase network;
			int nKey = 1;

			for (int nPower = 0; nPower <= 8; nPower++)
			{
				CStringArray arrFolders;
				network.GetFilePath(nKey, arrFolders);

				CString strMessage;
				strMessage.Format(_T("Key %d"), nKey);
				Assert::AreEqual(nPower, static_cast<int>(arrFolders.GetCount()), strMessage);
				nKey *= BALANCED_TREE_ORDER;
			}
		}

		TEST_METHOD(GetFilePath_AnyKey_UsesBackslashSeparatorsOnly)
		{
			CNetworkBase network;

			for (int nKey = 0; nKey < 2000; nKey++)
			{
				CStringArray arrFolders;
				const CString strPath = network.GetFilePath(nKey, arrFolders);

				Assert::AreEqual(-1, strPath.Find(_T('/')), strPath);
				Assert::IsTrue(strPath.Right(4) == _T(".dat"), strPath);
			}
		}

		TEST_METHOD(GetFilePath_EveryRandomId_MapsBijectivelyOntoBaseFourDigits)
		{
			CNetworkBase network;
			std::set<std::wstring> setPaths;

			for (int nKey = 0; nKey <= MAX_RANDOM_DATA; nKey++)
			{
				CStringArray arrFolders;
				const CString strPath = network.GetFilePath(nKey, arrFolders);
				const std::vector<CString> arrParts = SplitPath(strPath);

				int nExpectedDigits = 1;
				for (int nRest = nKey / BALANCED_TREE_ORDER; nRest > 0; nRest /= BALANCED_TREE_ORDER)
					nExpectedDigits++;

				CString strMessage;
				strMessage.Format(_T("Key %d mapped to '%s'"), nKey, static_cast<LPCWSTR>(strPath));
				Assert::AreEqual(nExpectedDigits, static_cast<int>(arrParts.size()), strMessage);
				Assert::AreEqual(nExpectedDigits - 1, static_cast<int>(arrFolders.GetCount()), strMessage);

				// Reading the components back as base-4 digits must give the original key.
				int nRebuilt = 0;
				for (size_t nPart = 0; nPart < arrParts.size(); nPart++)
				{
					CString strDigit = arrParts[nPart];
					if (nPart + 1 == arrParts.size())
					{
						Assert::IsTrue(strDigit.Right(4) == _T(".dat"), strMessage);
						strDigit = strDigit.Left(strDigit.GetLength() - 4);
					}
					else
					{
						Assert::IsTrue(arrParts[nPart] == arrFolders.GetAt(static_cast<INT_PTR>(nPart)), strMessage);
					}
					const int nDigit = network.DecodeNetworkID(strDigit);
					Assert::IsTrue((nDigit >= 0) && (nDigit < BALANCED_TREE_ORDER), strMessage);
					nRebuilt = nRebuilt * BALANCED_TREE_ORDER + nDigit;
				}
				Assert::AreEqual(nKey, nRebuilt, strMessage);

				setPaths.insert(static_cast<LPCWSTR>(strPath));
			}

			// No two keys may share a file.
			Assert::AreEqual(static_cast<size_t>(MAX_RANDOM_DATA) + 1, setPaths.size());
		}

		//----------------------------------------------------------------------
		// CreateNetworkPath
		//----------------------------------------------------------------------

		TEST_METHOD(CreateNetworkPath_MissingRootFolder_ReturnsFalseAndCreatesNothing)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001") };
			const CString strMissingRoot = folder.GetEntry(_T("missing"));

			Assert::IsFalse(network.CreateNetworkPath(strMissingRoot, arrFolders));

			Assert::IsFalse(TestHelpers::PathExists(strMissingRoot));
		}

		TEST_METHOD(CreateNetworkPath_EmptyRootFolder_ReturnsFalse)
		{
			CNetworkBase network;
			CStringArray arrFolders;

			Assert::IsFalse(network.CreateNetworkPath(CString(), arrFolders));
		}

		TEST_METHOD(CreateNetworkPath_NoSubFolders_ReturnsTrueAndCreatesNothing)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CStringArray arrFolders;

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsTrue(std::filesystem::is_empty(std::filesystem::path(static_cast<LPCWSTR>(folder.GetPath()))));
		}

		TEST_METHOD(CreateNetworkPath_SingleSubFolder_CreatesIt)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001") };

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001"))));
		}

		TEST_METHOD(CreateNetworkPath_NestedSubFolders_CreatesTheWholeChain)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001"), _T("0002"), _T("0003") };

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001"))));
			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001\\0002"))));
			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001\\0002\\0003"))));
		}

		TEST_METHOD(CreateNetworkPath_RootWithoutTrailingBackslash_CreatesTheChain)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001"), _T("0000") };
			CString strRoot = folder.GetPath();
			strRoot.TrimRight(_T('\\'));

			Assert::IsTrue(network.CreateNetworkPath(strRoot, arrFolders));

			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001\\0000"))));
		}

		TEST_METHOD(CreateNetworkPath_CalledTwice_StillSucceedsAndKeepsContent)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001"), _T("0002") };
			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));
			TestHelpers::WriteFile(folder.GetEntry(_T("0001\\0002\\0000.dat")), "keep me");

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::AreEqual(std::string("keep me"), TestHelpers::ReadFile(folder.GetEntry(_T("0001\\0002\\0000.dat"))));
		}

		TEST_METHOD(CreateNetworkPath_PartiallyExistingChain_OnlyAddsTheMissingFolders)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrExisting{ _T("0001") };
			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrExisting));
			TestHelpers::WriteFile(folder.GetEntry(_T("0001\\0000.dat")), "existing file");
			CFolderList arrFolders{ _T("0001"), _T("0002"), _T("0003") };

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsTrue(TestHelpers::IsDirectory(folder.GetEntry(_T("0001\\0002\\0003"))));
			Assert::AreEqual(std::string("existing file"), TestHelpers::ReadFile(folder.GetEntry(_T("0001\\0000.dat"))));
		}

		TEST_METHOD(CreateNetworkPath_FileUsesTheFolderName_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			TestHelpers::WriteFile(folder.GetEntry(_T("0001")), "I am a file, not a folder");
			CFolderList arrFolders{ _T("0001"), _T("0002") };

			Assert::IsFalse(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsFalse(TestHelpers::IsDirectory(folder.GetEntry(_T("0001"))));
		}

		TEST_METHOD(CreateNetworkPath_FoldersOfGetFilePath_MakeTheFilePathWritable)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CStringArray arrFolders;
			const CString strFilePath = network.GetFilePath(64, arrFolders);

			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			TestHelpers::WriteFile(folder.GetEntry(strFilePath), "deep");
			Assert::IsTrue(TestHelpers::PathExists(folder.GetEntry(_T("0001\\0000\\0000\\0000.dat"))));
		}

		//----------------------------------------------------------------------
		// FileExists
		//----------------------------------------------------------------------

		TEST_METHOD(FileExists_ExistingFile_ReturnsTrue)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			TestHelpers::WriteFile(folder.GetEntry(_T("0001.dat")), "content");

			Assert::IsTrue(network.FileExists(folder.GetEntry(_T("0001.dat"))));
		}

		TEST_METHOD(FileExists_EmptyFile_ReturnsTrue)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			TestHelpers::WriteFile(folder.GetEntry(_T("empty.dat")), "");

			Assert::IsTrue(network.FileExists(folder.GetEntry(_T("empty.dat"))));
		}

		TEST_METHOD(FileExists_MissingFile_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;

			Assert::IsFalse(network.FileExists(folder.GetEntry(_T("missing.dat"))));
		}

		TEST_METHOD(FileExists_FileInMissingFolder_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;

			Assert::IsFalse(network.FileExists(folder.GetEntry(_T("no\\such\\folder\\0001.dat"))));
		}

		TEST_METHOD(FileExists_Directory_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			CFolderList arrFolders{ _T("0001") };
			Assert::IsTrue(network.CreateNetworkPath(folder.GetPath(), arrFolders));

			Assert::IsFalse(network.FileExists(folder.GetEntry(_T("0001"))));
		}

		TEST_METHOD(FileExists_EmptyPath_ReturnsFalse)
		{
			CNetworkBase network;

			Assert::IsFalse(network.FileExists(CString()));
		}

		TEST_METHOD(FileExists_PathWithInvalidCharacters_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;

			Assert::IsFalse(network.FileExists(folder.GetEntry(_T("bad<name>?.dat"))));
		}

		TEST_METHOD(FileExists_FileDeletedAfterwards_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			const CString strFile = folder.GetEntry(_T("0001.dat"));
			TestHelpers::WriteFile(strFile, "content");
			Assert::IsTrue(network.FileExists(strFile));

			Assert::IsTrue(DeleteFile(strFile) != FALSE);

			Assert::IsFalse(network.FileExists(strFile));
		}

		TEST_METHOD(FileExists_ExistingFile_LeavesTheContentUntouched)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			const CString strFile = folder.GetEntry(_T("0001.dat"));
			TestHelpers::WriteFile(strFile, "content");

			Assert::IsTrue(network.FileExists(strFile));

			Assert::AreEqual(std::string("content"), TestHelpers::ReadFile(strFile));
		}

		// Documents the exclusive share mode of the probe: while any other handle is open the file looks absent.
		TEST_METHOD(FileExists_FileOpenInAnotherHandle_ReturnsFalse)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;
			const CString strFile = folder.GetEntry(_T("0001.dat"));
			TestHelpers::WriteFile(strFile, "content");

			{
				CFile fileOpen;
				Assert::IsTrue(fileOpen.Open(strFile, CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary) != FALSE);

				Assert::IsFalse(network.FileExists(strFile));
			}

			Assert::IsTrue(network.FileExists(strFile));
		}

		//----------------------------------------------------------------------
		// FormatLastError
		//----------------------------------------------------------------------

		TEST_METHOD(FormatLastError_CommonErrorCodes_ReturnNonEmptyMessages)
		{
			CNetworkBase network;
			const DWORD arrErrors[] = { ERROR_SUCCESS, ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_ACCESS_DENIED };

			for (const DWORD dwError : arrErrors)
			{
				CString strMessage;
				strMessage.Format(_T("Error %lu has no message"), dwError);
				Assert::IsFalse(network.FormatLastError(dwError).IsEmpty(), strMessage);
			}
		}

		TEST_METHOD(FormatLastError_KnownError_HasNoTrailingLineBreakOrSpace)
		{
			CNetworkBase network;

			const CString strMessage = network.FormatLastError(ERROR_FILE_NOT_FOUND);

			Assert::IsFalse(strMessage.IsEmpty());
			const TCHAR chLast = strMessage.GetAt(strMessage.GetLength() - 1);
			Assert::IsTrue((chLast != _T('\r')) && (chLast != _T('\n')) && (chLast != _T(' ')));
		}

		TEST_METHOD(FormatLastError_DifferentErrorCodes_ReturnDifferentMessages)
		{
			CNetworkBase network;

			const CString strNotFound = network.FormatLastError(ERROR_FILE_NOT_FOUND);
			const CString strNoPath = network.FormatLastError(ERROR_PATH_NOT_FOUND);
			const CString strDenied = network.FormatLastError(ERROR_ACCESS_DENIED);

			Assert::IsTrue(strNotFound != strNoPath);
			Assert::IsTrue(strNotFound != strDenied);
			Assert::IsTrue(strNoPath != strDenied);
		}

		TEST_METHOD(FormatLastError_SameErrorCodeTwice_ReturnsTheSameMessage)
		{
			CNetworkBase network;

			Assert::AreEqual(network.FormatLastError(ERROR_ACCESS_DENIED), network.FormatLastError(ERROR_ACCESS_DENIED));
		}

		TEST_METHOD(FormatLastError_UnknownErrorCode_ReturnsEmptyMessage)
		{
			CNetworkBase network;

			Assert::IsTrue(network.FormatLastError(0xFFFFFFFF).IsEmpty());
		}

		TEST_METHOD(FormatLastError_ErrorOfFailedWin32Call_MatchesTheDocumentedCode)
		{
			TestHelpers::CTempFolder folder;
			CNetworkBase network;

			Assert::IsTrue(GetFileAttributes(folder.GetEntry(_T("missing.dat"))) == INVALID_FILE_ATTRIBUTES);
			const DWORD dwLastError = GetLastError();

			Assert::AreEqual(static_cast<DWORD>(ERROR_FILE_NOT_FOUND), dwLastError);
			Assert::AreEqual(network.FormatLastError(ERROR_FILE_NOT_FOUND), network.FormatLastError(dwLastError));
		}
	};
}
