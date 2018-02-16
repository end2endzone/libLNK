![libLNK logo](https://github.com/end2endzone/libLNK/raw/master/docs/libLNK-splashscreen.png)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Github Releases](https://img.shields.io/github/release/end2endzone/libLNK.svg)](https://github.com/end2endzone/libLNK/releases)
[![Build status](https://ci.appveyor.com/api/projects/status/yiohs75vn2h6s4sw/branch/master?svg=true)](https://ci.appveyor.com/project/end2endzone/libLNK/branch/master)
[![Tests status](https://img.shields.io/appveyor/tests/end2endzone/libLNK/master.svg)](https://ci.appveyor.com/project/end2endzone/libLNK/branch/master/tests)

AppVeyor build statistics:

[![Build statistics](https://buildstats.info/appveyor/chart/end2endzone/libLNK)](https://ci.appveyor.com/project/end2endzone/libLNK/branch/master)


# libLNK

libLNK is a c++ library for creating Windows shortcuts (*.lnk files).

It's main features are:

*  Supports shortcuts created by Windows XP and over.
*  Create new shortcuts or read properties of an existing shortcuts.
*  Support the following shotcut properties:
   *  target (path to file or folder)
   *  arguments
   *  working directory (start in)
   *  hot key (shortcut key)
   *  window mode (normal, minimized or maximized)
   *  description (comments)
   *  icon

# Background

libLNK was first released in XXXX. At the time, there were not much documentation of the format that was widely available. 

The following links are references that were available at the time of the first public release:
* [http://www.stdlib.com/art6-Link-File-Format-lnk.html](http://www.stdlib.com/art6-Link-File-Format-lnk.html)
* [http://www.wotsit.org/list.asp?search=lnk](http://www.wotsit.org/list.asp?search=lnk)

libLNK implementation is a reverse engineering process to identify how the file format is working. 

Note that the Shell Link (.LNK) binary file format is now properly documented and can be found on [MSDN](https://msdn.microsoft.com/en-us/library/dd871305.aspx).


# Usage

The library publishes multiple functions in the '*lnk*' namespace. Each function is self explanatory to manipulate the Windows Shortcut binary file format:

```cpp
bool isLink(const unsigned char * iBuffer, const unsigned long & iSize); 
bool getLinkInfo(const char * iFilePath, LinkInfo & oLinkInfo); 
bool createLink(const char * iFilePath, const LinkInfo & iLinkInfo); 
std::string getLinkCommand(const char * iFilePath); 
```
                            
The library also publishes debuging API functions:
```cpp
const char * getVersionString(); 
bool printLinkInfo(const char * iFilePath); 
```

# Example

### Create shortcut:
The following example creates a shortcut to the '*History.txt*' file of [7-Zip](http://www.7-zip.org/download.html). The file is located in '*C:\Program Files\7-Zip\History.txt*'.

```cpp
lnk::LinkInfo info;
info.target = "C:\\Program Files\\7-Zip\\History.txt";
info.arguments = "";
info.description = "manual shortcut created by libLNK library";
info.workingDirectory = "C:\\Program Files\\7-Zip";
info.customIcon.filename = "%SystemRoot%\\system32\\SHELL32.dll";
info.customIcon.index = 101; //empty recycle bin icon
info.hotKey = lnk::LNK_NO_HOTKEY;

const char * linkFilename = "Shortcut to 7-zip History.txt.lnk";
bool success = lnk::createLink(linkFilename, info);
```

### Read shortcut properties
The following example read properties of the '*Calculator*' shortcut of the '*Start Menu*'.

```cpp
lnk::LinkInfo info;
bool success = lnk::getLinkInfo("C:\\Users\\All Users\\Microsoft\\Windows\\Start Menu\\Programs\\Accessories\\Calculator.lnk", info);
printf("target=%s\n", info.target.c_str());
printf("network path=%s\n", info.networkPath.c_str());
printf("arguments=%s\n", info.arguments.c_str());
printf("description=%s\n", info.description.c_str());
printf("workingDirectory=%s\n", info.workingDirectory.c_str());
printf("icon.filename=%s\n", info.customIcon.filename.c_str());
printf("icon.index=%d\n", info.customIcon.index);
```

Outputs:

```
target=%windir%\system32\calc.exe
network path=
arguments=
description=Performs basic arithmetic tasks with an on-screen calculator.
workingDirectory=
icon.filename=%windir%\system32\calc.exe
icon.index=0
```

# !!!ICI!!!
https://github.com/end2endzone/libLNK/blob/master/src/libLNK_unittest/TestLNK.cpp
https://github.com/libyal/liblnk/wiki/Building

### Command:
```batchfile
libLNK.exe --file=html5skeleton.html --output=.\outdir --headerfile=resourcehtml5skeleton.h
            --identifier=HtmlSample --chunksize=50
```

# Installing

Please refer to file [INSTALL.md](INSTALL.md) for details on how installing/building the application.

## Testing
libLNK comes with unit tests which tests for multiple combinations to validate the integrity of the library with multiple operating systems.

Test are build using the Google Test v1.6.0 framework. For more information on how googletest is working, see the [google test documentation primer](https://github.com/google/googletest/blob/release-1.8.0/googletest/docs/V1_6_Primer.md).  

Test are automatically build when building the solution. Please see the '*build step*' section for details on how to build the software.

Test can be executed from the following two locations:

1) From the Visual Studio IDE:
   1) Select the project '*libLNK_unittest*' as StartUp project.
   2) Hit CTRL+F5 (Start Without Debugging)
2) From the output binaries folder:
   1) Open a file navigator and browse to the output folder(for example c:\projects\libLNK\cmake\build\bin\Release)
   2) Run the '*libLNK_unittest.exe*' executable.

See also the latest test results at the beginning of the document.

# Compatible with

libLNK is only available for the Windows platform and has been tested with the following version of Windows:

*   Windows XP
*   Windows 7

# Versioning

We use [Semantic Versioning 2.0.0](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/end2endzone/libLNK/tags).

# Authors

* **Antoine Beauchamp** - *Initial work* - [end2endzone](https://github.com/end2endzone)

See also the list of [contributors](https://github.com/end2endzone/libLNK/blob/master/AUTHORS) who participated in this project.

# License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
