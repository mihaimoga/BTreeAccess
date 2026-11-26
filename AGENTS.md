# Repository Guidelines

## Project Overview

This repository contains **BTreeAccess**, a native Windows desktop application built with C++ and the Microsoft Foundation Class (MFC) library. The project file is `SiatelHomework.vcxproj`; despite its historical name, it is the authoritative build definition.

## Build and Validation

- Build on Windows with Visual Studio and the MSVC v145 toolset.
- The project supports `Debug` and `Release` configurations for `Win32` and `x64`.
- Use the Visual Studio project build, or run:
  `msbuild SiatelHomework.vcxproj /p:Configuration=Debug /p:Platform=x64`
- Build the affected configuration after changes. The project enables code analysis and uses warning level 4 in Debug builds.
- Unit tests are in the `UnitTests` project (`UnitTests\UnitTests.vcxproj`, part of `SiatelHomework.sln`), which uses the Microsoft Native Unit Test Framework. Run them from Test Explorer, or build and run:
  ```
  msbuild UnitTests\UnitTests.vcxproj /p:Configuration=Debug /p:Platform=x64
  vstest.console.exe UnitTests\x64\Debug\UnitTests.dll /Platform:x64
  ```
  For `Win32`, the DLL is in `UnitTests\Win32\<Configuration>\` and runs with `/Platform:x86`.
- Run the unit tests after changing `SiatelHomeworkExt.*` or `VersionInfo.*`, and follow the "Unit Tests" section below when editing them. They do not cover the MFC dialogs or `CHLinkCtrl`, so validate changes to those by running the corresponding dialog or workflow.

## Source Layout

- `SiatelHomework.cpp` and `SiatelHomework.h`: MFC application entry point and application class.
- `SiatelHomeworkDlg.cpp` and `SiatelHomeworkDlg.h`: main dialog and UI behavior.
- `SiatelHomeworkExt.cpp` and `SiatelHomeworkExt.h`: B-tree/hash-based network and file-management logic.
- `*Dlg.cpp` and `*Dlg.h`: supporting MFC dialogs.
- `HLinkCtrl.*` and `VersionInfo.*`: reusable UI and version-information helpers.
- `Resource.h`, `SiatelHomework.rc`, and `res/`: Windows resource identifiers and assets.
- `stdafx.h` and `stdafx.cpp`: precompiled-header configuration.
- `UnitTests/`: native unit-test project. The application is an EXE, so the test DLL compiles `SiatelHomeworkExt.cpp`, `VersionInfo.cpp` and `stdafx.cpp` directly. It has one `C<ClassUnderTest>Tests.cpp` file per class, `CNetworkWorkflowTests.cpp` for the upload, download, delete and restart scenarios, `TestHelpers.*` for the shared fixtures and the assertion and message-box guards, and `TestHelperTests.cpp` for the tests of those guards.

## C++ and MFC Conventions

- Keep `#include "stdafx.h"` as the first include in `.cpp` files that participate in the precompiled-header build.
- Preserve the existing MFC idioms: message maps, `CString`, `CDialog`, `CFile`, `TRACE`, and `ASSERT`/`VERIFY` where appropriate.
- Match the surrounding formatting: tabs for indentation, braces on their own lines, and existing naming conventions such as `m_` for members, `str` for strings, and `n`/`dw` prefixes for numeric values.
- Prefer modern C++ features already supported by the project (`stdcpplatest`) when they do not conflict with MFC or existing interfaces.
- Keep public declarations in matching `.h` files and implementations in `.cpp` files. Update `SiatelHomework.vcxproj` and `SiatelHomework.vcxproj.filters` when adding source, header, or resource files.

## Unit Tests

- Keep the tests in step with the code: update the matching `C<ClassUnderTest>Tests.cpp` when a class changes. Add every new test file, and every production source file that the tests need, to `UnitTests\UnitTests.vcxproj` and `UnitTests\UnitTests.vcxproj.filters`.
- Name a test `Method_Condition_ExpectedResult`, for example `Delete_FirstOfThree_MovesTheLastFileIntoItsSlot`.
- Create files only inside a `TestHelpers::CTempFolder`, which makes a unique folder under `%TEMP%` and removes it afterwards. Call `srand` first when a test depends on generated codes.
- Do not create the MFC dialogs in a test. Replay the calls that a handler makes, as `TestHelpers::StoreFile` and `CNetworkSession` (in `CNetworkWorkflowTests.cpp`) do.
- Put `GUARD_AGAINST_UNEXPECTED_POPUPS()` in every test class. A message box shown by the tested code is closed at once and fails the test, unless the test reads it through `CMessageBoxMonitor` and then calls `CMessageBoxMonitor::Clear()`. In Debug builds a failed MFC or ATL assertion fails the test too. The macro defines the `TEST_METHOD_INITIALIZE` and `TEST_METHOD_CLEANUP` of the class, so create fixtures inside the tests.
- The tests assume `BALANCED_TREE_ORDER` is 4. A `static_assert` in `CNetworkTreeTests.cpp` stops the build of the tests when it changes; update the expected paths then.
- The `CAssertionMonitor` tests exist only in Debug builds, so a Release run lists fewer tests.

## Resources and Licensing

- Modify dialog layouts and controls through the Visual Studio resource editor when possible. Keep `Resource.h` identifiers synchronized with `SiatelHomework.rc`.
- Preserve the existing GPLv3-or-later copyright and license header in source files. Add the same header to newly created source files unless the file type or existing repository conventions make it inappropriate.
- Avoid unrelated formatting or generated-file changes.
