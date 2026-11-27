# BTreeAccess

**BTreeAccess** is a Windows desktop application, written in C++ with the Microsoft Foundation Class (MFC) library, that works as a small file store addressed by short codes: you upload a file, get a four-character code back, and later use that code to download or delete the file. It addresses the following problem: *create a file server with optimal access to files, with the restriction that each folder can store at most N files.*

The B-tree theory and a walk through the code are in the article [Introduction to B-trees: Concepts and Applications](https://www.codeproject.com/articles/Introduction-to-B-trees-Concepts-and-Applications). A copy is kept in this repository as [BTreeAccess.html](BTreeAccess.html).

![BTreeAccess main window](BTreeAccess.png)

> **Note:** the window title, the executable and the Visual Studio project keep the historical name *SIATEL Homework* (`SiatelHomework.exe`, `SiatelHomework.vcxproj`).

## Features

- Upload any file and receive a random four-character code (digits `0-9` and letters `A-Z`).
- Download or delete a stored file by entering its code.
- A **File List** window shows the number, code and path of every stored file.
- No folder holds more than *N* files or *N* subfolders.
- The index of the stored files is saved as `filelist.txt` in the root folder, so a store can be reopened later.
- A progress bar tracks each file copy.

## How It Works

- **Codes.** Each upload draws a random key that is not in use yet. The code is that key written in base 36 and padded to four characters; for example, the key 65535 is the code `1EKF`.
- **Index.** A hash table (`CNetworkHash`) maps every key to the slot and the relative path of its file. The table is loaded from `filelist.txt`, in the root folder, when you select the folder and is saved back when the application closes. Each line of the file holds `<key> <slot> <relative path>`.
- **Folder layout.** The slot of a file is the number of files already stored when it is uploaded. The slot is written in base *N* and each digit becomes one part of the path: the last digit is the file name (with the `.dat` extension) and every other digit is a folder name, each encoded like a code (four characters). *N* is the `BALANCED_TREE_ORDER` macro in `SiatelHomeworkExt.h`; it is currently 4 (a comment in the source says it should be 1000). With *N* = 4, slots 0-3 are `0000.dat` to `0003.dat` in the root folder, slot 4 is `0001\0000.dat`, slot 5 is `0001\0001.dat`, slot 16 is `0001\0000\0000.dat` and slot 20 is `0001\0001\0000.dat`, so no folder holds more than *N* files or *N* subfolders.
- **Deleting.** The file in the last slot is moved into the slot that was freed, which keeps the slots contiguous.
- **B-tree status.** `USE_HASH_METHOD` in `SiatelHomeworkExt.h` selects the index: `TRUE`, the default, uses the hash table. `FALSE` selects the B-tree class `CNetworkTree`, which is still unfinished: it implements only *B-Tree-Create* and *B-Tree-Search*, and the upload, download, delete and file list branches that would use it are empty.

## Getting Started

### Prerequisites

To build this project, you will need:

- Windows and Visual Studio 2026 (the project uses the MSVC v145 toolset)
- The **Desktop development with C++** workload, including the **C++ MFC for x64/x86 (Latest MSVC)** component
- A Windows 10 or Windows 11 SDK

### Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/mihaimoga/BTreeAccess.git
   ```
2. Open `SiatelHomework.sln` in Visual Studio and select a configuration (`Debug` or `Release`) and a platform (`x64` or `Win32`).
3. Build the solution, or run `msbuild` from a **Developer Command Prompt for VS**:
   ```
   msbuild SiatelHomework.vcxproj /p:Configuration=Debug /p:Platform=x64
   ```

`SiatelHomework.exe` is created in `x64\Debug\` or `x64\Release\` for x64, and in `Debug\` or `Release\` for Win32. MFC is linked statically. Debug builds use warning level 4, and code analysis runs on every build. The solution also contains the `UnitTests` project, which is built before the application and runs its tests after every build, so a failing test fails the solution build and the application is not built (see [Running the Unit Tests](#running-the-unit-tests)); the `msbuild` command above builds only the application, without the tests.

### Running

Start `SiatelHomework.exe` (or press F5 in Visual Studio), then:

1. Click **Browse** at the top of the window and choose the root folder of the store.
2. To upload, choose a file with the **Browse** button of the *Upload* group and click **Upload**. The code of the stored file appears in the same group.
3. To download, choose the destination file with the **Browse** button of the *Download* group, type the code and click **Download**.
4. To delete, type the code in the *Delete* group and click **Delete**.
5. Click **File List** to see all the stored files.

The unit tests do not cover the dialogs, so validate changes to the user interface by running the application.

### Running the Unit Tests

The `UnitTests` project is part of `SiatelHomework.sln`. It uses the Microsoft Native Unit Test Framework and has one test file for each class under test, plus one for the workflows of the main dialog:

- `CNetworkBaseTests.cpp`: the codes (`EncodeNetworkID`, `DecodeNetworkID`, `IsValidCode`), `GetNextID`, the path of a slot (`GetFilePath`) and `CreateNetworkPath`.
- `CNetworkHashTests.cpp`: the index - `InsertItem`, `SearchItem`, `UpdateItem`, `DeleteItem` (including the move of the last file into the freed slot), `GenerateID`, and the import and export of `filelist.txt`.
- `CNetworkTreeTests.cpp`: `CNetworkTree` - `CreateTree`, `SearchNode`, `DeleteTree`, `GenerateID` and the placeholder `ImportData` and `ExportData`.
- `CVersionInfoTests.cpp`: `CVersionInfo`, which reads the version resource that the **About** dialog displays.
- `CNetworkWorkflowTests.cpp`: the Browse, Upload, Download and Delete steps of the main dialog, and closing and restarting the application. The dialog cannot be created in a test, so these tests make the calls that its handlers make, in the same order and on real folders, and check after every step that the index and the disk agree.
- `TestHelpers.*` and `TestHelperTests.cpp`: the shared fixtures, the guards described below, and the tests of those guards.

The dialogs themselves and the `CHLinkCtrl` control are not covered.

The tests run during every build of the project. A post-build event, defined for every configuration of `UnitTests.vcxproj`, runs `vstest.console.exe` from the Visual Studio installation (`$(VSInstallDir)Common7\IDE\Extensions\TestPlatform\`) on the `UnitTests.dll` that was just built, even when nothing was recompiled. The results appear in the build output, and a failing test fails the build. The solution builds `UnitTests` before the application, so a solution build runs the tests once and does not build the application when one of them fails.

You can also run the tests in Visual Studio: open **Test Explorer** and click **Run All Tests**. From a **Developer Command Prompt for VS**, build the tests, which runs them:

```
msbuild UnitTests\UnitTests.vcxproj /p:Configuration=Debug /p:Platform=x64
```

To run the tests that are already built again, without building, call `vstest.console.exe`:

```
vstest.console.exe UnitTests\x64\Debug\UnitTests.dll /Platform:x64
```

For another configuration, change the `Configuration` and `Platform` values and the path of the DLL; the `Win32` tests are in `UnitTests\Win32\<Configuration>\UnitTests.dll` and run with `/Platform:x86`.

The tests create their files in temporary folders and remove them when they finish. A message box shown by the code under test is closed at once, so an unattended run, such as the build, cannot hang, and it fails the running test unless the test expects it. In Debug builds a failed MFC or ATL assertion in the code under test fails the running test too; the tests of that guard exist only in Debug builds, so a Release run lists fewer tests.

The expected paths in the tests are written for `BALANCED_TREE_ORDER` = 4. A `static_assert` in `CNetworkTreeTests.cpp` stops the build of the tests when the value is changed, as a reminder to update them.

## File Structure

- `SiatelHomework.vcxproj`, `SiatelHomework.sln`: Visual Studio project (the authoritative build definition) and solution
- `SiatelHomework.h` / `.cpp`: MFC application class (`CSiatelHomeworkApp`)
- `SiatelHomeworkDlg.h` / `.cpp`: main dialog (browse, upload, download, delete) and the About dialog
- `SiatelFileListDlg.h` / `.cpp`: File List dialog
- `SiatelHomeworkExt.h` / `.cpp`: storage logic - `CNetworkBase` (codes and folder layout), `CNetworkHash` (hash-table index) and `CNetworkTree` (B-tree node, work in progress)
- `FolderDlg.*`, `FilteredFolderDlg.*`: folder picker by Armen Hakobyan
- `HLinkCtrl.*`, `VersionInfo.*`: hyperlink control and version information classes by PJ Naughter
- `resource.h`, `SiatelHomework.rc`, `res/`: resource identifiers, dialogs and icons
- `stdafx.h` / `.cpp`, `targetver.h`: precompiled header and target Windows platform
- `UnitTests/`: unit-test project (`UnitTests.vcxproj`) for the storage logic and the version-information classes; its post-build event runs the tests
- `BTreeAccess.html`: the article on B-trees and on this implementation
- `ReleaseNotes.html`: version history
- `SoftwareContentRegister.html`: third-party components and their licenses
- `ReadMe.txt`: overview generated by the Visual Studio MFC wizard

## Roadmap

The work below is still open. It follows from the unfinished parts of the code, and no release dates are set.

- **Finish the B-tree index.** `CNetworkTree` can only create a tree and search it: `ImportData` and `ExportData` are empty placeholders, no operation deletes a single key, and `InsertNode`, `UpdateNode` and `InsertEmpty` exist only in a comment in `SiatelHomeworkExt.h` ("methods for future use"). Once the missing operations are written, the empty `USE B-TREE` branches (upload, download, delete and file list) can be filled in and `USE_HASH_METHOD` can be set to `FALSE`.
- **Use the intended folder size.** Raise `BALANCED_TREE_ORDER` from 4 to 1000, as the comment in `SiatelHomeworkExt.h` asks. The path of a file depends on this value, so stores created with the old one would have to be migrated; otherwise a new upload could end up on the path of an older file.
- **Make key generation robust.** `GetNextID` relies on `rand()`, which returns 15-bit values, so only 32,768 different keys can ever be drawn, although the hash tables are sized for `MAX_RANDOM_DATA` (65,537) entries. `GenerateID` keeps drawing until it finds a free key, so it would never return once all of them are used. A wider random source and a "store is full" message would fix both.
- **Harden the file index.** Writing the tests showed some weak spots. The index is saved only when the application closes, so choosing another folder with **Browse** drops the changes made in the previous one. When `DeleteItem` cannot move the last file into the freed slot, it shows the system error but still removes the entry, so the index and the slots no longer match, and an entry whose file is missing on disk cannot be deleted from the dialog. In Debug builds, `ImportData` fails an `ASSERT` on a line that is not made of a key, a slot and a path.
- **Extend the automated tests.** `UnitTests` covers `CNetworkBase`, `CNetworkHash`, `CNetworkTree` and `CVersionInfo`. The dialogs and the `CHLinkCtrl` control are not covered yet, and the B-tree operations that are still to be written will need tests of their own. The tests assume `BALANCED_TREE_ORDER` is 4, so their expected paths must be updated when it is raised to 1000; a `static_assert` in `CNetworkTreeTests.cpp` fails the build of the tests as a reminder.
- **Clarify what `nLevel` means.** In `CNetworkHash` it is the slot of a file, not a depth in a tree, but the names and some comments still say "level" and "tree depth". Renaming it (for example to `nSlot`) and fixing the comments would match what the code does.

## Contributing

Bug reports and pull requests are welcome; please read [CONTRIBUTING.md](CONTRIBUTING.md) first. This project follows the [Code of Conduct](CODE_OF_CONDUCT.md). Please report security vulnerabilities privately, as described in [SECURITY.md](SECURITY.md). The repository conventions (formatting, build, unit tests and validation) are summarized in [AGENTS.md](AGENTS.md).

## License

BTreeAccess is licensed under the GNU General Public License, version 3 or (at your option) any later version - see the [LICENSE](LICENSE) file for details. The third-party components used by the project have their own licenses, listed in [SoftwareContentRegister.html](SoftwareContentRegister.html).

Created by [Stefan-Mihai MOGA](https://www.moga.doctor/).
