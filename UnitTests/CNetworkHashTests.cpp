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

// CNetworkHashTests.cpp : unit tests for CNetworkHash
//

#include "stdafx.h"
#include "TestHelpers.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	namespace
	{
		/// Gives the tests access to the protected maps, to build states the public interface cannot reach.
		class CTestNetworkHash : public CNetworkHash
		{
		public:
			void AddLevelOnly(int nKey, int nLevel) { m_mapFileCode.SetAt(nKey, nLevel); }
			void AddPathOnly(int nKey, const CString& strFilePath) { m_mapFilePath.SetAt(nKey, strFilePath); }
			int GetLevelCount() { return static_cast<int>(m_mapFileCode.GetCount()); }
			int GetPathCount() { return static_cast<int>(m_mapFilePath.GetCount()); }
		};

		/// A hash whose root folder and file list live in a private temporary folder.
		class CStoredHash
		{
		public:
			CStoredHash()
			{
				hash.SetRootFolder(folder.GetPath());
				hash.SetFileList(GetListFile());
			}

			/// Stores nCount files; the file with index i gets the key KeyOf(i) and the level i.
			void Fill(int nCount)
			{
				for (int nIndex = 0; nIndex < nCount; nIndex++)
					Assert::AreEqual(nIndex, TestHelpers::StoreFile(hash, KeyOf(nIndex)));
			}

			static int KeyOf(int nIndex) { return 100 + nIndex * 7; }
			CString GetListFile() const { return folder.GetEntry(_T("files.txt")); }
			CString GetAbsolute(LPCTSTR lpszRelativePath) const { return folder.GetEntry(lpszRelativePath); }

			TestHelpers::CTempFolder folder;
			CNetworkHash hash;
		};

		/// Fails the test unless the key is indexed with the given level and relative path.
		void ExpectItem(CNetworkHash& hash, int nKey, int nExpectedLevel, LPCTSTR lpszExpectedPath)
		{
			int nLevel = -1;
			CString strFilePath;
			CString strMessage;
			strMessage.Format(_T("Item with key %d"), nKey);

			Assert::IsTrue(hash.SearchItem(nKey, nLevel, strFilePath), strMessage);
			Assert::AreEqual(nExpectedLevel, nLevel, strMessage);
			Assert::AreEqual(CString(lpszExpectedPath), strFilePath, strMessage);
		}

		/// Fails the test when the key is indexed.
		void ExpectNoItem(CNetworkHash& hash, int nKey)
		{
			int nLevel = -1;
			CString strFilePath;
			CString strMessage;
			strMessage.Format(_T("Item with key %d"), nKey);

			Assert::IsFalse(hash.SearchItem(nKey, nLevel, strFilePath), strMessage);
		}
	}

	TEST_CLASS(CNetworkHashTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		//----------------------------------------------------------------------
		// Construction and run-time type information
		//----------------------------------------------------------------------

		TEST_METHOD(Constructor_NewHash_IsEmpty)
		{
			CNetworkHash hash;

			Assert::AreEqual(0, hash.GetSize());
			Assert::IsTrue(hash.GetFileList().IsEmpty());
			Assert::IsTrue(hash.GetRootFolder().IsEmpty());
		}

		TEST_METHOD(IsKindOf_Hash_MatchesItselfAndItsBaseClasses)
		{
			CNetworkHash hash;

			Assert::IsTrue(hash.IsKindOf(RUNTIME_CLASS(CNetworkHash)));
			Assert::IsTrue(hash.IsKindOf(RUNTIME_CLASS(CNetworkBase)));
			Assert::IsFalse(hash.IsKindOf(RUNTIME_CLASS(CNetworkTree)));
		}

		TEST_METHOD(Hash_InheritedHelpers_AreAvailable)
		{
			CNetworkHash hash;

			Assert::AreEqual(CString(_T("000A")), hash.EncodeNetworkID(10));
			Assert::AreEqual(10, hash.DecodeNetworkID(CString(_T("000A"))));
		}

		//----------------------------------------------------------------------
		// SearchItem / InsertItem / GetKey
		//----------------------------------------------------------------------

		TEST_METHOD(SearchItem_EmptyHash_ReturnsFalse)
		{
			CNetworkHash hash;

			ExpectNoItem(hash, 1);
			ExpectNoItem(hash, 0);
			ExpectNoItem(hash, -1);
		}

		TEST_METHOD(InsertItem_SingleItem_IsFoundWithItsLevelAndPath)
		{
			CNetworkHash hash;

			Assert::IsTrue(hash.InsertItem(42, 3, _T("0003.dat")));

			Assert::AreEqual(1, hash.GetSize());
			Assert::AreEqual(42, hash.GetKey(0));
			ExpectItem(hash, 42, 3, _T("0003.dat"));
		}

		TEST_METHOD(InsertItem_SeveralItems_EachKeepsItsOwnData)
		{
			CNetworkHash hash;

			hash.InsertItem(10, 0, _T("0000.dat"));
			hash.InsertItem(20, 1, _T("0001.dat"));
			hash.InsertItem(30, 4, _T("0001\\0000.dat"));

			Assert::AreEqual(3, hash.GetSize());
			ExpectItem(hash, 10, 0, _T("0000.dat"));
			ExpectItem(hash, 20, 1, _T("0001.dat"));
			ExpectItem(hash, 30, 4, _T("0001\\0000.dat"));
			ExpectNoItem(hash, 15);
			ExpectNoItem(hash, 31);
		}

		TEST_METHOD(GetKey_AfterInserts_ReturnsKeysInInsertionOrder)
		{
			CNetworkHash hash;

			hash.InsertItem(30, 0, _T("a.dat"));
			hash.InsertItem(10, 1, _T("b.dat"));
			hash.InsertItem(20, 2, _T("c.dat"));

			Assert::AreEqual(30, hash.GetKey(0));
			Assert::AreEqual(10, hash.GetKey(1));
			Assert::AreEqual(20, hash.GetKey(2));
		}

		TEST_METHOD(InsertItem_ManyItems_AreAllFound)
		{
			CNetworkHash hash;
			const int nCount = 5000;

			for (int nIndex = 0; nIndex < nCount; nIndex++)
			{
				CString strFilePath;
				strFilePath.Format(_T("%04d.dat"), nIndex);
				hash.InsertItem(1 + nIndex * 13, nIndex, strFilePath);
			}

			Assert::AreEqual(nCount, hash.GetSize());
			for (int nIndex = 0; nIndex < nCount; nIndex++)
			{
				CString strFilePath;
				strFilePath.Format(_T("%04d.dat"), nIndex);
				ExpectItem(hash, 1 + nIndex * 13, nIndex, strFilePath);
			}
			ExpectNoItem(hash, 2);
		}

		TEST_METHOD(InsertItem_PathWithSpaces_IsStoredVerbatim)
		{
			CNetworkHash hash;

			hash.InsertItem(7, 3, _T("sub folder\\my file.dat"));

			ExpectItem(hash, 7, 3, _T("sub folder\\my file.dat"));
		}

		TEST_METHOD(InsertItem_ExistingKey_ReplacesLevelAndPath)
		{
			CNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));

			hash.InsertItem(10, 5, _T("0001\\0001.dat"));

			ExpectItem(hash, 10, 5, _T("0001\\0001.dat"));
		}

		//----------------------------------------------------------------------
		// UpdateItem
		//----------------------------------------------------------------------

		TEST_METHOD(UpdateItem_ExistingKey_ReplacesLevelAndPath)
		{
			CNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));

			Assert::IsTrue(hash.UpdateItem(10, 4, _T("0001\\0000.dat")));

			ExpectItem(hash, 10, 4, _T("0001\\0000.dat"));
		}

		TEST_METHOD(UpdateItem_ExistingKey_KeepsTheOtherItemsAndTheKeyList)
		{
			CNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));
			hash.InsertItem(20, 1, _T("0001.dat"));
			hash.InsertItem(30, 2, _T("0002.dat"));

			hash.UpdateItem(20, 7, _T("0001\\0003.dat"));

			Assert::AreEqual(3, hash.GetSize());
			Assert::AreEqual(10, hash.GetKey(0));
			Assert::AreEqual(20, hash.GetKey(1));
			Assert::AreEqual(30, hash.GetKey(2));
			ExpectItem(hash, 10, 0, _T("0000.dat"));
			ExpectItem(hash, 20, 7, _T("0001\\0003.dat"));
			ExpectItem(hash, 30, 2, _T("0002.dat"));
		}

		TEST_METHOD(UpdateItem_UnknownKey_ReturnsTrueAndMakesTheKeySearchable)
		{
			CNetworkHash hash;

			Assert::IsTrue(hash.UpdateItem(99, 2, _T("0002.dat")));

			ExpectItem(hash, 99, 2, _T("0002.dat"));
		}

		//----------------------------------------------------------------------
		// RemoveAll
		//----------------------------------------------------------------------

		TEST_METHOD(RemoveAll_FilledHash_EmptiesTheListAndBothMaps)
		{
			CTestNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));
			hash.InsertItem(20, 1, _T("0001.dat"));

			hash.RemoveAll();

			Assert::AreEqual(0, hash.GetSize());
			Assert::AreEqual(0, hash.GetLevelCount());
			Assert::AreEqual(0, hash.GetPathCount());
			ExpectNoItem(hash, 10);
			ExpectNoItem(hash, 20);
		}

		TEST_METHOD(RemoveAll_EmptyHash_DoesNothing)
		{
			CNetworkHash hash;

			hash.RemoveAll();

			Assert::AreEqual(0, hash.GetSize());
		}

		TEST_METHOD(RemoveAll_ThenInsert_WorksAgain)
		{
			CNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));
			hash.RemoveAll();

			hash.InsertItem(11, 0, _T("0000.dat"));

			Assert::AreEqual(1, hash.GetSize());
			Assert::AreEqual(11, hash.GetKey(0));
			ExpectItem(hash, 11, 0, _T("0000.dat"));
			ExpectNoItem(hash, 10);
		}

		TEST_METHOD(RemoveAll_KeepsFileListAndRootFolder)
		{
			CNetworkHash hash;
			hash.SetFileList(CString(_T("C:\\Lists\\files.txt")));
			hash.SetRootFolder(CString(_T("C:\\Root\\")));
			hash.InsertItem(10, 0, _T("0000.dat"));

			hash.RemoveAll();

			Assert::AreEqual(CString(_T("C:\\Lists\\files.txt")), hash.GetFileList());
			Assert::AreEqual(CString(_T("C:\\Root\\")), hash.GetRootFolder());
		}

		//----------------------------------------------------------------------
		// GenerateID
		//----------------------------------------------------------------------

		TEST_METHOD(GenerateID_EmptyHash_ReturnsAnIdInTheRandomRange)
		{
			CNetworkHash hash;
			srand(31);

			for (int nCall = 0; nCall < 1000; nCall++)
			{
				const int nId = hash.GenerateID();
				Assert::IsTrue((nId >= 1) && (nId <= MAX_RANDOM_DATA));
			}
		}

		TEST_METHOD(GenerateID_FirstCandidateAlreadyTaken_ReturnsTheNextCandidate)
		{
			CNetworkHash hash;
			srand(99);
			const int nTaken = hash.GetNextID();
			int nExpected = nTaken;
			while (nExpected == nTaken)
				nExpected = hash.GetNextID();
			hash.InsertItem(nTaken, 0, _T("0000.dat"));

			srand(99);

			Assert::AreEqual(nExpected, hash.GenerateID());
		}

		TEST_METHOD(GenerateID_SeveralCandidatesAlreadyTaken_SkipsAllOfThem)
		{
			CNetworkHash hash;
			std::set<int> setTaken;
			srand(5);
			for (int nCall = 0; nCall < 10; nCall++)
			{
				const int nId = hash.GetNextID();
				if (setTaken.insert(nId).second)
					hash.InsertItem(nId, hash.GetSize(), _T("taken.dat"));
			}
			int nExpected = hash.GetNextID();
			while (setTaken.count(nExpected) != 0)
				nExpected = hash.GetNextID();

			srand(5);

			Assert::AreEqual(nExpected, hash.GenerateID());
		}

		TEST_METHOD(GenerateID_Call_DoesNotRegisterTheId)
		{
			CNetworkHash hash;

			const int nId = hash.GenerateID();

			Assert::AreEqual(0, hash.GetSize());
			ExpectNoItem(hash, nId);
		}

		TEST_METHOD(GenerateID_ManyIdsRegistered_NeverRepeatsOne)
		{
			CNetworkHash hash;
			std::set<int> setIds;
			srand(2024);

			for (int nCount = 0; nCount < 2000; nCount++)
			{
				const int nId = hash.GenerateID();
				Assert::IsTrue(setIds.insert(nId).second, L"GenerateID returned an ID that is already registered");
				Assert::IsTrue(hash.InsertItem(nId, hash.GetSize(), _T("generated.dat")));
			}

			Assert::AreEqual(2000, hash.GetSize());
		}

		//----------------------------------------------------------------------
		// DeleteItem
		//----------------------------------------------------------------------

		TEST_METHOD(DeleteItem_EmptyHash_ReturnsFalse)
		{
			CNetworkHash hash;

			Assert::IsFalse(hash.DeleteItem(1));

			Assert::AreEqual(0, hash.GetSize());
		}

		TEST_METHOD(DeleteItem_UnknownKey_ReturnsFalseAndChangesNothing)
		{
			CStoredHash stored;
			stored.Fill(5);

			Assert::IsFalse(TestHelpers::RemoveStoredFile(stored.hash, 999999));

			Assert::AreEqual(5, stored.hash.GetSize());
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_OnlyItem_EmptiesTheHash)
		{
			CStoredHash stored;
			stored.Fill(1);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(0)));

			Assert::AreEqual(0, stored.hash.GetSize());
			ExpectNoItem(stored.hash, CStoredHash::KeyOf(0));
			Assert::AreEqual(static_cast<size_t>(0), TestHelpers::CountDataFiles(stored.folder.GetPath()));
		}

		TEST_METHOD(DeleteItem_TwoItemsFirstDeleted_SecondTakesTheFirstSlot)
		{
			CStoredHash stored;
			stored.Fill(2);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(0)));

			Assert::AreEqual(1, stored.hash.GetSize());
			ExpectItem(stored.hash, CStoredHash::KeyOf(1), 0, _T("0000.dat"));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_LastItem_MovesNothing)
		{
			CStoredHash stored;
			stored.Fill(5);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(4)));

			Assert::AreEqual(4, stored.hash.GetSize());
			ExpectNoItem(stored.hash, CStoredHash::KeyOf(4));
			ExpectItem(stored.hash, CStoredHash::KeyOf(0), 0, _T("0000.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(1), 1, _T("0001.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(2), 2, _T("0002.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(3), 3, _T("0003.dat"));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_FirstItem_LastItemMovesIntoTheFreedSlot)
		{
			CStoredHash stored;
			stored.Fill(5);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(0)));

			Assert::AreEqual(4, stored.hash.GetSize());
			ExpectNoItem(stored.hash, CStoredHash::KeyOf(0));
			ExpectItem(stored.hash, CStoredHash::KeyOf(4), 0, _T("0000.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(1), 1, _T("0001.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(2), 2, _T("0002.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(3), 3, _T("0003.dat"));
			Assert::IsFalse(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0000.dat"))));
			Assert::AreEqual(TestHelpers::MakeContent(CStoredHash::KeyOf(4)), TestHelpers::ReadFile(stored.GetAbsolute(_T("0000.dat"))));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_MiddleItem_LastItemMovesIntoTheFreedSlot)
		{
			CStoredHash stored;
			stored.Fill(5);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(2)));

			Assert::AreEqual(4, stored.hash.GetSize());
			ExpectNoItem(stored.hash, CStoredHash::KeyOf(2));
			ExpectItem(stored.hash, CStoredHash::KeyOf(4), 2, _T("0002.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(0), 0, _T("0000.dat"));
			Assert::IsFalse(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0000.dat"))));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_ItemInSubFolder_LastItemMovesWithinTheFolder)
		{
			CStoredHash stored;
			stored.Fill(6);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(4)));

			Assert::AreEqual(5, stored.hash.GetSize());
			ExpectItem(stored.hash, CStoredHash::KeyOf(5), 4, _T("0001\\0000.dat"));
			Assert::IsFalse(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0001.dat"))));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_ItemInRootFolder_LastItemMovesOutOfItsSubFolder)
		{
			CStoredHash stored;
			stored.Fill(6);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(1)));

			Assert::AreEqual(5, stored.hash.GetSize());
			ExpectItem(stored.hash, CStoredHash::KeyOf(5), 1, _T("0001.dat"));
			Assert::IsFalse(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0001.dat"))));
			Assert::IsTrue(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0000.dat"))));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_DeeplyNestedLastItem_MovesToTheFreedRootSlot)
		{
			CStoredHash stored;
			stored.Fill(18);

			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(3)));

			Assert::AreEqual(17, stored.hash.GetSize());
			ExpectItem(stored.hash, CStoredHash::KeyOf(17), 3, _T("0003.dat"));
			ExpectItem(stored.hash, CStoredHash::KeyOf(16), 16, _T("0001\\0000\\0000.dat"));
			Assert::IsFalse(TestHelpers::PathExists(stored.GetAbsolute(_T("0001\\0000\\0001.dat"))));
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_SameKeyTwice_SecondCallReturnsFalse)
		{
			CStoredHash stored;
			stored.Fill(3);
			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(1)));

			Assert::IsFalse(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(1)));

			Assert::AreEqual(2, stored.hash.GetSize());
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_ThenStoreAgain_NewFileTakesTheVacatedLastSlot)
		{
			CStoredHash stored;
			stored.Fill(5);
			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(1)));

			const int nLevel = TestHelpers::StoreFile(stored.hash, 5000);

			Assert::AreEqual(4, nLevel);
			ExpectItem(stored.hash, 5000, 4, _T("0001\\0000.dat"));
			Assert::AreEqual(5, stored.hash.GetSize());
			TestHelpers::VerifyNetwork(stored.hash);
		}

		TEST_METHOD(DeleteItem_AllItemsInInsertionOrder_KeepsTheNetworkConsistentUntilItIsEmpty)
		{
			CStoredHash stored;
			const int nCount = 25;
			stored.Fill(nCount);

			for (int nIndex = 0; nIndex < nCount; nIndex++)
			{
				Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(nIndex)));
				TestHelpers::VerifyNetwork(stored.hash);
			}

			Assert::AreEqual(0, stored.hash.GetSize());
		}

		TEST_METHOD(DeleteItem_AllItemsInReverseOrder_KeepsTheNetworkConsistentUntilItIsEmpty)
		{
			CStoredHash stored;
			const int nCount = 25;
			stored.Fill(nCount);

			for (int nIndex = nCount - 1; nIndex >= 0; nIndex--)
			{
				Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(nIndex)));
				TestHelpers::VerifyNetwork(stored.hash);
			}

			Assert::AreEqual(0, stored.hash.GetSize());
		}

		TEST_METHOD(DeleteItem_AllItemsInShuffledOrder_KeepsTheNetworkConsistentUntilItIsEmpty)
		{
			CStoredHash stored;
			const int nCount = 30;
			stored.Fill(nCount);
			std::vector<int> arrOrder(nCount);
			std::iota(arrOrder.begin(), arrOrder.end(), 0);
			std::mt19937 generator(20260101);
			std::shuffle(arrOrder.begin(), arrOrder.end(), generator);

			for (const int nIndex : arrOrder)
			{
				Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(nIndex)));
				TestHelpers::VerifyNetwork(stored.hash);
			}

			Assert::AreEqual(0, stored.hash.GetSize());
		}

		//----------------------------------------------------------------------
		// Half-registered keys (only reachable through the protected maps)
		//----------------------------------------------------------------------

		TEST_METHOD(SearchItem_KeyOnlyInTheLevelMap_ReturnsFalse)
		{
			CTestNetworkHash hash;
			hash.AddLevelOnly(5, 2);

			ExpectNoItem(hash, 5);
		}

		TEST_METHOD(SearchItem_KeyOnlyInThePathMap_ReturnsFalse)
		{
			CTestNetworkHash hash;
			hash.AddPathOnly(5, CString(_T("0002.dat")));

			ExpectNoItem(hash, 5);
		}

		TEST_METHOD(DeleteItem_KeyOnlyInTheLevelMap_ReturnsFalseButDropsTheEntry)
		{
			CTestNetworkHash hash;
			hash.AddLevelOnly(5, 2);

			Assert::IsFalse(hash.DeleteItem(5));

			Assert::AreEqual(0, hash.GetLevelCount());
		}

		TEST_METHOD(DeleteItem_KeyOnlyInThePathMap_ReturnsFalseButDropsTheEntry)
		{
			CTestNetworkHash hash;
			hash.AddPathOnly(5, CString(_T("0002.dat")));

			Assert::IsFalse(hash.DeleteItem(5));

			Assert::AreEqual(0, hash.GetPathCount());
		}

		TEST_METHOD(DeleteItem_HalfRegisteredKeyInANonEmptyHash_LeavesTheRealItemsAlone)
		{
			CTestNetworkHash hash;
			hash.InsertItem(1, 0, _T("0000.dat"));
			hash.AddLevelOnly(5, 1);

			Assert::IsFalse(hash.DeleteItem(5));

			Assert::AreEqual(1, hash.GetSize());
			ExpectItem(hash, 1, 0, _T("0000.dat"));
			Assert::AreEqual(1, hash.GetLevelCount());
		}

		//----------------------------------------------------------------------
		// DeleteItem when the relocation fails (the code under test shows a message box)
		//----------------------------------------------------------------------

		TEST_METHOD(DeleteItem_FileToRelocateIsMissing_ShowsTheSystemErrorAndKeepsTheOldLocation)
		{
			TestHelpers::CTempFolder folder;
			CNetworkHash hash;
			hash.SetRootFolder(folder.GetPath());
			hash.InsertItem(10, 0, _T("0000.dat"));
			hash.InsertItem(20, 1, _T("0001.dat")); // neither file exists on disk

			Assert::IsTrue(hash.DeleteItem(10));

			Assert::AreEqual(1, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(hash.FormatLastError(ERROR_FILE_NOT_FOUND), TestHelpers::CMessageBoxMonitor::GetFirstText());
			TestHelpers::CMessageBoxMonitor::Clear();
			Assert::AreEqual(1, hash.GetSize());
			ExpectNoItem(hash, 10);
			ExpectItem(hash, 20, 1, _T("0001.dat")); // the index is not updated because nothing was moved
		}

		TEST_METHOD(DeleteItem_FreedSlotIsInAMissingFolder_ShowsTheSystemErrorAndLeavesTheFileInPlace)
		{
			TestHelpers::CTempFolder folder;
			CNetworkHash hash;
			hash.SetRootFolder(folder.GetPath());
			hash.InsertItem(10, 0, _T("0001\\0000.dat")); // its folder was never created
			hash.InsertItem(20, 1, _T("0001.dat"));
			TestHelpers::WriteFile(folder.GetEntry(_T("0001.dat")), TestHelpers::MakeContent(20));

			Assert::IsTrue(hash.DeleteItem(10));

			Assert::AreEqual(1, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(hash.FormatLastError(ERROR_PATH_NOT_FOUND), TestHelpers::CMessageBoxMonitor::GetFirstText());
			TestHelpers::CMessageBoxMonitor::Clear();
			ExpectItem(hash, 20, 1, _T("0001.dat"));
			Assert::AreEqual(TestHelpers::MakeContent(20), TestHelpers::ReadFile(folder.GetEntry(_T("0001.dat"))));
		}

		//----------------------------------------------------------------------
		// ExportData
		//----------------------------------------------------------------------

		TEST_METHOD(ExportData_EmptyHash_WritesAnEmptyFile)
		{
			CStoredHash stored;

			Assert::IsTrue(stored.hash.ExportData());

			Assert::IsTrue(TestHelpers::PathExists(stored.GetListFile()));
			Assert::AreEqual(std::string(), TestHelpers::ReadFile(stored.GetListFile()));
		}

		TEST_METHOD(ExportData_Items_WritesOneLinePerItemInInsertionOrder)
		{
			CStoredHash stored;
			stored.hash.InsertItem(30, 4, _T("0001\\0000.dat"));
			stored.hash.InsertItem(10, 0, _T("0000.dat"));
			stored.hash.InsertItem(20, 1, _T("0001.dat"));

			Assert::IsTrue(stored.hash.ExportData());

			const std::vector<CString> arrLines = TestHelpers::ReadLines(stored.GetListFile());
			Assert::AreEqual(3, static_cast<int>(arrLines.size()));
			Assert::AreEqual(CString(_T("30 4 0001\\0000.dat")), arrLines[0]);
			Assert::AreEqual(CString(_T("10 0 0000.dat")), arrLines[1]);
			Assert::AreEqual(CString(_T("20 1 0001.dat")), arrLines[2]);
		}

		TEST_METHOD(ExportData_PathWithSpaces_IsWrittenVerbatim)
		{
			CStoredHash stored;
			stored.hash.InsertItem(7, 3, _T("sub folder\\my file.dat"));

			Assert::IsTrue(stored.hash.ExportData());

			const std::vector<CString> arrLines = TestHelpers::ReadLines(stored.GetListFile());
			Assert::AreEqual(1, static_cast<int>(arrLines.size()));
			Assert::AreEqual(CString(_T("7 3 sub folder\\my file.dat")), arrLines[0]);
		}

		TEST_METHOD(ExportData_ExistingLongerFile_IsTruncated)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), std::string(5000, 'x'));
			stored.hash.InsertItem(10, 0, _T("0000.dat"));

			Assert::IsTrue(stored.hash.ExportData());

			const std::vector<CString> arrLines = TestHelpers::ReadLines(stored.GetListFile());
			Assert::AreEqual(1, static_cast<int>(arrLines.size()));
			Assert::AreEqual(CString(_T("10 0 0000.dat")), arrLines[0]);
		}

		TEST_METHOD(ExportData_Twice_WritesTheSameContent)
		{
			CStoredHash stored;
			stored.hash.InsertItem(10, 0, _T("0000.dat"));
			stored.hash.InsertItem(20, 1, _T("0001.dat"));

			Assert::IsTrue(stored.hash.ExportData());
			const std::string strFirst = TestHelpers::ReadFile(stored.GetListFile());
			Assert::IsTrue(stored.hash.ExportData());

			Assert::AreEqual(strFirst, TestHelpers::ReadFile(stored.GetListFile()));
		}

		TEST_METHOD(ExportData_Items_LeavesTheHashUnchanged)
		{
			CStoredHash stored;
			stored.hash.InsertItem(10, 0, _T("0000.dat"));
			stored.hash.InsertItem(20, 1, _T("0001.dat"));

			Assert::IsTrue(stored.hash.ExportData());

			Assert::AreEqual(2, stored.hash.GetSize());
			ExpectItem(stored.hash, 10, 0, _T("0000.dat"));
			ExpectItem(stored.hash, 20, 1, _T("0001.dat"));
		}

		TEST_METHOD(ExportData_AfterDeleteItem_ListsTheNewLocationOfTheMovedItem)
		{
			CStoredHash stored;
			stored.Fill(5);
			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(0)));

			Assert::IsTrue(stored.hash.ExportData());

			CString strMovedLine;
			strMovedLine.Format(_T("%d 0 0000.dat"), CStoredHash::KeyOf(4));
			const std::vector<CString> arrLines = TestHelpers::ReadLines(stored.GetListFile());
			Assert::AreEqual(4, static_cast<int>(arrLines.size()));
			Assert::AreEqual(strMovedLine, arrLines[3]);
		}

		TEST_METHOD(ExportData_FolderDoesNotExist_ReturnsFalse)
		{
			CNetworkHash hash;
			TestHelpers::CTempFolder folder;
			hash.SetFileList(folder.GetEntry(_T("no\\such\\folder\\files.txt")));
			hash.InsertItem(10, 0, _T("0000.dat"));

			Assert::IsFalse(hash.ExportData());
		}

		TEST_METHOD(ExportData_NoFileListSet_ReturnsFalse)
		{
			CNetworkHash hash;
			hash.InsertItem(10, 0, _T("0000.dat"));

			Assert::IsFalse(hash.ExportData());
		}

		//----------------------------------------------------------------------
		// ImportData
		//----------------------------------------------------------------------

		TEST_METHOD(ImportData_MissingFile_ReturnsFalse)
		{
			CStoredHash stored;

			Assert::IsFalse(stored.hash.ImportData());

			Assert::AreEqual(0, stored.hash.GetSize());
		}

		TEST_METHOD(ImportData_NoFileListSet_ReturnsFalse)
		{
			CNetworkHash hash;

			Assert::IsFalse(hash.ImportData());
		}

		TEST_METHOD(ImportData_EmptyFile_ReturnsTrueAndLoadsNothing)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "");

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(0, stored.hash.GetSize());
		}

		TEST_METHOD(ImportData_ValidLines_LoadsEveryItem)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "1001 0 0000.dat\r\n1002 1 0001.dat\r\n2003 4 0001\\0000.dat\r\n");

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(3, stored.hash.GetSize());
			ExpectItem(stored.hash, 1001, 0, _T("0000.dat"));
			ExpectItem(stored.hash, 1002, 1, _T("0001.dat"));
			ExpectItem(stored.hash, 2003, 4, _T("0001\\0000.dat"));
		}

		TEST_METHOD(ImportData_Lines_KeepTheOrderOfTheFile)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "3003 0 0000.dat\r\n1001 1 0001.dat\r\n2002 2 0002.dat\r\n");

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(3003, stored.hash.GetKey(0));
			Assert::AreEqual(1001, stored.hash.GetKey(1));
			Assert::AreEqual(2002, stored.hash.GetKey(2));
		}

		TEST_METHOD(ImportData_LinesEndingWithLineFeedOnly_LoadsEveryItem)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "1001 0 0000.dat\n1002 1 0001.dat\n");

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(2, stored.hash.GetSize());
			ExpectItem(stored.hash, 1001, 0, _T("0000.dat"));
			ExpectItem(stored.hash, 1002, 1, _T("0001.dat"));
		}

		TEST_METHOD(ImportData_LastLineWithoutLineBreak_IsLoaded)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "1001 0 0000.dat\r\n1002 1 0001.dat");

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(2, stored.hash.GetSize());
			ExpectItem(stored.hash, 1002, 1, _T("0001.dat"));
		}

		TEST_METHOD(ImportData_PathWithSpaces_KeepsTheWholePath)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "7 3 sub folder\\my file.dat\r\n");

			Assert::IsTrue(stored.hash.ImportData());

			ExpectItem(stored.hash, 7, 3, _T("sub folder\\my file.dat"));
		}

		TEST_METHOD(ImportData_MultiDigitKeyAndLevel_AreParsed)
		{
			CStoredHash stored;
			TestHelpers::WriteFile(stored.GetListFile(), "65535 17 0001\\0000\\0001.dat\r\n123 100 0001\\0000\\0000\\0000.dat\r\n");

			Assert::IsTrue(stored.hash.ImportData());

			ExpectItem(stored.hash, 65535, 17, _T("0001\\0000\\0001.dat"));
			ExpectItem(stored.hash, 123, 100, _T("0001\\0000\\0000\\0000.dat"));
		}

		TEST_METHOD(ImportData_ManyLines_LoadsEveryItem)
		{
			CStoredHash stored;
			std::string strContent;
			const int nCount = 300;
			for (int nIndex = 0; nIndex < nCount; nIndex++)
				strContent += std::to_string(500 + nIndex) + " " + std::to_string(nIndex) + " " + std::to_string(nIndex) + ".dat\r\n";
			TestHelpers::WriteFile(stored.GetListFile(), strContent);

			Assert::IsTrue(stored.hash.ImportData());

			Assert::AreEqual(nCount, stored.hash.GetSize());
			for (int nIndex = 0; nIndex < nCount; nIndex++)
			{
				CString strFilePath;
				strFilePath.Format(_T("%d.dat"), nIndex);
				ExpectItem(stored.hash, 500 + nIndex, nIndex, strFilePath);
			}
		}

		//----------------------------------------------------------------------
		// ExportData followed by ImportData
		//----------------------------------------------------------------------

		TEST_METHOD(ExportThenImport_NewHash_HoldsTheSameItems)
		{
			CStoredHash stored;
			const int nCount = 30;
			stored.Fill(nCount);
			Assert::IsTrue(stored.hash.ExportData());
			CNetworkHash restored;
			restored.SetRootFolder(stored.hash.GetRootFolder());
			restored.SetFileList(stored.hash.GetFileList());

			Assert::IsTrue(restored.ImportData());

			Assert::AreEqual(nCount, restored.GetSize());
			for (int nIndex = 0; nIndex < nCount; nIndex++)
			{
				Assert::AreEqual(stored.hash.GetKey(nIndex), restored.GetKey(nIndex));
				int nExpectedLevel = -1, nLevel = -1;
				CString strExpectedPath, strFilePath;
				Assert::IsTrue(stored.hash.SearchItem(stored.hash.GetKey(nIndex), nExpectedLevel, strExpectedPath));
				Assert::IsTrue(restored.SearchItem(restored.GetKey(nIndex), nLevel, strFilePath));
				Assert::AreEqual(nExpectedLevel, nLevel);
				Assert::AreEqual(strExpectedPath, strFilePath);
			}
			TestHelpers::VerifyNetwork(restored);
		}

		TEST_METHOD(ExportThenImport_AfterDeletions_RestoredHashKeepsWorking)
		{
			CStoredHash stored;
			stored.Fill(12);
			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(2)));
			Assert::IsTrue(TestHelpers::RemoveStoredFile(stored.hash, CStoredHash::KeyOf(9)));
			Assert::IsTrue(stored.hash.ExportData());
			CNetworkHash restored;
			restored.SetRootFolder(stored.hash.GetRootFolder());
			restored.SetFileList(stored.hash.GetFileList());
			Assert::IsTrue(restored.ImportData());

			Assert::IsTrue(TestHelpers::RemoveStoredFile(restored, CStoredHash::KeyOf(0)));
			TestHelpers::StoreFile(restored, 7777);

			Assert::AreEqual(10, restored.GetSize());
			TestHelpers::VerifyNetwork(restored);
		}
	};
}
