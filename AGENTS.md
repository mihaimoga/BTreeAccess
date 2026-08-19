# Repository Guidelines

## Project Overview

This repository contains **BTreeAccess**, a native Windows desktop application built with C++ and the Microsoft Foundation Class (MFC) library. The project file is `SiatelHomework.vcxproj`; despite its historical name, it is the authoritative build definition.

## Build and Validation

- Build on Windows with Visual Studio and the MSVC v145 toolset.
- The project supports `Debug` and `Release` configurations for `Win32` and `x64`.
- Use the Visual Studio project build, or run:
  `msbuild SiatelHomework.vcxproj /p:Configuration=Debug /p:Platform=x64`
- Build the affected configuration after changes. The project enables code analysis and uses warning level 4 in Debug builds.
- No automated test project is currently present. Validate behavioral changes by running the corresponding MFC dialog or workflow.

## Source Layout

- `SiatelHomework.cpp` and `SiatelHomework.h`: MFC application entry point and application class.
- `SiatelHomeworkDlg.cpp` and `SiatelHomeworkDlg.h`: main dialog and UI behavior.
- `SiatelHomeworkExt.cpp` and `SiatelHomeworkExt.h`: B-tree/hash-based network and file-management logic.
- `*Dlg.cpp` and `*Dlg.h`: supporting MFC dialogs.
- `HLinkCtrl.*` and `VersionInfo.*`: reusable UI and version-information helpers.
- `Resource.h`, `SiatelHomework.rc`, and `res/`: Windows resource identifiers and assets.
- `stdafx.h` and `stdafx.cpp`: precompiled-header configuration.

## C++ and MFC Conventions

- Keep `#include "stdafx.h"` as the first include in `.cpp` files that participate in the precompiled-header build.
- Preserve the existing MFC idioms: message maps, `CString`, `CDialog`, `CFile`, `TRACE`, and `ASSERT`/`VERIFY` where appropriate.
- Match the surrounding formatting: tabs for indentation, braces on their own lines, and existing naming conventions such as `m_` for members, `str` for strings, and `n`/`dw` prefixes for numeric values.
- Prefer modern C++ features already supported by the project (`stdcpplatest`) when they do not conflict with MFC or existing interfaces.
- Keep public declarations in matching `.h` files and implementations in `.cpp` files. Update `SiatelHomework.vcxproj` and `SiatelHomework.vcxproj.filters` when adding source, header, or resource files.

## Resources and Licensing

- Modify dialog layouts and controls through the Visual Studio resource editor when possible. Keep `Resource.h` identifiers synchronized with `SiatelHomework.rc`.
- Preserve the existing GPLv3-or-later copyright and license header in source files. Add the same header to newly created source files unless the file type or existing repository conventions make it inappropriate.
- Avoid unrelated formatting or generated-file changes.
