# Installing

This section explains how to compile and build the software and how to get a development environment running.

## Compatible with

libLNK is only available for the Windows platform and has been tested with the following version of Windows:

*   Windows XP
*   Windows Vista
*   Windows 7

## Prerequisites

The following software must be installed on the system for compiling source code:

* A C++ compiler (Visual Studio 2010 or newwer is recommended)
* [Google C++ Testing Framework v1.6.0](https://github.com/google/googletest/tree/release-1.6.0) (untested with other versions)
* [CMake](http://www.cmake.org/) 3.4.3 (or newer)
* [libLNK source code](https://github.com/end2endzone/libLNK/tags)

The following software must be installed on the system for building the deploy packages:

* [7-Zip](http://www.7-zip.org/) for building the win32 portable package. Tested with version 9.20.
* [NSIS (Nullsoft Scriptable Install System)](http://nsis.sourceforge.net/) for building Windows Self-Extracting Setup Installer (setup.exe). Tested with version 3.0a1.

## Build steps

### Google C++ testing framework

*Note:* libLNK library is expecting to find googletest as a project's third party. To be properly detected by libLNK, all third party libraries must be installed in the ***third_party*** folder of the source code.

1) Download googletest source code to your computer using one of the following:
   1) Download googletest as a [zip file](https://github.com/google/googletest/archive/release-1.6.0.zip) and extract to a temporary directory (for example c:\projects\libLNK\third_party\googletest).
   
   2) Clone the git repository using the following commands:
      * git clone https://github.com/google/googletest.git c:\projects\libLNK\third_party\googletest
      * cd /d c:\projects\libLNK\third_party\googletest
      * git checkout release-1.6.0

2) Generate googletest Visual Studio 2010 solution using cmake. Enter the following commands:
   * cd c:\projects\libLNK\third_party\googletest
   * mkdir msvc2010
   * cd msvc2010
   * cmake -G "Visual Studio 10 2010" -Dgtest_force_shared_crt=ON -DCMAKE_CXX_FLAGS_DEBUG=/MDd -DCMAKE_CXX_FLAGS_RELEASE=/MD "c:\projects\libLNK\third_party\googletest"

3) Open the generated Visual Studio 2010 solution file located in 
   ***c:\projects\libLNK\third_party\googletest\msvc2010\gtest.sln***

4) Build the solution.

### libLNK

1) Download the [libLNK source code](https://github.com/end2endzone/libLNK/tags) and extract the content to a temporary directory (for example c:\projects\libLNK).

2) Navigate to the ***cmake*** directory.

3) Run the batch script which matches your desired compiler (ie: vs2010.bat). This will create the ***build*** directory inside current directory.

2) Open the Visual Studio 2010 solution file located in 
   ***c:\projects\libLNK\cmake\build\libLNK.sln***

3) Build the solution.

