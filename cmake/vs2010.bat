@echo off
cd /d %~dp0

set PATH=%PATH%;D:\dev\third_parties\3rd_parties\cmake\3.4.3\cmake-3.4.3-win32-x86\bin

::Find src folder
cd..
cd src
set SRC_DIR=%cd%
cd /d %~dp0

REM Create build directory
mkdir build >NUL 2>NUL
cd build

cmake -G "Visual Studio 10 2010" %SRC_DIR%
