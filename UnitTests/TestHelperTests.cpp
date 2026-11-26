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

// TestHelperTests.cpp : unit tests for the monitors that guard the other tests
//

#include "stdafx.h"
#include "TestHelpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	namespace
	{
		/// Shows a message box that stays on screen until somebody answers it, the way the code under test does.
		int ShowMessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption)
		{
			return ::MessageBox(nullptr, lpszText, lpszCaption, MB_OKCANCEL | MB_ICONQUESTION);
		}

		/// Tells whether a check reports a failure. Assert::Fail raises a structured exception that catch (...) does not see.
		bool ReportsFailure(void (*pfnCheck)())
		{
			__try
			{
				pfnCheck();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return true;
			}
			return false;
		}
	}

	TEST_CLASS(CMessageBoxMonitorTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		TEST_METHOD(Begin_NoMessageBoxShown_HasNothingRecorded)
		{
			Assert::AreEqual(0, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::IsTrue(TestHelpers::CMessageBoxMonitor::GetFirstText().IsEmpty());
			Assert::IsTrue(TestHelpers::CMessageBoxMonitor::GetFirstCaption().IsEmpty());
			TestHelpers::CMessageBoxMonitor::VerifyNone();
		}

		TEST_METHOD(MessageBox_WhileWatched_IsClosedWithOkAndItsTextAndCaptionAreRecorded)
		{
			const int nAnswer = ShowMessageBox(_T("the text"), _T("the caption"));

			Assert::AreEqual(static_cast<int>(IDOK), nAnswer);
			Assert::AreEqual(1, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(CString(_T("the text")), TestHelpers::CMessageBoxMonitor::GetFirstText());
			Assert::AreEqual(CString(_T("the caption")), TestHelpers::CMessageBoxMonitor::GetFirstCaption());
			TestHelpers::CMessageBoxMonitor::Clear();
		}

		TEST_METHOD(AfxMessageBox_WhileWatched_IsClosedWithOkAndItsTextIsRecorded)
		{
			// This is the call CNetworkHash::DeleteItem makes when it cannot move a file.
			const int nAnswer = AfxMessageBox(_T("Access denied."), MB_OK | MB_ICONSTOP);

			Assert::AreEqual(static_cast<int>(IDOK), nAnswer);
			Assert::AreEqual(1, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(CString(_T("Access denied.")), TestHelpers::CMessageBoxMonitor::GetFirstText());
			Assert::IsFalse(TestHelpers::CMessageBoxMonitor::GetFirstCaption().IsEmpty());
			TestHelpers::CMessageBoxMonitor::Clear();
		}

		TEST_METHOD(MessageBox_SeveralWhileWatched_AreAllCountedAndTheFirstOneIsKept)
		{
			ShowMessageBox(_T("first text"), _T("first caption"));
			ShowMessageBox(_T("second text"), _T("second caption"));
			ShowMessageBox(_T("third text"), _T("third caption"));

			Assert::AreEqual(3, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(CString(_T("first text")), TestHelpers::CMessageBoxMonitor::GetFirstText());
			Assert::AreEqual(CString(_T("first caption")), TestHelpers::CMessageBoxMonitor::GetFirstCaption());
			TestHelpers::CMessageBoxMonitor::Clear();
		}

		TEST_METHOD(Clear_AfterAMessageBox_ForgetsItAndVerifyNoneAccepts)
		{
			ShowMessageBox(_T("text"), _T("caption"));

			TestHelpers::CMessageBoxMonitor::Clear();

			Assert::AreEqual(0, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::IsTrue(TestHelpers::CMessageBoxMonitor::GetFirstText().IsEmpty());
			Assert::IsTrue(TestHelpers::CMessageBoxMonitor::GetFirstCaption().IsEmpty());
			TestHelpers::CMessageBoxMonitor::VerifyNone();
		}

		TEST_METHOD(Begin_AfterAMessageBox_ForgetsItAndKeepsWatching)
		{
			ShowMessageBox(_T("earlier"), _T("caption"));

			TestHelpers::CMessageBoxMonitor::Begin();

			Assert::AreEqual(0, TestHelpers::CMessageBoxMonitor::GetCount());
			ShowMessageBox(_T("later"), _T("caption"));
			Assert::AreEqual(1, TestHelpers::CMessageBoxMonitor::GetCount());
			Assert::AreEqual(CString(_T("later")), TestHelpers::CMessageBoxMonitor::GetFirstText());
			TestHelpers::CMessageBoxMonitor::Clear();
		}

		TEST_METHOD(VerifyNone_AfterAMessageBox_FailsOnceAndForgetsIt)
		{
			ShowMessageBox(_T("unexpected"), _T("caption"));

			Assert::IsTrue(ReportsFailure(&TestHelpers::CMessageBoxMonitor::VerifyNone), L"VerifyNone accepted a message box");
			Assert::AreEqual(0, TestHelpers::CMessageBoxMonitor::GetCount());
		}
	};

#ifdef _DEBUG
	// The assertions are compiled out of Release builds, where the monitor has nothing to observe.
	TEST_CLASS(CAssertionMonitorTests)
	{
	public:
		GUARD_AGAINST_UNEXPECTED_POPUPS()

		TEST_METHOD(Reset_NothingFired_HasNothingRecorded)
		{
			Assert::AreEqual(0, TestHelpers::CAssertionMonitor::GetCount());
			Assert::IsTrue(TestHelpers::CAssertionMonitor::GetFirstMessage().IsEmpty());
			TestHelpers::CAssertionMonitor::VerifyNone();
		}

		TEST_METHOD(MfcAssert_Failing_IsRecordedInsteadOfShowingADialog)
		{
			ASSERT(FALSE);

			Assert::AreEqual(1, TestHelpers::CAssertionMonitor::GetCount());
			Assert::IsFalse(TestHelpers::CAssertionMonitor::GetFirstMessage().IsEmpty());
			TestHelpers::CAssertionMonitor::Reset();
		}

		TEST_METHOD(AtlAssert_Failing_IsRecordedInsteadOfShowingADialog)
		{
			ATLASSERT(!"the monitor must see this");

			Assert::AreEqual(1, TestHelpers::CAssertionMonitor::GetCount());
			Assert::IsTrue(TestHelpers::CAssertionMonitor::GetFirstMessage().Find(_T("the monitor must see this")) >= 0);
			TestHelpers::CAssertionMonitor::Reset();
		}

		TEST_METHOD(Reset_AfterAnAssertion_ForgetsItAndVerifyNoneAccepts)
		{
			ASSERT(FALSE);
			ASSERT(FALSE);
			Assert::AreEqual(2, TestHelpers::CAssertionMonitor::GetCount());

			TestHelpers::CAssertionMonitor::Reset();

			Assert::AreEqual(0, TestHelpers::CAssertionMonitor::GetCount());
			Assert::IsTrue(TestHelpers::CAssertionMonitor::GetFirstMessage().IsEmpty());
			TestHelpers::CAssertionMonitor::VerifyNone();
		}

		TEST_METHOD(VerifyNone_AfterAnAssertion_FailsOnceAndForgetsIt)
		{
			ASSERT(FALSE);

			Assert::IsTrue(ReportsFailure(&TestHelpers::CAssertionMonitor::VerifyNone), L"VerifyNone accepted an assertion");
			Assert::AreEqual(0, TestHelpers::CAssertionMonitor::GetCount());
		}
	};
#endif
}
