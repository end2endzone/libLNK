@echo off

echo =======================================================================
echo Finding root folder of repository
echo =======================================================================
cd /d %~dp0
cd ..\..
set REPOSITORY_ROOT=%cd%
echo REPOSITORY_ROOT=%REPOSITORY_ROOT%
echo done.
echo.

echo Change directory to output directory
cd /d %REPOSITORY_ROOT%\cmake\build\bin\Release

echo =======================================================================
echo Running unit tests...
echo =======================================================================
libLNK_unittest.exe
echo done
echo.

echo =======================================================================
echo Uploading test results to AppVeyor
echo =======================================================================
set TEST_RESULT_URL=https://ci.appveyor.com/api/testresults/junit/%APPVEYOR_JOB_ID%
set TEST_RESULT_FILE=%CD%\libLNK_unittest.x86.release.xml
echo TEST_RESULT_URL=%TEST_RESULT_URL%
echo TEST_RESULT_FILE=%TEST_RESULT_FILE%
powershell "(New-Object 'System.Net.WebClient').UploadFile($($env:TEST_RESULT_URL), $($env:TEST_RESULT_FILE))"
