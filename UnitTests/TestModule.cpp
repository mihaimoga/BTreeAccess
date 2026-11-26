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

// TestModule.cpp : set-up and tear-down that apply to the whole test DLL
//

#include "stdafx.h"
#include "TestHelpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_MODULE_INITIALIZE(ModuleInitialize)
{
	TestHelpers::CAssertionMonitor::Install();
}

TEST_MODULE_CLEANUP(ModuleCleanup)
{
	TestHelpers::CAssertionMonitor::Uninstall();
}
