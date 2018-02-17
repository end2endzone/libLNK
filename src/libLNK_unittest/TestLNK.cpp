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

TEST_F(TestLNK, testWin7CdRom)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "E:\\Specials\\IMG_5187_LR5.jpg" );
}

TEST_F(TestLNK, testWin7LongFilename)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "D:\\temp\\foo\\thisisalongfilename.txt" );
}

TEST_F(TestLNK, testWin7LongFolderName)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "D:\\temp\\foo\\thisisalongfolder\\bar\\History.txt" );
}

TEST_F(TestLNK, testWin7MappedDrive)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "Z:\\STANDARD\\9000\\E\\9001E.PDF" );
}

TEST_F(TestLNK, testWin7MultipleFolders)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "D:\\temp\\foo\\777\\bar\\History.txt" );
}

TEST_F(TestLNK, testWin7NetworkPath)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "HOME.PDF" );
  ASSERT_TRUE( info.networkPath == "\\\\FILSRV01\\ISO\\HOME.PDF" );
  ASSERT_TRUE( info.workingDirectory == "\\\\filsrv01\\ISO" );
}

TEST_F(TestLNK, testWin7UsbDrive2)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\foo\\bar\\History.txt" );
}

TEST_F(TestLNK, testWin7UsbDriveReadOnly)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "Z:\\TUG\\Scope.txt" );
}

TEST_F(TestLNK, testWin7UsbDriveRemovableMedium)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "Z:\\TUG\\Scope.txt" );
}

TEST_F(TestLNK, testWin7SpaceFilename)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\Temp\\foo bar.txt" );
}

TEST_F(TestLNK, testWin7SpaceFolder)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\Temp\\foo bar\\History.txt" );
}

TEST_F(TestLNK, testWin7SpecialCharactersEacute)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\Temp\\école.txt" );
}

TEST_F(TestLNK, testWin7SpecialCharactersNtilde)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\Temp\\español.txt" );
}

TEST_F(TestLNK, testWin7SpecialCharactersCcedil)
{
  //Build test case link file
  std::string lnkFilePath = getTestLink();

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "G:\\Temp\\français.txt" );
}

TEST_F(TestLNK, testDocumentationExampleShortcutToFile)
{
  //testing example from MSDN documentation
  //https://winprotocoldoc.blob.core.windows.net/productionwindowsarchives/MS-SHLLINK/[MS-SHLLINK].pdf
  //section 3.1

  //Build test case link file
  std::string lnkFilePath = getTestLink();

  unsigned char content[] = {
    0x4C, 0x00, 0x00, 0x00, 0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x46, 0x9B, 0x00, 0x08, 0x00, 0x20, 0x00, 0x00, 0x00, 0xD0, 0xE9, 0xEE, 0xF2,
    0x15, 0x15, 0xC9, 0x01, 0xD0, 0xE9, 0xEE, 0xF2, 0x15, 0x15, 0xC9, 0x01, 0xD0, 0xE9, 0xEE, 0xF2,
    0x15, 0x15, 0xC9, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBD, 0x00, 0x14, 0x00,
    0x1F, 0x50, 0xE0, 0x4F, 0xD0, 0x20, 0xEA, 0x3A, 0x69, 0x10, 0xA2, 0xD8, 0x08, 0x00, 0x2B, 0x30,
    0x30, 0x9D, 0x19, 0x00, 0x2F, 0x43, 0x3A, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x31, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2C, 0x39, 0x69, 0xA3, 0x10, 0x00, 0x74, 0x65, 0x73, 0x74, 0x00, 0x00, 0x32,
    0x00, 0x07, 0x00, 0x04, 0x00, 0xEF, 0xBE, 0x2C, 0x39, 0x65, 0xA3, 0x2C, 0x39, 0x69, 0xA3, 0x26,
    0x00, 0x00, 0x00, 0x03, 0x1E, 0x00, 0x00, 0x00, 0x00, 0xF5, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74, 0x00, 0x00, 0x00, 0x14,
    0x00, 0x48, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x39, 0x69, 0xA3, 0x20, 0x00, 0x61,
    0x2E, 0x74, 0x78, 0x74, 0x00, 0x34, 0x00, 0x07, 0x00, 0x04, 0x00, 0xEF, 0xBE, 0x2C, 0x39, 0x69,
    0xA3, 0x2C, 0x39, 0x69, 0xA3, 0x26, 0x00, 0x00, 0x00, 0x2D, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x96,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x00, 0x2E, 0x00, 0x74,
    0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x1C,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x81,
    0x8A, 0x7A, 0x30, 0x10, 0x00, 0x00, 0x00, 0x00, 0x43, 0x3A, 0x5C, 0x74, 0x65, 0x73, 0x74, 0x5C,
    0x61, 0x2E, 0x74, 0x78, 0x74, 0x00, 0x00, 0x07, 0x00, 0x2E, 0x00, 0x5C, 0x00, 0x61, 0x00, 0x2E,
    0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x07, 0x00, 0x43, 0x00, 0x3A, 0x00, 0x5C, 0x00, 0x74,
    0x00, 0x65, 0x00, 0x73, 0x00, 0x74, 0x00, 0x60, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0xA0, 0x58,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x68, 0x72, 0x69, 0x73, 0x2D, 0x78, 0x70, 0x73,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x78, 0xC7, 0x94, 0x47, 0xFA, 0xC7, 0x46, 0xB3,
    0x56, 0x5C, 0x2D, 0xC6, 0xB6, 0xD1, 0x15, 0xEC, 0x46, 0xCD, 0x7B, 0x22, 0x7F, 0xDD, 0x11, 0x94,
    0x99, 0x00, 0x13, 0x72, 0x16, 0x87, 0x4A, 0x40, 0x78, 0xC7, 0x94, 0x47, 0xFA, 0xC7, 0x46, 0xB3,
    0x56, 0x5C, 0x2D, 0xC6, 0xB6, 0xD1, 0x15, 0xEC, 0x46, 0xCD, 0x7B, 0x22, 0x7F, 0xDD, 0x11, 0x94,
    0x99, 0x00, 0x13, 0x72, 0x16, 0x87, 0x4A, 0x00, 0x00, 0x00, 0x00
  };
  static const size_t EXAMPLE_ACTUAL_SIZE = sizeof(content)/sizeof(content[0]);
  static const size_t EXAMPLE_EXPECTED_SIZE = 459;

  ASSERT_TRUE(EXAMPLE_ACTUAL_SIZE == EXAMPLE_EXPECTED_SIZE);

  FILE * f = fopen(lnkFilePath.c_str(), "wb");
  ASSERT_TRUE(f != NULL);
  fwrite(content, 1, EXAMPLE_ACTUAL_SIZE, f);
  fclose(f);

  lnk::LinkInfo info;
  bool success = lnk::getLinkInfo(lnkFilePath.c_str(), info);
  ASSERT_TRUE( success == true );
  ASSERT_TRUE( info.target == "C:\\test\\a.txt" );
}
