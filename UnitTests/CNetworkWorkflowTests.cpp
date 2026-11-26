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

// CNetworkWorkflowTests.cpp : tests of the Browse, Upload, Download and Delete workflows on a CNetworkHash
//

#include "stdafx.h"
#include "TestHelpers.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	namespace
	{
		/// The outcome of a replayed button click.
		struct CReply
		{
			CReply() : m_bDone(false) {}

			bool m_bDone;         ///< true when the click carried out its job
			CString m_strMessage; ///< the text the dialog shows in its message line
			CString m_strCode;    ///< the network code the dialog shows after an upload
		};

		/// Fails the test unless the request was carried out.
		void ExpectDone(const CReply& reply)
		{
			Assert::IsTrue(reply.m_bDone, reply.m_strMessage);
		}

		/// Fails the test unless the request was refused with exactly this message.
		void ExpectRejected(const CReply& reply, LPCTSTR lpszMessage)
		{
			Assert::IsFalse(reply.m_bDone, L"The request was carried out");
			Assert::AreEqual(CString(lpszMessage), reply.m_strMessage);
		}

		/**
		 * Drives a CNetworkHash through the calls that the Browse, Upload, Download and Delete handlers of
		 * CSiatelHomeworkDlg (SiatelHomeworkDlg.cpp) make, in the same order, on real folders. The dialog cannot be
		 * created without a window and the application object, so these replays stand in for it.
		 * The session also records what every stored file should contain, for the consistency checks.
		 * The files to upload and the downloaded copies live in a second folder, outside of the network.
		 */
		class CNetworkSession
		{
		public:
			/// The seed makes rand(), and with it every generated code, the same on every run.
			explicit CNetworkSession(unsigned int nSeed = 1)
				: m_pHash(std::make_unique<CNetworkHash>())
				, m_nFileNumber(0)
			{
				srand(nSeed);
				Browse(m_folderNetwork.GetPath());
			}

			CNetworkHash& GetHash() { return *m_pHash; }
			CString GetRootFolder() const { return m_strRootFolder; }
			CString GetFilesFolder() const { return m_folderFiles.GetPath(); }
			const std::map<int, std::string>& GetContent() const { return m_mapContent; }

			//------------------------------------------------------------------
			// Replays of the handlers
			//------------------------------------------------------------------

			/// Browse: selects a folder, makes sure that its path ends with a backslash, and loads its file list.
			/// Returns whether a file list was loaded.
			bool Browse(CString strFolder)
			{
				if (strFolder.Right(1) != _T("\\"))
					strFolder += _T("\\");
				m_strRootFolder = strFolder;
				m_pHash->RemoveAll();
				m_pHash->SetFileList(m_strRootFolder + _T("filelist.txt"));
				m_pHash->SetRootFolder(m_strRootFolder);
				return m_pHash->ImportData();
			}

			/// Closing the dialog: the file list is written.
			void Close()
			{
				Assert::IsTrue(m_pHash->ExportData(), L"Unable to write the file list");
			}

			/// Closing the application and starting it again on the same folder.
			void Restart()
			{
				Close();
				m_pHash = std::make_unique<CNetworkHash>();
				Browse(m_strRootFolder);
			}

			/// Upload: copies a file into the network under a newly generated code.
			CReply Upload(const CString& strInputFile)
			{
				return UploadWithKey(m_pHash->GenerateID(), strInputFile);
			}

			/// Upload under a chosen ID, as if the generator had returned it.
			CReply UploadWithKey(int nFileCode, const CString& strInputFile)
			{
				CReply reply;
				CStringArray arrNetworkPath;
				const int nPathCode = m_pHash->GetSize();
				const CString strFileCode = m_pHash->EncodeNetworkID(nFileCode);
				const CString strFilePath = m_pHash->GetFilePath(nPathCode, arrNetworkPath);
				Assert::IsTrue(m_pHash->CreateNetworkPath(m_strRootFolder, arrNetworkPath), L"Unable to create the folders of the new file");
				const CString strFilename = m_strRootFolder + strFilePath;

				BOOL bCancelOperation = FALSE;
				if (!CopyFileEx(strInputFile, strFilename, nullptr, nullptr, &bCancelOperation, 0))
				{
					reply.m_strMessage = m_pHash->FormatLastError(GetLastError());
					return reply;
				}

				Assert::IsTrue(m_pHash->InsertItem(nFileCode, nPathCode, strFilePath), L"Unable to register the new file");
				reply.m_bDone = true;
				reply.m_strCode = strFileCode;
				reply.m_strMessage.Format(_T("Done writing file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(strFileCode));
				return reply;
			}

			/// Download: copies the file that has the code to a chosen path.
			CReply Download(const CString& strCode, const CString& strOutputFile)
			{
				CReply reply;
				if (!m_pHash->IsValidCode(strCode))
				{
					reply.m_strMessage = INVALID_CHARS;
					return reply;
				}

				int nLevel = 0;
				CString strFilePath;
				if (!m_pHash->SearchItem(m_pHash->DecodeNetworkID(strCode), nLevel, strFilePath))
				{
					reply.m_strMessage = FILE_DOES_NOT_EXISTS;
					return reply;
				}

				const CString strFilename = m_strRootFolder + strFilePath;
				BOOL bCancelOperation = FALSE;
				if (!CopyFileEx(strFilename, strOutputFile, nullptr, nullptr, &bCancelOperation, 0))
				{
					const DWORD dwLastError = GetLastError();
					if (!m_pHash->FileExists(strFilename))
						reply.m_strMessage = FILE_DOES_NOT_EXISTS;
					else
						reply.m_strMessage = m_pHash->FormatLastError(dwLastError);
					return reply;
				}

				reply.m_bDone = true;
				reply.m_strMessage.Format(_T("Done reading file %s [%s] ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(strCode));
				return reply;
			}

			/// Delete: removes the file that has the code from the disk and from the index.
			CReply Delete(const CString& strCode)
			{
				CReply reply;
				if (!m_pHash->IsValidCode(strCode))
				{
					reply.m_strMessage = INVALID_CHARS;
					return reply;
				}

				int nLevel = 0;
				CString strFilePath;
				const int nFileCode = m_pHash->DecodeNetworkID(strCode);
				if (!m_pHash->SearchItem(nFileCode, nLevel, strFilePath))
				{
					reply.m_strMessage = FILE_DOES_NOT_EXISTS;
					return reply;
				}

				const CString strFilename = m_strRootFolder + strFilePath;
				if (!m_pHash->FileExists(strFilename))
				{
					reply.m_strMessage = FILE_DOES_NOT_EXISTS;
					return reply;
				}
				if (!DeleteFile(strFilename))
				{
					reply.m_strMessage = m_pHash->FormatLastError(GetLastError());
					return reply;
				}

				Assert::IsTrue(m_pHash->DeleteItem(nFileCode), L"Unable to remove the file from the index");
				reply.m_bDone = true;
				reply.m_strMessage.Format(_T("File %s [%s] has been deleted ..."), static_cast<LPCWSTR>(strFilename), static_cast<LPCWSTR>(strCode));
				return reply;
			}

			//------------------------------------------------------------------
			// Conveniences that expect success and keep the record of the files
			//------------------------------------------------------------------

			/// Gets the path of a file that does not exist yet, in the folder of the files to upload and download.
			CString MakeUnusedPath()
			{
				CString strName;
				strName.Format(_T("file%04d.bin"), m_nFileNumber++);
				return m_folderFiles.GetEntry(strName);
			}

			/// Creates a file with the given content, outside of the network.
			CString MakeInputFile(const std::string& strContent)
			{
				const CString strPath = MakeUnusedPath();
				TestHelpers::WriteFile(strPath, strContent);
				return strPath;
			}

			/// Uploads a new file, which must succeed, and returns its code.
			CString StoreText(const std::string& strContent)
			{
				const CReply reply = Upload(MakeInputFile(strContent));
				ExpectDone(reply);
				Record(reply.m_strCode, strContent);
				return reply.m_strCode;
			}

			/// Uploads a new file under a chosen ID, which must succeed.
			void StoreTextAs(int nKey, const std::string& strContent)
			{
				ExpectDone(UploadWithKey(nKey, MakeInputFile(strContent)));
				Record(m_pHash->EncodeNetworkID(nKey), strContent);
			}

			/// Downloads a file, which must succeed, and returns what was copied.
			std::string FetchText(const CString& strCode)
			{
				const CString strOutput = MakeUnusedPath();
				ExpectDone(Download(strCode, strOutput));
				return TestHelpers::ReadFile(strOutput);
			}

			/// Deletes a file, which must succeed.
			void RemoveFile(const CString& strCode)
			{
				ExpectDone(Delete(strCode));
				m_mapContent.erase(m_pHash->DecodeNetworkID(strCode));
			}

			/// Fails the test unless the index, the disk and the record agree, and every file can be downloaded.
			void VerifyAll()
			{
				TestHelpers::VerifyNetwork(*m_pHash, m_mapContent);
				for (const auto& pairFile : m_mapContent)
					Assert::AreEqual(pairFile.second, FetchText(m_pHash->EncodeNetworkID(pairFile.first)));
			}

		private:
			void Record(const CString& strCode, const std::string& strContent)
			{
				const bool bNewKey = m_mapContent.emplace(m_pHash->DecodeNetworkID(strCode), strContent).second;
				Assert::IsTrue(bNewKey, L"Two files were stored under the same key");
			}

			TestHelpers::CTempFolder m_folderNetwork; ///< the root folder of the network
			TestHelpers::CTempFolder m_folderFiles;   ///< the files to upload and the downloaded copies
			std::unique_ptr<CNetworkHash> m_pHash;
			CString m_strRootFolder;
			int m_nFileNumber;
			std::map<int, std::string> m_mapContent;  ///< what every indexed key should hold
		};

		/// Fails the test unless the code is indexed at the given level, with the path that belongs to that level.
		void ExpectLevel(CNetworkSession& session, const CString& strCode, int nExpectedLevel)
		{
			int nLevel = -1;
			CString strFilePath;
			CStringArray arrFolders;

			Assert::IsTrue(session.GetHash().SearchItem(session.GetHash().DecodeNetworkID(strCode), nLevel, strFilePath), strCode);
			Assert::AreEqual(nExpectedLevel, nLevel, strCode);
			Assert::AreEqual(session.GetHash().GetFilePath(nExpectedLevel, arrFolders), strFilePath, strCode);
		}

		/// Stores one file per entry of the order, deletes them in that order and checks the network after every step.
		void StoreThenDeleteAll(const std::vector<size_t>& arrOrder)
		{
			CNetworkSession session(31);
			std::vector<CString> arrCodes;
			for (size_t nIndex = 0; nIndex < arrOrder.size(); nIndex++)
				arrCodes.push_back(session.StoreText("file number " + std::to_string(nIndex)));
			session.VerifyAll();

			for (const size_t nIndex : arrOrder)
			{
				session.RemoveFile(arrCodes[nIndex]);
				TestHelpers::VerifyNetwork(session.GetHash(), session.GetContent());
			}

			Assert::AreEqual(0, session.GetHash().GetSize());
		}
	}

	TEST_CLASS(CNetworkWorkflowTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		//----------------------------------------------------------------------
		// Upload
		//----------------------------------------------------------------------

		TEST_METHOD(Upload_FirstFile_IsStoredAtLevelZeroUnderAValidCode)
		{
			CNetworkSession session;
			const std::string strContent = "the very first file";

			const CReply reply = session.Upload(session.MakeInputFile(strContent));

			ExpectDone(reply);
			Assert::AreEqual(MAX_CODE_LENGTH, reply.m_strCode.GetLength());
			Assert::IsTrue(session.GetHash().IsValidCode(reply.m_strCode));
			Assert::AreEqual(1, session.GetHash().GetSize());
			ExpectLevel(session, reply.m_strCode, 0);
			Assert::AreEqual(strContent, TestHelpers::ReadFile(session.GetRootFolder() + _T("0000.dat")));
		}

		TEST_METHOD(Upload_SeveralFiles_AreStoredAtConsecutiveLevels)
		{
			CNetworkSession session;
			std::vector<CString> arrCodes;
			for (int nIndex = 0; nIndex < 6; nIndex++)
				arrCodes.push_back(session.StoreText("file " + std::to_string(nIndex)));

			Assert::AreEqual(6, session.GetHash().GetSize());
			for (int nIndex = 0; nIndex < 6; nIndex++)
				ExpectLevel(session, arrCodes[nIndex], nIndex);
			session.VerifyAll();
		}

		TEST_METHOD(Upload_SeventyFiles_AcrossTheFolderLevels_AreAllRetrievable)
		{
			// Levels 0-3 are stored in the root folder, 4-15 one subfolder down, 16-63 two down and 64-69 three down.
			CNetworkSession session;
			for (int nIndex = 0; nIndex < 70; nIndex++)
				session.StoreText("file " + std::to_string(nIndex) + " of the many");

			Assert::AreEqual(70, session.GetHash().GetSize());
			session.VerifyAll();
		}

		TEST_METHOD(Upload_HundredFiles_EachGetsItsOwnCode)
		{
			CNetworkSession session;
			for (int nIndex = 0; nIndex < 100; nIndex++)
				session.StoreText("small file " + std::to_string(nIndex)); // a repeated key would fail inside StoreText

			Assert::AreEqual(100, session.GetHash().GetSize());
			TestHelpers::VerifyNetwork(session.GetHash(), session.GetContent());
		}

		TEST_METHOD(Upload_WhenTheNextRandomIdIsTaken_UsesTheFollowingOne)
		{
			CNetworkSession session;
			srand(5);
			const int nTakenKey = session.GetHash().GetNextID();
			const int nFollowingKey = session.GetHash().GetNextID();
			Assert::AreNotEqual(nTakenKey, nFollowingKey, L"The seed must give two different draws");
			session.StoreTextAs(nTakenKey, "already there");

			srand(5); // the generator will offer the taken ID first
			const CString strCode = session.StoreText("the newcomer");

			Assert::AreEqual(session.GetHash().EncodeNetworkID(nFollowingKey), strCode);
			Assert::AreEqual(2, session.GetHash().GetSize());
			session.VerifyAll();
		}

		TEST_METHOD(Upload_EmptyFile_IsStoredAndRetrievedEmpty)
		{
			CNetworkSession session;

			const CString strCode = session.StoreText("");

			Assert::IsTrue(TestHelpers::PathExists(session.GetRootFolder() + _T("0000.dat")));
			Assert::AreEqual(std::string(), session.FetchText(strCode));
		}

		TEST_METHOD(Upload_BinaryContent_IsStoredByteForByte)
		{
			CNetworkSession session;
			std::string strContent;
			for (int nIndex = 0; nIndex < 4096; nIndex++)
				strContent.push_back(static_cast<char>(nIndex % 256)); // every byte value, including NUL and Ctrl+Z
			strContent += "\r\n\n\r"; // line terminators must stay as they are

			const CString strCode = session.StoreText(strContent);

			Assert::AreEqual(strContent, TestHelpers::ReadFile(session.GetRootFolder() + _T("0000.dat")));
			Assert::AreEqual(strContent, session.FetchText(strCode));
		}

		TEST_METHOD(Upload_MissingInputFile_RegistersNothing)
		{
			CNetworkSession session;

			const CReply reply = session.Upload(session.MakeUnusedPath());

			Assert::IsFalse(reply.m_bDone);
			Assert::IsTrue(reply.m_strCode.IsEmpty());
			Assert::IsFalse(reply.m_strMessage.IsEmpty(), L"The system error text is missing");
			Assert::AreEqual(0, session.GetHash().GetSize());
			session.VerifyAll();
		}

		TEST_METHOD(Upload_AfterAFailedUpload_ReusesTheLevelAndTheFolders)
		{
			CNetworkSession session;
			for (int nIndex = 0; nIndex < 4; nIndex++) // levels 0-3 fill the root folder
				session.StoreText("root file " + std::to_string(nIndex));

			// Level 4 lives in a new subfolder, which the failed attempt creates before the copy fails.
			Assert::IsFalse(session.Upload(session.MakeUnusedPath()).m_bDone);
			Assert::AreEqual(4, session.GetHash().GetSize());
			session.VerifyAll();

			const CString strCode = session.StoreText("fifth file");

			ExpectLevel(session, strCode, 4);
			session.VerifyAll();
		}

		//----------------------------------------------------------------------
		// Download
		//----------------------------------------------------------------------

		TEST_METHOD(Download_ReturnsWhatWasUploaded)
		{
			CNetworkSession session;
			const std::string strContent = "content to be stored and read back";
			const CString strCode = session.StoreText(strContent);
			const CString strOutput = session.MakeUnusedPath();

			ExpectDone(session.Download(strCode, strOutput));

			Assert::AreEqual(strContent, TestHelpers::ReadFile(strOutput));
		}

		TEST_METHOD(Download_EachCodeReturnsItsOwnFile)
		{
			CNetworkSession session;
			std::vector<CString> arrCodes;
			for (int nIndex = 0; nIndex < 8; nIndex++)
				arrCodes.push_back(session.StoreText("payload " + std::to_string(nIndex)));

			for (int nIndex = 7; nIndex >= 0; nIndex--)
				Assert::AreEqual("payload " + std::to_string(nIndex), session.FetchText(arrCodes[nIndex]));
		}

		TEST_METHOD(Download_Twice_ReturnsTheSameContentAndChangesNothing)
		{
			CNetworkSession session;
			const CString strCode = session.StoreText("read me twice");

			Assert::AreEqual(std::string("read me twice"), session.FetchText(strCode));
			Assert::AreEqual(std::string("read me twice"), session.FetchText(strCode));

			Assert::AreEqual(1, session.GetHash().GetSize());
			session.VerifyAll();
		}

		TEST_METHOD(Download_InvalidCode_IsRejectedBeforeTheLookup)
		{
			CNetworkSession session;
			session.StoreText("stays untouched");
			// Lower case, separators and non-ASCII letters are not digits of a code, so a code cannot name a path either.
			const LPCTSTR arrInvalidCodes[] = { _T("abcd"), _T("AB-D"), _T("12 4"), _T("\u00C4BCD"), _T("..\\A"), _T("A/B"), _T(" ") };

			for (const LPCTSTR lpszCode : arrInvalidCodes)
			{
				const CString strOutput = session.MakeUnusedPath();

				ExpectRejected(session.Download(lpszCode, strOutput), INVALID_CHARS);

				Assert::IsFalse(TestHelpers::PathExists(strOutput), lpszCode);
			}
			session.VerifyAll();
		}

		TEST_METHOD(Download_UnknownCode_ReportsThatThereIsNoFile)
		{
			CNetworkSession session;
			session.StoreText("one");
			session.StoreText("two");
			// GenerateID never returns an ID that is in use; 0000 is never generated and ZZZZ is out of the range.
			const CString arrUnknownCodes[] = { session.GetHash().EncodeNetworkID(session.GetHash().GenerateID()), CString(_T("0000")), CString(_T("ZZZZ")) };

			for (const CString& strCode : arrUnknownCodes)
			{
				const CString strOutput = session.MakeUnusedPath();

				ExpectRejected(session.Download(strCode, strOutput), FILE_DOES_NOT_EXISTS);

				Assert::IsFalse(TestHelpers::PathExists(strOutput), strCode);
			}
			session.VerifyAll();
		}

		TEST_METHOD(Download_EmptyCode_ReportsThatThereIsNoFile)
		{
			CNetworkSession session;
			session.StoreText("one");
			const CString strOutput = session.MakeUnusedPath();

			// An empty code is valid and decodes to the key 0, which is never in use.
			ExpectRejected(session.Download(CString(), strOutput), FILE_DOES_NOT_EXISTS);

			Assert::IsFalse(TestHelpers::PathExists(strOutput));
		}

		TEST_METHOD(Download_CodeWithoutLeadingZeros_FindsTheSameFile)
		{
			CNetworkSession session;
			session.StoreTextAs(10, "the file with key ten");
			Assert::AreEqual(CString(_T("000A")), session.GetHash().EncodeNetworkID(10));

			Assert::AreEqual(std::string("the file with key ten"), session.FetchText(CString(_T("000A"))));
			Assert::AreEqual(std::string("the file with key ten"), session.FetchText(CString(_T("0A"))));
			Assert::AreEqual(std::string("the file with key ten"), session.FetchText(CString(_T("A"))));
			ExpectRejected(session.Download(CString(_T("a")), session.MakeUnusedPath()), INVALID_CHARS);
		}

		TEST_METHOD(Download_FileMissingOnDisk_ReportsThatThereIsNoFile)
		{
			CNetworkSession session;
			const CString strCode = session.StoreText("will vanish");
			int nLevel = -1;
			CString strFilePath;
			Assert::IsTrue(session.GetHash().SearchItem(session.GetHash().DecodeNetworkID(strCode), nLevel, strFilePath));
			Assert::IsTrue(DeleteFile(session.GetRootFolder() + strFilePath) != FALSE);
			const CString strOutput = session.MakeUnusedPath();

			ExpectRejected(session.Download(strCode, strOutput), FILE_DOES_NOT_EXISTS);

			Assert::IsFalse(TestHelpers::PathExists(strOutput));
			Assert::AreEqual(1, session.GetHash().GetSize());
		}

		TEST_METHOD(Download_ToAMissingFolder_ReportsTheSystemErrorInsteadOfNoFile)
		{
			CNetworkSession session;
			const CString strCode = session.StoreText("content");
			const CString strOutput = session.GetFilesFolder() + _T("no such folder\\copy.bin");

			const CReply reply = session.Download(strCode, strOutput);

			Assert::IsFalse(reply.m_bDone);
			Assert::AreEqual(session.GetHash().FormatLastError(ERROR_PATH_NOT_FOUND), reply.m_strMessage);
			Assert::AreNotEqual(CString(FILE_DOES_NOT_EXISTS), reply.m_strMessage);
			session.VerifyAll();
		}

		//----------------------------------------------------------------------
		// Delete
		//----------------------------------------------------------------------

		TEST_METHOD(Delete_RemovesTheFileAndItsIndexEntry)
		{
			CNetworkSession session;
			const CString strFirst = session.StoreText("first");
			const CString strSecond = session.StoreText("second");
			const CString strThird = session.StoreText("third");

			session.RemoveFile(strSecond);

			Assert::AreEqual(2, session.GetHash().GetSize());
			session.VerifyAll();
			Assert::AreEqual(std::string("first"), session.FetchText(strFirst));
			Assert::AreEqual(std::string("third"), session.FetchText(strThird));
			ExpectRejected(session.Download(strSecond, session.MakeUnusedPath()), FILE_DOES_NOT_EXISTS);
		}

		TEST_METHOD(Delete_InvalidCode_IsRejectedAndChangesNothing)
		{
			CNetworkSession session;
			session.StoreText("one");
			session.StoreText("two");
			const LPCTSTR arrInvalidCodes[] = { _T("abcd"), _T("00-1"), _T("A B"), _T("\u00C4BCD") };

			for (const LPCTSTR lpszCode : arrInvalidCodes)
				ExpectRejected(session.Delete(lpszCode), INVALID_CHARS);

			session.VerifyAll();
		}

		TEST_METHOD(Delete_UnknownCode_ReportsThatThereIsNoFileAndChangesNothing)
		{
			CNetworkSession session;
			session.StoreText("one");
			session.StoreText("two");
			const CString strUnknown = session.GetHash().EncodeNetworkID(session.GetHash().GenerateID());

			ExpectRejected(session.Delete(strUnknown), FILE_DOES_NOT_EXISTS);
			ExpectRejected(session.Delete(CString()), FILE_DOES_NOT_EXISTS);

			session.VerifyAll();
		}

		TEST_METHOD(Delete_SameCodeTwice_TheSecondAttemptFindsNothing)
		{
			CNetworkSession session;
			const CString strCode = session.StoreText("delete me");
			session.StoreText("keep me");
			session.RemoveFile(strCode);

			ExpectRejected(session.Delete(strCode), FILE_DOES_NOT_EXISTS);

			session.VerifyAll();
		}

		TEST_METHOD(Delete_TheOnlyFile_LeavesAnEmptyNetworkThatAcceptsNewFiles)
		{
			CNetworkSession session;
			session.RemoveFile(session.StoreText("alone"));

			Assert::AreEqual(0, session.GetHash().GetSize());
			session.VerifyAll();

			const CString strCode = session.StoreText("again");

			ExpectLevel(session, strCode, 0);
			session.VerifyAll();
		}

		TEST_METHOD(Delete_FirstOfThree_MovesTheLastFileIntoItsSlot)
		{
			CNetworkSession session;
			const CString strFirst = session.StoreText("first");
			const CString strSecond = session.StoreText("second");
			const CString strThird = session.StoreText("third");

			session.RemoveFile(strFirst);

			ExpectLevel(session, strThird, 0);
			ExpectLevel(session, strSecond, 1);
			Assert::AreEqual(std::string("third"), TestHelpers::ReadFile(session.GetRootFolder() + _T("0000.dat")));
			Assert::IsFalse(TestHelpers::PathExists(session.GetRootFolder() + _T("0002.dat")));
			session.VerifyAll();
		}

		TEST_METHOD(Delete_LastOfThree_LeavesTheOthersInPlace)
		{
			CNetworkSession session;
			const CString strFirst = session.StoreText("first");
			const CString strSecond = session.StoreText("second");
			const CString strThird = session.StoreText("third");

			session.RemoveFile(strThird);

			ExpectLevel(session, strFirst, 0);
			ExpectLevel(session, strSecond, 1);
			session.VerifyAll();
		}

		TEST_METHOD(Delete_ThenUpload_ReusesTheFreedLevel)
		{
			CNetworkSession session;
			session.StoreText("first");
			const CString strSecond = session.StoreText("second");
			const CString strThird = session.StoreText("third");

			session.RemoveFile(strSecond);
			const CString strFourth = session.StoreText("fourth");

			ExpectLevel(session, strThird, 1);
			ExpectLevel(session, strFourth, 2);
			session.VerifyAll();
		}

		TEST_METHOD(DeleteAll_InUploadOrder_KeepsTheNetworkConsistentAtEveryStep)
		{
			// 24 files cross the boundary between the root folder and its subfolders and the one between the first two levels.
			std::vector<size_t> arrOrder(24);
			std::iota(arrOrder.begin(), arrOrder.end(), static_cast<size_t>(0));

			StoreThenDeleteAll(arrOrder);
		}

		TEST_METHOD(DeleteAll_InReverseOrder_KeepsTheNetworkConsistentAtEveryStep)
		{
			std::vector<size_t> arrOrder(24);
			std::iota(arrOrder.begin(), arrOrder.end(), static_cast<size_t>(0));
			std::reverse(arrOrder.begin(), arrOrder.end());

			StoreThenDeleteAll(arrOrder);
		}

		TEST_METHOD(DeleteAll_InShuffledOrder_KeepsTheNetworkConsistentAtEveryStep)
		{
			std::vector<size_t> arrOrder(24);
			std::iota(arrOrder.begin(), arrOrder.end(), static_cast<size_t>(0));
			std::shuffle(arrOrder.begin(), arrOrder.end(), std::mt19937(2024));

			StoreThenDeleteAll(arrOrder);
		}

		TEST_METHOD(RandomUploadsAndDeletes_KeepTheIndexAndTheDiskConsistent)
		{
			CNetworkSession session(777);
			std::mt19937 generator(4242);
			int nUploads = 0;
			int nDeletes = 0;
			size_t nPeak = 0;

			for (int nStep = 1; nStep <= 200; nStep++)
			{
				if (session.GetContent().empty() || (generator() % 100 < 60))
				{
					session.StoreText("upload " + std::to_string(nUploads++) + std::string(generator() % 40, '.'));
					nPeak = (std::max)(nPeak, session.GetContent().size());
				}
				else
				{
					auto itFile = session.GetContent().begin();
					std::advance(itFile, static_cast<std::ptrdiff_t>(generator() % session.GetContent().size()));
					session.RemoveFile(session.GetHash().EncodeNetworkID(itFile->first));
					nDeletes++;
				}
				if (nStep % 10 == 0)
					TestHelpers::VerifyNetwork(session.GetHash(), session.GetContent());
			}

			session.VerifyAll();
			Assert::IsTrue(nDeletes >= 30, L"The sequence deleted too few files to be meaningful");
			Assert::IsTrue(nPeak > 16, L"The network never grew past the second folder level");
		}

		//----------------------------------------------------------------------
		// Closing, restarting and browsing
		//----------------------------------------------------------------------

		TEST_METHOD(FirstUseOfAFolder_StartsEmptyAndTheFileListAppearsOnClose)
		{
			CNetworkSession session;
			const CString strFileList = session.GetRootFolder() + _T("filelist.txt");

			Assert::AreEqual(0, session.GetHash().GetSize());
			Assert::IsFalse(TestHelpers::PathExists(strFileList));

			session.StoreText("first ever file");
			session.Close();

			Assert::IsTrue(TestHelpers::PathExists(strFileList));
			Assert::AreEqual(static_cast<size_t>(1), TestHelpers::ReadLines(strFileList).size());
		}

		TEST_METHOD(Restart_KeepsEveryStoredFileRetrievable)
		{
			CNetworkSession session;
			for (int nIndex = 0; nIndex < 10; nIndex++)
				session.StoreText("file " + std::to_string(nIndex));

			session.Restart();

			Assert::AreEqual(10, session.GetHash().GetSize());
			Assert::AreEqual(static_cast<size_t>(10), TestHelpers::ReadLines(session.GetRootFolder() + _T("filelist.txt")).size());
			session.VerifyAll();
		}

		TEST_METHOD(Restart_ThenDeleteAndUpload_ThenRestartAgain_KeepsWorking)
		{
			CNetworkSession session;
			std::vector<CString> arrCodes;
			for (int nIndex = 0; nIndex < 8; nIndex++)
				arrCodes.push_back(session.StoreText("before the restart " + std::to_string(nIndex)));
			session.Restart();

			session.RemoveFile(arrCodes[1]);
			session.RemoveFile(arrCodes[6]);
			session.StoreText("after the restart 1");
			session.StoreText("after the restart 2");
			session.VerifyAll();
			session.Restart();

			Assert::AreEqual(8, session.GetHash().GetSize());
			session.VerifyAll();
		}

		TEST_METHOD(Restart_AfterDeletingEverything_StartsEmpty)
		{
			CNetworkSession session;
			std::vector<CString> arrCodes;
			for (int nIndex = 0; nIndex < 3; nIndex++)
				arrCodes.push_back(session.StoreText("file " + std::to_string(nIndex)));
			for (const CString& strCode : arrCodes)
				session.RemoveFile(strCode);

			session.Restart();

			Assert::AreEqual(0, session.GetHash().GetSize());
			const CString strCode = session.StoreText("a new beginning");
			ExpectLevel(session, strCode, 0);
			session.VerifyAll();
		}

		TEST_METHOD(Browse_AnotherFolder_ShowsOnlyThatFoldersFiles)
		{
			CNetworkSession sessionOne(11);
			const CString strInOne = sessionOne.StoreText("stored in the first folder");
			CNetworkSession sessionTwo(22);
			const CString strInTwo = sessionTwo.StoreText("stored in the second folder");
			sessionTwo.StoreText("another one in the second folder");
			sessionTwo.Close();
			Assert::AreNotEqual(strInOne, strInTwo, L"The seeds must give different codes");

			Assert::IsTrue(sessionOne.Browse(sessionTwo.GetRootFolder()));

			Assert::AreEqual(2, sessionOne.GetHash().GetSize());
			TestHelpers::VerifyNetwork(sessionOne.GetHash(), sessionTwo.GetContent());
			Assert::AreEqual(std::string("stored in the second folder"), sessionOne.FetchText(strInTwo));
			ExpectRejected(sessionOne.Download(strInOne, sessionOne.MakeUnusedPath()), FILE_DOES_NOT_EXISTS);
		}
	};
}
