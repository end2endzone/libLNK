#include "TestLNK.h"
#include "gtesthelper.h"

#include "libLNK.h"
#include "filesystemfunc.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif
#include <Windows.h>

gTestHelper & hlp = gTestHelper::getInstance();

std::string getTestLink()
{
  std::string file;
  file.append("./tests/");
  file.append(hlp.getTestCaseName());
  file.append(".lnk");  
  return file;
}

bool findAndCloseWindow(const char * iWindowTitle)
{
  //Detect the specified window (5 sec timeout)
  HWND hWnd = NULL;
  for(int i=0; i<100 && hWnd==NULL; i++)
  {
    Sleep(50);
    hWnd = FindWindow(NULL, iWindowTitle);
  }
  if (hWnd == NULL)
    return false; //window not found

  //Close the window
  SendMessage(hWnd, WM_CLOSE, 0, 0); //note: blocking call. use PostMessage for non-blocking call.

  bool success = false;
  for(int i=0; i<100 && success==false; i++)
  {
    Sleep(50);

    //find the window again, if not found, then close was successful
    hWnd = FindWindow(NULL, iWindowTitle);
    success = (hWnd == NULL);
  }

  return success;
}

void TestLNK::SetUp()
{

}

void TestLNK::TearDown()
{
}

TEST_F(TestLNK, testCreateSimpleLink)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  //test creation identical
  lnk::LinkInfo info;
  info.target = "C:\\Program Files\\7-Zip\\History.txt";
  info.arguments = "\"this is the arguments\"";
  info.description = "this is my comment";
  info.workingDirectory = "C:\\Program Files\\7-Zip";
  info.customIcon.filename = "%SystemRoot%\\system32\\SHELL32.dll";
  info.customIcon.index = 5;
  info.hotKey = lnk::LNK_NO_HOTKEY;

  bool success = createLink(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );

  //run the link
  std::string openCommandLine = "start \"\" ";
  openCommandLine += lnkFilePath;
  system(openCommandLine.c_str());
  ASSERT_TRUE( findAndCloseWindow("History.txt - Notepad") );

  //test command
  std::string command = lnk::getLinkCommand(lnkFilePath.c_str());
  std::string expectedCommand = info.target;
  expectedCommand += " ";
  expectedCommand += info.arguments;
  ASSERT_TRUE( command == expectedCommand );

  //test getLinkInfo on a custom (handmade) link
  {
    lnk::LinkInfo info;
    bool success = false;
    success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
    ASSERT_TRUE( success == true );
    ASSERT_TRUE( info.target == "C:\\Program Files\\7-Zip\\History.txt" );
    ASSERT_TRUE( info.networkPath == "" );
    ASSERT_TRUE( info.arguments == "\"this is the arguments\"" );
    ASSERT_TRUE( info.description == "this is my comment" );
    ASSERT_TRUE( info.workingDirectory == "C:\\Program Files\\7-Zip" );
    ASSERT_TRUE( info.customIcon.filename == "%SystemRoot%\\system32\\SHELL32.dll" );
    ASSERT_TRUE( info.customIcon.index == 5 );
  }
}

TEST_F(TestLNK, testCreateCustomLink)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  //test creation
  lnk::LinkInfo info;
  info.target = "C:\\WINDOWS\\system32\\cmd.exe";
  info.arguments = "/c pause|echo this is a pause. please press a key";
  info.description = "testCreateCustomLink()";
  info.workingDirectory = filesystem::getCurrentFolder();
  info.workingDirectory += "\\tests";
  info.customIcon.filename = "C:\\Program Files (x86)\\PDFCreator\\PDFCreator.exe";
  info.customIcon.index = 0;
  info.hotKey = lnk::LNK_NO_HOTKEY;

  bool success = lnk::createLink(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );

  //test command
  std::string command = lnk::getLinkCommand(lnkFilePath.c_str());
  std::string expectedCommand = info.target;
  expectedCommand += " ";
  expectedCommand += info.arguments;
  ASSERT_TRUE( command == expectedCommand );

  //test getLinkInfo on a custom (handmade) link
  {
    lnk::LinkInfo info;
    bool success = false;
    success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
    ASSERT_TRUE( success == true );
    ASSERT_TRUE( info.target == "C:\\WINDOWS\\system32\\cmd.exe" );
    ASSERT_TRUE( info.networkPath == "" );
    ASSERT_TRUE( info.arguments == "/c pause|echo this is a pause. please press a key" );
    ASSERT_TRUE( info.description == "testCreateCustomLink()" );
    ASSERT_TRUE( info.workingDirectory == info.workingDirectory.c_str() );
    ASSERT_TRUE( info.customIcon.filename == "C:\\Program Files (x86)\\PDFCreator\\PDFCreator.exe" );
    ASSERT_TRUE( info.customIcon.index == 0 );
  }
}

TEST_F(TestLNK, testWinXPSystemIni)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "C:\\WINDOWS\\system.ini" );
  ASSERT_TRUE( info.networkPath == "" );
  ASSERT_TRUE( info.arguments == "" );
  ASSERT_TRUE( info.description == "" );
  ASSERT_TRUE( info.workingDirectory == "C:\\WINDOWS" );
  ASSERT_TRUE( info.customIcon.filename == "" );
  ASSERT_TRUE( info.customIcon.index == 0 );
}

TEST_F(TestLNK, testWinXpLongFilename)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "M:\\thisisanextralongfoldername\\thisisasuperhugelongfilename\\thisisasuperlongfilename.txt" );
  ASSERT_TRUE( info.networkPath == "\\\\d49ads02\\users$\\BEAUCHAMP.A3\\thisisanextralongfoldername\\thisisasuperhugelongfilename\\thisisasuperlongfilename.txt" );
  ASSERT_TRUE( info.arguments == "" );
  ASSERT_TRUE( info.description == "" );
  ASSERT_TRUE( info.workingDirectory == "M:\\thisisanextralongfoldername\\thisisasuperhugelongfilename" );
  ASSERT_TRUE( info.customIcon.filename == "" );
  ASSERT_TRUE( info.customIcon.index == 0 );
}

TEST_F(TestLNK, testWinXpUsbDrive)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\usbdrive.txt" );
  ASSERT_TRUE( info.networkPath == "" );
  ASSERT_TRUE( info.arguments == "" );
  ASSERT_TRUE( info.description == "" );
  ASSERT_TRUE( info.workingDirectory == "G:\\" );
  ASSERT_TRUE( info.customIcon.filename == "" );
  ASSERT_TRUE( info.customIcon.index == 0 );
}

TEST_F(TestLNK, testWinXpComment)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target != "" );
  ASSERT_TRUE( info.description == "this is my comment" );
}

TEST_F(TestLNK, testWinXpArguments)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target != "" );
  ASSERT_TRUE( info.arguments == "\"this is the arguments\"" );
}

TEST_F(TestLNK, testWinXpIcon)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = false;
  success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target != "" );
  ASSERT_TRUE( info.customIcon.filename == "%SystemRoot%\\system32\\SHELL32.dll" );
  ASSERT_TRUE( info.customIcon.index == 219 );
}
