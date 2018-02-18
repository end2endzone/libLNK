#include "libLNK.h"
#include <assert.h>
#include <memory.h>
#include <windows.h>
#include <direct.h>
#include "..\..\version_info.h"

#include "filesystemfunc.h"

#include "MemoryBuffer.h"

namespace lnk
{

//----------------------------------------------------------------------------------------------------------------------------------------
// Defines, Pre-declarations & typedefs
//----------------------------------------------------------------------------------------------------------------------------------------
typedef std::vector<std::string> StringList;

//----------------------------------------------------------------------------------------------------------------------------------------
// Structures
//----------------------------------------------------------------------------------------------------------------------------------------
//http://www.stdlib.com/art6-Link-File-Format-lnk.html
//http://www.wotsit.org/list.asp?search=lnk

#pragma pack(push)
#pragma pack(1)

//Link flags
struct LinkFlags
{
  bool HasLinkTargetIDList      :1;
  bool HasLinkInfo              :1;
  bool HasName                  :1;
  bool HasRelativePath          :1;
  bool HasWorkingDir            :1;
  bool HasArguments             :1;
  bool HasIconLocation          :1;
  bool reserved1                :1;
  bool reserved2                :8;
  bool reserved3                :8;
  bool reserved4                :8;
};

//Target flags
struct FileAttributesFlags
{
  bool isReadOnly                     :1; //FILE_ATTRIBUTE_READONLY
  bool isHidden                       :1; //FILE_ATTRIBUTE_HIDDEN
  bool isSystemFile                   :1; //FILE_ATTRIBUTE_SYSTEM
  bool isVolumeLabel                  :1; //Reserved1, MUST be zero.
  bool isDirectory                    :1; //FILE_ATTRIBUTE_DIRECTORY
  bool hasBeenModifiedSinceLastBackup :1; //FILE_ATTRIBUTE_ARCHIVE
  bool isEncrypted                    :1; //Reserved2, MUST be zero.
  bool isNormal                       :1; //FILE_ATTRIBUTE_NORMAL
  bool isTemporary                    :1; //FILE_ATTRIBUTE_TEMPORARY
  bool isSparseFile                   :1; //FILE_ATTRIBUTE_SPARSE_FILE
  bool hasReparsePointData            :1; //FILE_ATTRIBUTE_REPARSE_POINT
  bool isCompressed                   :1; //FILE_ATTRIBUTE_COMPRESSED
  bool isOffline                      :1; //FILE_ATTRIBUTE_OFFLINE
  bool reserved1                      :3; //FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
  bool reserved2                      :8; //FILE_ATTRIBUTE_ENCRYPTED
  bool reserved3                      :8;
};

typedef uint8_t LNK_CLSID[16];
static const LNK_CLSID DEFAULT_LINKCLSID = { 0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};
struct ShellLinkHeader
{
  uint32_t HeaderSize;
  LNK_CLSID LinkCLSID;
  LinkFlags linkFlags; 
  FileAttributesFlags FileAttributes;
  unsigned __int64 CreationTime;
  unsigned __int64 AccessTime;
  unsigned __int64 WriteTime;
  unsigned long FileSize;
  unsigned long IconIndex;
  unsigned long ShowCommand;
  LNK_HOTKEY HotKey;
  unsigned long reserved1;
  unsigned long reserved2;
};

static const unsigned long LNK_LOCATION_UNKNOWN = 0;
static const unsigned long LNK_LOCATION_LOCAL = 1;
static const unsigned long LNK_LOCATION_NETWORK = 2;
//File Location Info 
//This section is always present, but if bit 1 is not set in the flags value,
//then the length of this section will be zero.
//The header of this section is described below. 
//1 dword This length value includes all the assorted pathnames and other data structures. All offsets are relative to the start of this section.  
//1 dword The offset at which the basic file info structure ends. Should be 1C. 
//1 dword File available on local volume (0) or network share(1) 
//1 dword Offset to the local volume table. 
//1 dword Offset to the base path on the local volume. 
//1 dword Offset to the network volume table. 
//1 dword Offset to the final part of the pathname. 
struct LNK_FILE_LOCATION_INFO
{
  unsigned long length;
  unsigned long endOffset;
  unsigned long location;
  unsigned long localVolumeTableOffset;
  unsigned long basePathOffset;
  unsigned long networkVolumeTableOffset;
  unsigned long finalPathOffset;
};
static const unsigned long LNK_FILE_LOCATION_INFO_SIZE = sizeof(LNK_FILE_LOCATION_INFO);


//Type of volumes
//Code Description 
//0 Unknown 
//1 No root directory 
//2 Removable (Floppy, Zip ...) 
//3 Fixed (Hard disk) 
//4 Remote (Network drive) 
//5 CD-ROM 
//6 Ram drive 
static const unsigned long LNK_VOLUME_TYPE_UNKNOWN           = 0;
static const unsigned long LNK_VOLUME_TYPE_NO_ROOT_DIRECTORY = 1;
static const unsigned long LNK_VOLUME_TYPE_REMOVABLE         = 2;
static const unsigned long LNK_VOLUME_TYPE_FIXED             = 3;
static const unsigned long LNK_VOLUME_TYPE_REMOTE            = 4;
static const unsigned long LNK_VOLUME_TYPE_CDROM             = 5;
static const unsigned long LNK_VOLUME_TYPE_RAMDRIVE          = 6;

//The local volume table
//1 dword Length of this structure including the volume label string. 
//1 dword Type of volume (code below) 
//1 dword Volume serial number 
//1 dword Offset of the volume name (Always 0x10) 
//ASCIZ Volume label 
struct LNK_LOCAL_VOLUME_TABLE
{
  unsigned long length;
  unsigned long volumeType;
  unsigned long volumeSerialNumber;
  unsigned long volumeNameOffset;
  char volumeLabel;
};
static const unsigned long LNK_LOCAL_VOLUME_TABLE_SIZE = sizeof(LNK_LOCAL_VOLUME_TABLE);


//The network volume table
//1 dword Length of this structure 
//1 dword Always 02 
//1 dword Offset of network share name (Always 0x14) 
//1 dword Reserved 0 
//1 dword Always 0x20000 
//ASCIZ Network share name 
struct LNK_NETWORK_VOLUME_TABLE
{
  unsigned long length;
  unsigned long reserved1;
  unsigned long networkShareNameOffset;
  unsigned long reserved2;
  unsigned long reserved3;
  char networkShareName;
};
static const unsigned long LNK_NETWORK_VOLUME_TABLE_SIZE = sizeof(LNK_NETWORK_VOLUME_TABLE);

//itemId structure used in shellItemIdList part of a link
struct LNK_ITEMID
{
  unsigned short size;
  unsigned char signature[6];       
  unsigned char unknown01;          
  unsigned char unknown02;          
  unsigned char unknown03;          
  unsigned char unknown04;          
  unsigned char unknown05;          
  unsigned char unknown06;          
  const char * name83;
  unsigned char unknown07;          
  unsigned char unknown08;          
  unsigned char unknown09[7];       
  unsigned char unknown10;          
  unsigned char unknown11;          
  unsigned char unknown12;          
  unsigned char unknown13;          
  unsigned char unknown14;          
  unsigned char unknown15;          
  unsigned char unknown16;          
  unsigned char unknown17;          
  unsigned char unknown18[4];       
  const char * nameUnicode;
  unsigned char unknown19;          
  unsigned char unknown20;          
};
extern const LNK_ITEMID LNK_ITEMIDFolderDefault;
extern const LNK_ITEMID LNK_ITEMIDFileDefault;

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------------------------
// global classes & functions
//----------------------------------------------------------------------------------------------------------------------------------------
std::string toTimeString(const unsigned __int64 & iTime)
{
  //time stamps
  const FILETIME * utcFileTime = (const FILETIME *)&iTime;
  FILETIME localFileTime = {0};
  FileTimeToLocalFileTime(utcFileTime, &localFileTime);
  SYSTEMTIME localSystemTime = {0};
  FileTimeToSystemTime(&localFileTime, &localSystemTime);

  char buffer[1024];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
    localSystemTime.wYear,
    localSystemTime.wMonth,
    localSystemTime.wDay,
    localSystemTime.wHour,
    localSystemTime.wMinute,
    localSystemTime.wSecond,
    localSystemTime.wMilliseconds );

  std::string value = buffer;
  return value;
}

static const LNK_HOTKEY LNK_NO_HOTKEY = {LNK_HK_NONE, LNK_HK_MOD_NONE};
static const LNK_ITEMID         LNK_ITEMIDFolderDefault = { 0,
                                                          {0x31, 0x00, 0x00, 0x00, 0x00, 0x00},           //always 0x31 0x00 0x00 0x00 0x00 0x00
                                                          0x3a,                                           //0x3a is a possible value
                                                          0x3e,                                           //always 0x3e
                                                          0x0e,                                           //0x0e is a possible value
                                                          0x6b,                                           //0x6b is a possible value
                                                          0x10,                                           //always 0x10
                                                          0x00,                                           //always 0x00
                                                          NULL,                                           //name in 8.3 format
                                                          0x00,                                           //always 0x00
                                                          0x28,                                           //0x28 is a possible value
                                                          {0x00, 0x03, 0x00, 0x04, 0x00, 0xef, 0xbe},     //always 00 03 00 04 00 ef be
                                                          0x3a,                                           //always unknown1
                                                          0x3e,                                           //always unknown2
                                                          0x0e,                                           //always unknown3
                                                          0x6b,                                           //always unknown4
                                                          0x3a,                                           //always unknown1
                                                          0x3e,                                           //always unknown2
                                                          0x0e,                                           //always unknown3
                                                          0x6b,                                           //always unknown4
                                                          {0x14, 0x00, 0x00, 0x00},                       //always 14 00 00 00
                                                          NULL,                                           //name in unicode format
                                                          0x18,                                           //0x18 is a possible value or 0x14 is a possible value
                                                          0x00                                            //always 0x00
};
static const LNK_ITEMID       LNK_ITEMIDFileDefault = { 0,
                                                      {0x32, 0x00, 0x00, 0x00, 0x00, 0x00},         //always 0x32 0x00 0x00 0x00 0x00 0x00
                                                      0x3a,                                         //0x3a is a possible value
                                                      0x3e,                                         //always 0x3e
                                                      0x13,                                         //0x13 is a possible value
                                                      0x6b,                                         //0x6b is a possible value
                                                      0x20,                                         //always 0x20
                                                      0x00,                                         //always 0x00
                                                      NULL,                                         //name in 8.3 format
                                                      0x00,                                         //always 0x00
                                                      0x2c,                                         //0x2c is a possible value
                                                      {0x00, 0x03, 0x00, 0x04, 0x00, 0xef, 0xbe},   //always 00 03 00 04 00 ef be
                                                      0x3a,                                         //always unknown1
                                                      0x3e,                                         //always unknown2
                                                      0x13,                                         //always unknown3
                                                      0x6b,                                         //always unknown4
                                                      0x3a,                                         //always unknown1
                                                      0x3e,                                         //always unknown2
                                                      0x13,                                         //always unknown3
                                                      0x6b,                                         //always unknown4
                                                      {0x14, 0x00, 0x00, 0x00},                     //always 14 00 00 00
                                                      NULL,                                         //name in unicode format
                                                      0x18,                                         //0x18 is a possible value or 0x14 is a possible value
                                                      0x00                                          //always 0x00
};


template <typename T>
inline const T & readData(const unsigned char * iBuffer, unsigned long & ioOffset)
{
  const T & value = *(   (T*)(&iBuffer[ioOffset])  );
  ioOffset += sizeof(T);
  return value;
}

void readString(const unsigned char * iBuffer, unsigned long & ioOffset, std::string & oValue)
{
  oValue = "";
  const unsigned short & length = readData<unsigned short>(iBuffer, ioOffset);
  for(unsigned short i=0; i<length; i++)
  {
    const unsigned short & tmp = readData<unsigned short>(iBuffer, ioOffset);
    char c = (char)tmp;
    oValue += c;
  }
}

std::string readUnicodeString(FILE * iFile)
{
  std::string value;

  unsigned short length = 0;
  fread(&length, 1, sizeof(length), iFile);
  
  for(unsigned short i=0; i<length; i++)
  {
    unsigned short tmp = 0;
    fread(&tmp, 1, sizeof(tmp), iFile);

    char c = (char)tmp;
    value += c;
  }

  return value;
}

void saveStringUnicode(FILE * iFile, const std::string & iValue)
{
  unsigned short length = (unsigned short)iValue.size();
  fwrite(&length, 1, sizeof(length), iFile);

  for(unsigned short i=0; i<length; i++)
  {
    unsigned short c = iValue[i];
    fwrite(&c, 1, sizeof(c), iFile);
  }
}

void saveString(FILE * iFile, const std::string & iValue)
{
  size_t length = iValue.size();
  const char * text = iValue.c_str();
  fwrite(text, 1, length, iFile);

  static const char NULLCHARACTER = '\0';
  fwrite(&NULLCHARACTER, 1, sizeof(NULLCHARACTER), iFile);
}

unsigned short getSizeOf(const LNK_ITEMID & iValue)
{
  unsigned short size = 0;

  size += sizeof(LNK_ITEMID) - sizeof(iValue.name83) - sizeof(iValue.nameUnicode);
  std::string shortPath = iValue.name83;
  std::string longPath = iValue.nameUnicode;
  size_t name83Length = shortPath.size();
  size += (unsigned short)name83Length + 1;
  size_t nameUnicodeLength = longPath.size();
  size += (unsigned short)(nameUnicodeLength+1)*2;

  return size;
}

template <typename T>
inline bool serialize(const T & iValue, MemoryBuffer & ioBuffer)
{
  unsigned long dataSize = sizeof(T);
  unsigned long oldSize = ioBuffer.getSize();
  unsigned long newSize = oldSize + dataSize;
  if (ioBuffer.reallocate(newSize))
  {
    //save data
    unsigned char * buffer = ioBuffer.getBuffer();
    T * offset = (T*)&buffer[oldSize];
    *offset = iValue;

    return true;
  }
  return false;
}

template <>
inline bool serialize(const LNK_ITEMID & iValue, MemoryBuffer & ioBuffer)
{
  serialize(iValue.size, ioBuffer);
  serialize(iValue.signature[0], ioBuffer);
  serialize(iValue.signature[1], ioBuffer);
  serialize(iValue.signature[2], ioBuffer);
  serialize(iValue.signature[3], ioBuffer);
  serialize(iValue.signature[4], ioBuffer);
  serialize(iValue.signature[5], ioBuffer);
  serialize(iValue.unknown01, ioBuffer);
  serialize(iValue.unknown02, ioBuffer);
  serialize(iValue.unknown03, ioBuffer);
  serialize(iValue.unknown04, ioBuffer);
  serialize(iValue.unknown05, ioBuffer);
  serialize(iValue.unknown06, ioBuffer);

  //name83
  {
    std::string tmp = iValue.name83;
    for(unsigned long i=0; i<=tmp.size(); i++)
    {
      const char & c = iValue.name83[i];
      serialize(c, ioBuffer);
    }
  }

  serialize(iValue.unknown07, ioBuffer);
  serialize(iValue.unknown08, ioBuffer);
  serialize(iValue.unknown09[0], ioBuffer);
  serialize(iValue.unknown09[1], ioBuffer);
  serialize(iValue.unknown09[2], ioBuffer);
  serialize(iValue.unknown09[3], ioBuffer);
  serialize(iValue.unknown09[4], ioBuffer);
  serialize(iValue.unknown09[5], ioBuffer);
  serialize(iValue.unknown09[6], ioBuffer);
  serialize(iValue.unknown10, ioBuffer);
  serialize(iValue.unknown11, ioBuffer);
  serialize(iValue.unknown12, ioBuffer);
  serialize(iValue.unknown13, ioBuffer);
  serialize(iValue.unknown14, ioBuffer);
  serialize(iValue.unknown15, ioBuffer);
  serialize(iValue.unknown16, ioBuffer);
  serialize(iValue.unknown17, ioBuffer);
  serialize(iValue.unknown18[0], ioBuffer);
  serialize(iValue.unknown18[1], ioBuffer);
  serialize(iValue.unknown18[2], ioBuffer);
  serialize(iValue.unknown18[3], ioBuffer);

  //nameUnicode
  {
    std::string tmp = iValue.nameUnicode;
    for(unsigned long i=0; i<=tmp.size(); i++)
    {
      const char & c = iValue.nameUnicode[i];
      serialize(c, ioBuffer);
      serialize('\0', ioBuffer);
    }
  }

  serialize(iValue.unknown19, ioBuffer);
  serialize(iValue.unknown20, ioBuffer);

  return true;
}

inline bool serialize(const unsigned char * iData, const unsigned long & iSize, MemoryBuffer & ioBuffer)
{
  unsigned long oldSize = ioBuffer.getSize();
  unsigned long newSize = oldSize + iSize;
  if (ioBuffer.reallocate(newSize))
  {
    //save data
    unsigned char * buffer = ioBuffer.getBuffer();
    unsigned char * offset = &buffer[oldSize];
    for(unsigned long i=0; i<iSize; i++)
    {
      offset[i] = iData[i];
    }

    return true;
  }
  return false;
}

bool deserialize(const MemoryBuffer & iBuffer, LNK_ITEMID & oValue, std::string & oName83, std::string & oNameLong)
{
  unsigned long offset = 0;
  const unsigned char * buffer = iBuffer.getBuffer();

  oValue.size = readData<unsigned short>(buffer, offset);
  oValue.signature[0] = readData<unsigned char>(buffer, offset);
  oValue.signature[1] = readData<unsigned char>(buffer, offset);
  oValue.signature[2] = readData<unsigned char>(buffer, offset);
  oValue.signature[3] = readData<unsigned char>(buffer, offset);
  oValue.signature[4] = readData<unsigned char>(buffer, offset);
  oValue.signature[5] = readData<unsigned char>(buffer, offset);
  oValue.unknown01 = readData<unsigned char>(buffer, offset);
  oValue.unknown02 = readData<unsigned char>(buffer, offset);
  oValue.unknown03 = readData<unsigned char>(buffer, offset);
  oValue.unknown04 = readData<unsigned char>(buffer, offset);
  oValue.unknown05 = readData<unsigned char>(buffer, offset);
  oValue.unknown06 = readData<unsigned char>(buffer, offset);

  //name83
  oValue.name83 = NULL;
  {
    char c = readData<unsigned char>(buffer, offset);
    while (c != '\0')
    {
      oName83 += c;
      c = readData<unsigned char>(buffer, offset);
    }
  }

  //search for location of nameUnicode
  //nameUnicode is located at the end of the ItemID
  //following a NULL unicode character.
  const unsigned short * nameUnicodeAddress = (const unsigned short *)(&buffer[iBuffer.getSize()-2]); //last uint16_t of the ItemID
  nameUnicodeAddress--; //NULL terminating character;
  nameUnicodeAddress--; //last string character;
  while(*nameUnicodeAddress != 0x0000)
  {
    nameUnicodeAddress--; //rewind until the beginning of the 
  }
  nameUnicodeAddress++; //move to first string character
  
  //move buffer up to nameUnicodeAddress
  while(nameUnicodeAddress != (const unsigned short *)(&buffer[offset]))
  {
    unsigned char c = readData<unsigned char>(buffer, offset);
  }

  //nameUnicode
  oValue.nameUnicode = NULL;
  {
    unsigned short unicodec = readData<unsigned short>(buffer, offset);
    char c = (char)unicodec;
    while (c != '\0')
    {
      oNameLong += c;
      unicodec = readData<unsigned short>(buffer, offset);
      c = (char)unicodec;
    }
  }

  oValue.unknown19 = readData<unsigned char>(buffer, offset);
  oValue.unknown20 = readData<unsigned char>(buffer, offset);
  
  bool success = (offset == iBuffer.getSize());
  assert( success == true );
  return success;
}



//----------------------------------------------------------------------------------------------------------------------------------------
// lnkLib functions & API
//----------------------------------------------------------------------------------------------------------------------------------------
const char * getVersionString()
{
  return PRODUCT_VERSION;
}

bool isLink(const unsigned char * iBuffer, const unsigned long & iSize);

bool isLink(const MemoryBuffer & iFileContent)
{
  return isLink(iFileContent.getBuffer(), iFileContent.getSize());
}

bool isLink(const unsigned char * iBuffer, const unsigned long & iSize)
{
  if (iSize > sizeof(ShellLinkHeader))
  {
    const ShellLinkHeader * header = (const ShellLinkHeader*)iBuffer;

    if (header->HeaderSize != sizeof(ShellLinkHeader))
      return false;

    bool guidSuccess = (memcmp(header->LinkCLSID, DEFAULT_LINKCLSID, sizeof(LNK_CLSID)) == 0);
    if (!guidSuccess)
      return false;

    return true;
  }
  return false;
}

bool isLink(const char * iFilePath)
{
  MemoryBuffer fileContent;
  bool loadSuccess = fileContent.loadFile(iFilePath);
  if (loadSuccess)
  {
    //validate signature
    bool link = isLink(fileContent);
    if (link)
    {
      return true;
    }
  }
  return false;
}

bool getLinkInfo(const char * iFilePath, LinkInfo & oLinkInfo)
{
  MemoryBuffer fileContent;
  bool loadSuccess = fileContent.loadFile(iFilePath);
  if (loadSuccess)
  {
    //validate signature
    bool link = isLink(fileContent);
    if (link)
    {
      const ShellLinkHeader * header = (const ShellLinkHeader*)fileContent.getBuffer();
      const unsigned char * content = fileContent.getBuffer();

      oLinkInfo.customIcon.index = header->IconIndex;
      oLinkInfo.hotKey = header->HotKey;

      unsigned long offset = sizeof(ShellLinkHeader);

      if (header->linkFlags.HasLinkTargetIDList)
      {
        //Shell Item Id List 
        //Note: This section exists only if the first bit for link flags is set the header section.
        //      If that bit is not set then this section does not exists.
        //      The first word contains the size of the list in bytes.
        //      Each item (except the last) in the list contains its size in a word fallowed by the content.
        //      The size includes and the space used to store it. The last item has the size 0.
        //      These items are used to store various informations.
        //      For more info read the SHITEMID documentation. 
        const unsigned short & listSize = readData<unsigned short>(content, offset);
        unsigned short itemIdSize = 0xFFFF;
        while (itemIdSize != 0)
        {
          itemIdSize = readData<unsigned short>(content, offset);
          if (itemIdSize > 0)
          {
            //item is valid (last item has a size of 0)

            //read itemId's content
            MemoryBuffer itemIdContent;
            serialize(itemIdSize, itemIdContent);
            while (itemIdContent.getSize() < itemIdSize)
            {
              const unsigned char & c = readData<unsigned char>(content, offset);
              serialize(c, itemIdContent);
            }

            //check itemId's content
            const unsigned char & itemIdType = itemIdContent.getBuffer()[2];
            switch(itemIdType)
            {
            case 0x1f: //computer data. ignore
              break;
            case 0x2f: //drive data.
              oLinkInfo.target = (char*)&itemIdContent.getBuffer()[3];
              break;
            case 0x31: //folder data
            case 0x32: //file data
              {
                std::string name83;
                std::string nameLong;
                LNK_ITEMID itemId = {0};
                bool success = deserialize(itemIdContent, itemId, name83, nameLong);
                assert( success == true );
                
                if (oLinkInfo.target.size() == 0)
                {
                  oLinkInfo.target += ".\\";
                }
                else
                {
                  //since we are adding a folder of file name,
                  //make sure the path is ending with a separator
                  const char & lastCharacter = oLinkInfo.target[oLinkInfo.target.size() - 1];
                  if (lastCharacter != '\\')
                    oLinkInfo.target += '\\';
                }
                oLinkInfo.target += nameLong;
              }
            };
          }
        }
      }

      {
        //File location info
        const LNK_FILE_LOCATION_INFO & fileInfo = readData<LNK_FILE_LOCATION_INFO>(content, offset);
        offset += (fileInfo.length - fileInfo.endOffset);

        const unsigned char * baseFileLocationAddress = (unsigned char*)(&fileInfo);
        if (fileInfo.length > 0)
        {
          std::string basePath = "";
          if (fileInfo.basePathOffset)
            basePath = (const char *)&baseFileLocationAddress[fileInfo.basePathOffset];
          std::string finalPath = "";
          if (fileInfo.finalPathOffset)
            finalPath = (const char *)&baseFileLocationAddress[fileInfo.finalPathOffset];

          //concat paths
          if (oLinkInfo.target.size() == 0)
          {
            //target was not resolved using shellItemIdList, resolve using base and final paths
            if (basePath.size() > 0)
              oLinkInfo.target = basePath;
            if (finalPath.size() > 0)
            {
              if (oLinkInfo.target.size() == 0)
                oLinkInfo.target = finalPath;
              else
              {
                oLinkInfo.target += '\\';
                oLinkInfo.target += finalPath;
              }
            }
          }

          if (fileInfo.localVolumeTableOffset > 0 && fileInfo.location == LNK_LOCATION_LOCAL)
          {
            unsigned long tmpOffset = fileInfo.localVolumeTableOffset;
            const LNK_LOCAL_VOLUME_TABLE & volumeTable = readData<LNK_LOCAL_VOLUME_TABLE>(baseFileLocationAddress, tmpOffset);
            const char * volumeName = &volumeTable.volumeLabel;
            assert( volumeTable.length >= LNK_LOCAL_VOLUME_TABLE_SIZE );
          }
          if (fileInfo.networkVolumeTableOffset > 0 && fileInfo.location == LNK_LOCATION_NETWORK)
          {
            unsigned long tmpOffset = fileInfo.networkVolumeTableOffset;
            const LNK_NETWORK_VOLUME_TABLE & volumeTable = readData<LNK_NETWORK_VOLUME_TABLE>(baseFileLocationAddress, tmpOffset);
            const char * volumeName = &volumeTable.networkShareName;
            assert( volumeTable.length >= LNK_NETWORK_VOLUME_TABLE_SIZE );

            //build network path
            oLinkInfo.networkPath = volumeName;
            oLinkInfo.networkPath += '\\';
            oLinkInfo.networkPath += finalPath;
          }
        }
      }
      
      //Description
      //This section is present if bit 2 is set in the flags value in the header.
      //The first word value indicates the length of the string.
      //Following the length value is a string of ASCII characters.
      //It is a description of the item.
      if (header->linkFlags.HasName)
        readString(content, offset, oLinkInfo.description);
      
      //Relative path string
      //This section is present if bit 3 is set in the flags value in the header.
      //The first word value indicates the length of the string.
      //Following the length value is a string of ASCII characters.
      //It is a relative path to the target.
      std::string relativePath;
      if (header->linkFlags.HasRelativePath)
        readString(content, offset, relativePath);

      //Working directory
      //This section is present if bit 4 is set in the flags value in the header.
      //The first word value indicates the length of the string.
      //Following the length value is a string of ASCII characters.
      //It is the working directory as specified in the link properties.
      if (header->linkFlags.HasWorkingDir)
        readString(content, offset, oLinkInfo.workingDirectory);

      //Command line arguments
      //This section is present if bit 5 is set in the flags value in the header.
      //The first word value indicates the length of the string.
      //Following the length value is a string of ASCII characters.
      //The command line string includes everything except the program name.
      if (header->linkFlags.HasArguments)
        readString(content, offset, oLinkInfo.arguments);

      //Icon filename
      //This section is present if bit 6 is set in the flags value in the header.
      //The first word value indicates the length of the string.
      //Following the length value is a string of ASCII characters.
      //This the name of the file containing the icon.
      if (header->linkFlags.HasIconLocation)
        readString(content, offset, oLinkInfo.customIcon.filename);

      //Additonal Info Usualy consists of a dword with the value 0.
      const unsigned long & additionnalInfo = readData<unsigned long>(content, offset);

      return true;
    }
  }
  return false;
}

MemoryBuffer createShellItemIdList(const char * iFilePath, const LinkInfo & iLinkInfo)
{
  MemoryBuffer shellIdList;

  //static const unsigned char  itemsData[] = {0x14, 0x00, 0x1f, 0x50, 0xe0, 0x4f, 0xd0, 0x20, 0xea, 0x3a, 0x69, 0x10, 0xa2, 0xd8, 0x08, 0x00, 0x2b, 0x30, 0x30, 0x9d, 0x19, 0x00, 0x2f, 0x43, 0x3a, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x3e, 0xb5, 0x8b, 0x11, 0x00, 0x50, 0x52, 0x4f, 0x47, 0x52, 0x41, 0x7e, 0x31, 0x00, 0x00, 0x32, 0x00, 0x03, 0x00, 0x04, 0x00, 0xef, 0xbe, 0x77, 0x35, 0x5b, 0x38, 0x38, 0x3e, 0x0c, 0x64, 0x14, 0x00, 0x00, 0x00, 0x50, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x20, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x73, 0x00, 0x00, 0x00, 0x18, 0x00, 0x44, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x3a, 0xef, 0x70, 0x10, 0x00, 0x50, 0x44, 0x46, 0x43, 0x52, 0x45, 0x7e, 0x31, 0x00, 0x00, 0x2c, 0x00, 0x03, 0x00, 0x04, 0x00, 0xef, 0xbe, 0x57, 0x3a, 0xe7, 0x70, 0x38, 0x3e, 0x60, 0x64, 0x14, 0x00, 0x00, 0x00, 0x50, 0x00, 0x44, 0x00, 0x46, 0x00, 0x43, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x00, 0x00, 0x18, 0x00, 0x48, 0x00, 0x32, 0x00, 0xa2, 0x1a, 0x00, 0x00, 0x16, 0x35, 0x6c, 0x6a, 0x20, 0x00, 0x48, 0x69, 0x73, 0x74, 0x6f, 0x72, 0x79, 0x2e, 0x74, 0x78, 0x74, 0x00, 0x2e, 0x00, 0x03, 0x00, 0x04, 0x00, 0xef, 0xbe, 0x57, 0x3a, 0xe7, 0x70, 0x38, 0x3e, 0xc0, 0x64, 0x14, 0x00, 0x00, 0x00, 0x48, 0x00, 0x69, 0x00, 0x73, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x79, 0x00, 0x2e, 0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00};
  //static const unsigned short listSize = sizeof(itemsData);
  static const unsigned char  itemIdComputerData[] = {0x14, 0x00, 0x1f, 0x50, 0xe0, 0x4f, 0xd0, 0x20, 0xea, 0x3a, 0x69, 0x10, 0xa2, 0xd8, 0x08, 0x00, 0x2b, 0x30, 0x30, 0x9d};
  static const unsigned short itemIdComputerSize = sizeof(itemIdComputerData);
               unsigned char  itemIdDriveData[] = {0x19, 0x00, 0x2f, 'C', 0x3a, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
               unsigned short itemIdDriveSize = sizeof(itemIdDriveData);

  typedef std::vector<LNK_ITEMID> ITEMIDList;

  static const unsigned char  itemIdFinalData[] = {0x00, 0x00};
  static const unsigned short itemIdFinalDataSize = sizeof(itemIdFinalData);

  //convert the long path name to short path name
  std::string shortPath = filesystem::getShortPathForm(iLinkInfo.target.c_str());
  const char * shortPathStr = shortPath.c_str();
  if (shortPath.size() == 0)
    return shellIdList; //short path not found
  
  //split path
  StringList shortPathParts;
  filesystem::splitPath(shortPathStr, shortPathParts);
  if (shortPathParts.size() <= 2)
    return shellIdList; //short path needs at least a drive/folder/filename structure
  StringList longPathParts;
  filesystem::splitPath(iLinkInfo.target.c_str(), longPathParts);
  if (longPathParts.size() <= 2)
    return shellIdList; //short path needs at least a drive/folder/filename structure
  if (shortPathParts.size() != longPathParts.size())
    return shellIdList; //both long and short paths needs to be the same size

  //shortPathParts[0]	C:
  //shortPathParts[1]	PROGRA~1
  //shortPathParts[2]	7-Zip
  //shortPathParts[3]	History.txt

  //longPathParts[0]	C:
  //longPathParts[1]	Program Files
  //longPathParts[2]	7-Zip
  //longPathParts[3]	History.txt

  //setup drive data
  std::string & drive = shortPathParts[0]; // C:
  const char & driveLetter = drive[0];     // C
  itemIdDriveData[3] = toupper(driveLetter);

  //create data holder for folder & filename ItemId
  ITEMIDList itemIds;

  //setup folder/filename data
  size_t numParts = shortPathParts.size();
  for(size_t i=1; i<numParts; i++)
  {
    std::string & shortItem = shortPathParts[i];
    std::string &  longItem =  longPathParts[i];
    if (i+1 < numParts)
    {
      //this is a folder
      LNK_ITEMID item = LNK_ITEMIDFolderDefault;
      item.name83 = shortItem.c_str();
      item.nameUnicode = longItem.c_str();

      //save the item
      itemIds.push_back(item);
    }
    else
    {
      //this is a folder
      LNK_ITEMID item = LNK_ITEMIDFileDefault;
      item.name83 = shortItem.c_str();
      item.nameUnicode = longItem.c_str();

      //save the item
      itemIds.push_back(item);
    }
  }

  //validation
  size_t numItemIdsFound = itemIds.size();
  if (numItemIdsFound == 0)
    return shellIdList; //no folder ItemIds found
  for(size_t i=0; i<numItemIdsFound; i++)
  {
    const LNK_ITEMID & itemId = itemIds[i];
    if (itemId.name83 == NULL || itemId.nameUnicode == NULL)
      return shellIdList; //folder ItemId names are not found
  }

  //compute shellItemIdList's size
  unsigned short shellIdListSize = 0;

  //building shellItemIdList
  serialize(shellIdListSize, shellIdList);
  serialize(itemIdComputerData, itemIdComputerSize, shellIdList); //size of itemId is part of the data
  serialize(itemIdDriveData, itemIdDriveSize, shellIdList); //size of itemId is part of the data
  for(unsigned long i=0; i<numItemIdsFound; i++)
  {
    const LNK_ITEMID & itemId = itemIds[i];
    LNK_ITEMID copy = itemId;
    copy.size = getSizeOf(copy);
    serialize(copy, shellIdList);
  }
  serialize(itemIdFinalData, itemIdFinalDataSize, shellIdList); //final data is not preceded by a length

  //update shellItemIdList's size
  shellIdListSize = (unsigned short)shellIdList.getSize() - sizeof(shellIdListSize); //size does not include itself
  (*(unsigned short*)shellIdList.getBuffer()) = shellIdListSize;

  return shellIdList;
}

bool createLink(const char * iFilePath, const LinkInfo & iLinkInfo)
{
  //building header
  ShellLinkHeader header = {0};
  header.HeaderSize = sizeof(ShellLinkHeader);
  memcpy(header.LinkCLSID, DEFAULT_LINKCLSID, sizeof(LNK_CLSID));
  
  //detect target
  bool isTargetFolder = filesystem::folderExists(iLinkInfo.target.c_str());
  bool isTargetFile = filesystem::fileExists(iLinkInfo.target.c_str());

  //building LinkFlags
  {
    LinkFlags & flags = header.linkFlags;
    flags.HasLinkTargetIDList = 1;
    flags.HasLinkInfo = isTargetFile || isTargetFolder;
    flags.HasName = iLinkInfo.description.size() > 0;
    flags.HasRelativePath = 0;
    flags.HasWorkingDir = iLinkInfo.workingDirectory.size() > 0;
    flags.HasArguments = iLinkInfo.arguments.size() > 0;
    flags.HasIconLocation = iLinkInfo.customIcon.filename.size() > 0;
    flags.reserved1 = 1;
    flags.reserved2 = 0;
    flags.reserved3 = 0;
    flags.reserved4 = 0;
  }

  //build FileAttributesFlags
  {
    FileAttributesFlags & flags = header.FileAttributes;
    flags.isReadOnly = 0;
    flags.isHidden = 0;
    flags.isSystemFile = 0;
    flags.isVolumeLabel = 0;
    flags.isDirectory = isTargetFolder;
    flags.hasBeenModifiedSinceLastBackup = 1;
    flags.isEncrypted = 0;
    flags.isNormal = 0;
    flags.isTemporary = 0;
    flags.isSparseFile = 0;
    flags.hasReparsePointData = 0;
    flags.isCompressed = 0;
    flags.isOffline = 0;
    flags.reserved1 = 0;
    flags.reserved2 = 0;
    flags.reserved3 = 0;
  }

  header.CreationTime = 0;
  header.AccessTime = 0;
  header.WriteTime = 0;
  header.FileSize = (isTargetFile ? filesystem::getFileSize(iLinkInfo.target.c_str()) : 0);
  header.IconIndex = (iLinkInfo.customIcon.filename.size() > 0 ? iLinkInfo.customIcon.index : 0);
  header.ShowCommand = 1;
  header.HotKey = iLinkInfo.hotKey;
  header.reserved1 = 0;
  header.reserved2 = 0;

  //shellItemIdList
  MemoryBuffer shellItemIdList = createShellItemIdList(iFilePath, iLinkInfo);
  if (shellItemIdList.getSize() == 0)
    return false; //unable to build shellItemIdList

  //File location info
  LNK_FILE_LOCATION_INFO fileInfo = {0};
  fileInfo.length = LNK_FILE_LOCATION_INFO_SIZE + LNK_LOCAL_VOLUME_TABLE_SIZE + iLinkInfo.target.size() + 2;
  fileInfo.endOffset = LNK_FILE_LOCATION_INFO_SIZE;
  fileInfo.location = LNK_LOCATION_LOCAL;
  fileInfo.localVolumeTableOffset = LNK_FILE_LOCATION_INFO_SIZE;
  fileInfo.basePathOffset = LNK_FILE_LOCATION_INFO_SIZE + LNK_LOCAL_VOLUME_TABLE_SIZE;
  fileInfo.networkVolumeTableOffset = 0;
  fileInfo.finalPathOffset = fileInfo.length - 1;

  LNK_LOCAL_VOLUME_TABLE volumeTable = {0};
  volumeTable.length = LNK_LOCAL_VOLUME_TABLE_SIZE;
  volumeTable.volumeType = LNK_VOLUME_TYPE_FIXED;
  volumeTable.volumeSerialNumber = 0;
  volumeTable.volumeNameOffset = LNK_LOCAL_VOLUME_TABLE_SIZE - 1;
  volumeTable.volumeLabel = '\0';

  //Save data to a file
  FILE * f = fopen(iFilePath, "wb");
  if (f)
  {
    size_t tmp = 0;

    //Save header
    tmp = sizeof(header);
    fwrite(&header, 1, tmp, f);

    //shellItemIdList
    tmp = shellItemIdList.getSize();
    fwrite(shellItemIdList.getBuffer(), 1, tmp, f);

    //File location info & volume table
    tmp = sizeof(fileInfo);
    fwrite(&fileInfo, 1, tmp, f);
    tmp = sizeof(volumeTable);
    fwrite(&volumeTable, 1, tmp, f);
    saveString(f, iLinkInfo.target); //basic path
    saveString(f, ""); //final path

    LinkFlags & flags = header.linkFlags;

    //Description
    if (flags.HasName)
      saveStringUnicode(f, iLinkInfo.description);

    //Relative path string
    //(never)

    //Working directory
    if (flags.HasWorkingDir)
      saveStringUnicode(f, iLinkInfo.workingDirectory);

    //Command line arguments
    if (flags.HasArguments)
      saveStringUnicode(f, iLinkInfo.arguments);

    //Icon filename
    if (flags.HasIconLocation)
      saveStringUnicode(f, iLinkInfo.customIcon.filename);
    
    //Additonal Info Usualy consists of a dword with the value 0. 
    const unsigned long additionnalInfo = 0;
    fwrite(&additionnalInfo, 1, sizeof(additionnalInfo), f);

    fclose(f);
    return true;
  }

  return false;
}

std::string toString(const LNK_HOTKEY & iHotKey)
{
  std::string value;

  if (iHotKey.modifiers & LNK_HK_MOD_CONTROL)
  {
    if (value.size() > 0)
      value += ' ';
    value += "CTRL +";
  }
  if (iHotKey.modifiers & LNK_HK_MOD_ALT)
  {
    if (value.size() > 0)
      value += ' ';
    value += "ALT +";
  }
  if (iHotKey.modifiers & LNK_HK_MOD_SHIFT)
  {
    if (value.size() > 0)
      value += ' ';
    value += "SHIFT +";
  }
  if (value.size() > 0)
    value += ' ';

  //key
  if (iHotKey.keyCode >= '0' && iHotKey.keyCode <= 'Z')
    value += (char)iHotKey.keyCode;
  if (iHotKey.keyCode >= LNK_HK_F1 && iHotKey.keyCode <= LNK_HK_F12)
  {
    value += 'F';
    value += (char)(iHotKey.keyCode + 1 - LNK_HK_F1);
  }
  if (iHotKey.keyCode >= LNK_HK_NUMLOCK)
    value += "Numlock";
  if (iHotKey.keyCode >= LNK_HK_SCROLL)
    value += "ScroolLock";

  return value;
}

bool printLinkInfo(const char * iFilePath)
{
  FILE * f = fopen(iFilePath, "rb");
  if (f)
  {
    printf("Link file: %s\n", iFilePath);

    //read & print header
    ShellLinkHeader header = {0};
    fread(&header, 1, sizeof(header), f);
    //signature
    printf("HeaderSize: %d\n", header.HeaderSize);
    //LinkCLSID
    printf("LinkCLSID: 0x%02x 0x%02x 0x%02x 0x%02x \n", 
      header.LinkCLSID[0],
      header.LinkCLSID[1],
      header.LinkCLSID[2],
      header.LinkCLSID[3]   );
    printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", 
      header.LinkCLSID[4],
      header.LinkCLSID[5],
      header.LinkCLSID[6],
      header.LinkCLSID[7]   );
    printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", 
      header.LinkCLSID[8],
      header.LinkCLSID[9],
      header.LinkCLSID[10],
      header.LinkCLSID[11]   );
    printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", 
      header.LinkCLSID[12],
      header.LinkCLSID[13],
      header.LinkCLSID[14],
      header.LinkCLSID[15]   );
    //link flags
    printf("link flags: HasLinkTargetIDList   =%c\n", (header.linkFlags.HasLinkTargetIDList ? 'T' : 'F')  );
    printf("                HasLinkInfo  =%c\n", (header.linkFlags.HasLinkInfo ? 'T' : 'F')  );
    printf("                HasName           =%c\n", (header.linkFlags.HasName ? 'T' : 'F')  );
    printf("                HasRelativePath          =%c\n", (header.linkFlags.HasRelativePath ? 'T' : 'F')  );
    printf("                HasWorkingDir      =%c\n", (header.linkFlags.HasWorkingDir ? 'T' : 'F')  );
    printf("                HasArguments  =%c\n", (header.linkFlags.HasArguments ? 'T' : 'F')  );
    printf("                HasIconLocation            =%c\n", (header.linkFlags.HasIconLocation ? 'T' : 'F')  );
    printf("                reserved1                =%c\n", (header.linkFlags.reserved1 ? 'T' : 'F')  );
    printf("                reserved2                =%02x\n", header.linkFlags.reserved2 );
    printf("                reserved3                =%02x\n", header.linkFlags.reserved3 );
    printf("                reserved4                =%02x\n", header.linkFlags.reserved4 );
    //target flags
    printf("target flags: isReadOnly                      =%c\n", (header.FileAttributes.isReadOnly ? 'T' : 'F')  );
    printf("              isHidden                        =%c\n", (header.FileAttributes.isHidden ? 'T' : 'F')  );
    printf("              isSystemFile                    =%c\n", (header.FileAttributes.isSystemFile ? 'T' : 'F')  );
    printf("              isVolumeLabel                   =%c\n", (header.FileAttributes.isVolumeLabel ? 'T' : 'F')  );
    printf("              isDirectory                     =%c\n", (header.FileAttributes.isDirectory ? 'T' : 'F')  );
    printf("              hasBeenModifiedSinceLastBackup  =%c\n", (header.FileAttributes.hasBeenModifiedSinceLastBackup ? 'T' : 'F')  );
    printf("              isEncrypted                     =%c\n", (header.FileAttributes.isEncrypted ? 'T' : 'F')  );
    printf("              isNormal                        =%c\n", (header.FileAttributes.isNormal ? 'T' : 'F')  );
    printf("              isTemporary                     =%c\n", (header.FileAttributes.isTemporary ? 'T' : 'F')  );
    printf("              isSparseFile                    =%c\n", (header.FileAttributes.isSparseFile ? 'T' : 'F')  );
    printf("              hasReparsePointData             =%c\n", (header.FileAttributes.hasReparsePointData ? 'T' : 'F')  );
    printf("              isCompressed                    =%c\n", (header.FileAttributes.isCompressed ? 'T' : 'F')  );
    printf("              isOffline                       =%c\n", (header.FileAttributes.isOffline ? 'T' : 'F')  );
    printf("              reserved1                       =%c\n", (header.FileAttributes.reserved1 ? 'T' : 'F')  );
    printf("              reserved2                       =%c\n", (header.FileAttributes.reserved2 ? 'T' : 'F')  );
    printf("              reserved3                       =%c\n", (header.FileAttributes.reserved3 ? 'T' : 'F')  );
    std::string creationTimeStr = toTimeString(header.CreationTime);
    std::string modificationTimeStr = toTimeString(header.WriteTime);
    std::string lastAccessTimeStr = toTimeString(header.AccessTime);
    const char * test = "test";
    //PRINTF BUG. Unable to display using "0x%08x (%s)"
    //printf("time stamps:  CreationTime      = 0x%08x (%s) \n", header.CreationTime, creationTimeStr.c_str());
    //printf("              WriteTime  = 0x%08x (%s) \n", header.WriteTime, modificationTimeStr.c_str());
    //printf("              AccessTime    = 0x%08x (%s) \n", header.AccessTime, lastAccessTimeStr.c_str());
    printf("time stamps:  CreationTime      = 0x%08x ", header.CreationTime);       printf("(%s) \n", creationTimeStr.c_str());
    printf("              WriteTime  = 0x%08x ", header.WriteTime);   printf("(%s) \n", modificationTimeStr.c_str());
    printf("              AccessTime    = 0x%08x ", header.AccessTime);     printf("(%s) \n", lastAccessTimeStr.c_str());
    //remaining properties
    printf("FileSize = 0x%04x\n", header.FileSize);
    printf("IconIndex = 0x%04x\n", header.IconIndex);
    printf("ShowCommand = 0x%04x\n", header.ShowCommand);
    std::string hotKeyDescription = toString(header.HotKey);
    printf("HotKey    = 0x%04x (%s)\n", header.HotKey, hotKeyDescription.c_str());
    printf("reserved1 = 0x%04x\n", header.reserved1);
    printf("reserved2 = 0x%04x\n", header.reserved2);

    //shellItemIdList
    if (header.linkFlags.HasLinkTargetIDList)
    {
      unsigned short listSize = 0;
      fread(&listSize, 1, sizeof(listSize), f);
      printf("shellItemIdList size=0x%02x (%02d)\n", listSize, listSize);

      unsigned short itemIdSize = 0xFFFF;
      unsigned char sequenceNumber = 0;
      while (itemIdSize != 0)
      {
        fread(&itemIdSize, 1, sizeof(itemIdSize), f);
        if (itemIdSize > 0)
        {
          sequenceNumber++;

          //item is valid (last item has a size of 0)
          printf("itemId %d size=0x%02x (%02d)\n", sequenceNumber, itemIdSize, itemIdSize);
          printf("data=");

          unsigned short remainingDataSize = itemIdSize - sizeof(itemIdSize);
          while (remainingDataSize)
          {
            unsigned char c = 0;
            fread(&c, 1, sizeof(c), f);
            printf("%02x", c);
            remainingDataSize--;
            if (remainingDataSize)
              printf(" ");
          }
          printf("\n");
        }
      }
    }

    //File location info
    {
      long fileOffset = ftell(f);

      //read file location info size
      unsigned long fileInfoDataSize = 0;
      fread(&fileInfoDataSize, 1, sizeof(fileInfoDataSize), f);

      //rewind back to before the fileInfoDataSize
      fseek(f, fileOffset, SEEK_SET);

      //read the whole LNK_FILE_LOCATION_INFO data
      MemoryBuffer fileInfoData;
      fileInfoData.allocate(fileInfoDataSize);
      fread(fileInfoData.getBuffer(), 1, fileInfoData.getSize(), f);

      const LNK_FILE_LOCATION_INFO * fileInfo = (const LNK_FILE_LOCATION_INFO *)fileInfoData.getBuffer();

      printf("file location info: length                    = 0x%04x (%d)\n", fileInfo->length, fileInfo->length );
      printf("                    endOffset                 = 0x%04x (%d)\n", fileInfo->endOffset, fileInfo->endOffset );
      printf("                    location                  = 0x%04x (%d)\n", fileInfo->location, fileInfo->location );
      printf("                    localVolumeTableOffset    = 0x%04x (%d)\n", fileInfo->localVolumeTableOffset, fileInfo->localVolumeTableOffset );
      printf("                    basePathOffset            = 0x%04x (%d)\n", fileInfo->basePathOffset, fileInfo->basePathOffset );
      printf("                    networkVolumeTableOffset  = 0x%04x (%d)\n", fileInfo->networkVolumeTableOffset, fileInfo->networkVolumeTableOffset );
      printf("                    finalPathOffset           = 0x%04x (%d)\n", fileInfo->finalPathOffset, fileInfo->finalPathOffset );

      const unsigned char * baseFileLocationAddress = fileInfoData.getBuffer();;
      if (fileInfo->length > 0)
      {
        const char * basePath = "";
        if (fileInfo->basePathOffset)
          basePath = (const char *)&baseFileLocationAddress[fileInfo->basePathOffset];
        const char * finalPath = "";
        if (fileInfo->finalPathOffset)
          finalPath = (const char *)&baseFileLocationAddress[fileInfo->finalPathOffset];

        printf("                    basePath                  = \"%s\" \n", basePath);
        printf("                    finalPath                 = \"%s\" \n", finalPath);

        if (fileInfo->localVolumeTableOffset > 0 && fileInfo->location == LNK_LOCATION_LOCAL)
        {
          const LNK_LOCAL_VOLUME_TABLE * volumeTable = (LNK_LOCAL_VOLUME_TABLE *)&baseFileLocationAddress[fileInfo->localVolumeTableOffset];
          const char * volumeName = &volumeTable->volumeLabel;
          assert( volumeTable->length >= LNK_LOCAL_VOLUME_TABLE_SIZE );

          printf("                    LNK_LOCAL_VOLUME_TABLE:\n");
          printf("                           length             = 0x%04x (%d)\n", volumeTable->length ,volumeTable->length );
          printf("                           volumeType         = 0x%04x (%d)\n", volumeTable->volumeType, volumeTable->volumeType );
          printf("                           volumeSerialNumber = 0x%04x (%d)\n", volumeTable->volumeSerialNumber, volumeTable->volumeSerialNumber );
          printf("                           volumeNameOffset   = 0x%04x (%d)\n", volumeTable->volumeNameOffset, volumeTable->volumeNameOffset );
          printf("                           volumeLabel        = \"%s\" \n", volumeName);
        }
        if (fileInfo->networkVolumeTableOffset > 0 && fileInfo->location == LNK_LOCATION_NETWORK)
        {
          const LNK_NETWORK_VOLUME_TABLE * volumeTable = (LNK_NETWORK_VOLUME_TABLE *)&baseFileLocationAddress[fileInfo->networkVolumeTableOffset];
          const char * volumeName = &volumeTable->networkShareName;
          assert( volumeTable->length >= LNK_NETWORK_VOLUME_TABLE_SIZE );

          printf("                    LNK_NETWORK_VOLUME_TABLE:\n");
          printf("                           length                 = 0x%04x (%d)\n", volumeTable->length ,volumeTable->length );
          printf("                           reserved1              = 0x%04x (%d)\n", volumeTable->reserved1, volumeTable->reserved1 );
          printf("                           networkShareNameOffset = 0x%04x (%d)\n", volumeTable->networkShareNameOffset, volumeTable->networkShareNameOffset );
          printf("                           reserved2              = 0x%04x (%d)\n", volumeTable->reserved2, volumeTable->reserved2 );
          printf("                           reserved3              = 0x%04x (%d)\n", volumeTable->reserved3, volumeTable->reserved3 );
          printf("                           networkShareName       = \"%s\" \n", volumeName);
        }
      }
    }

    //Description
    if (header.linkFlags.HasName)
    {
      std::string value = readUnicodeString(f);
      printf("Description = \"%s\" \n", value.c_str() );
    }

    //Relative path string
    if (header.linkFlags.HasRelativePath)
    {
      std::string value = readUnicodeString(f);
      printf("Relative path string = \"%s\" \n", value.c_str() );
    }

    //Working directory
    if (header.linkFlags.HasWorkingDir)
    {
      std::string value = readUnicodeString(f);
      printf("Working directory = \"%s\" \n", value.c_str() );
    }

    //Command line arguments
    if (header.linkFlags.HasArguments)
    {
      std::string value = readUnicodeString(f);
      printf("Command line arguments = \"%s\" \n", value.c_str() );
    }

    //Icon filename
    if (header.linkFlags.HasIconLocation)
    {
      std::string value = readUnicodeString(f);
      printf("Icon filename = \"%s\" \n", value.c_str() );
    }

    //Additonal Info Usualy consists of a dword with the value 0. 
    unsigned long additionalInfoBlockSize = 0;
    fread(&additionalInfoBlockSize, 1, sizeof(additionalInfoBlockSize), f);
    unsigned long blockNumber = 0;
    while (additionalInfoBlockSize > 0)
    {
      blockNumber++;

      //rewind for reading block size
      long seekOffset = sizeof(additionalInfoBlockSize);
      seekOffset *= -1;
      fseek(f, seekOffset, SEEK_CUR);

      //read additionnal info block
      MemoryBuffer block(additionalInfoBlockSize);
      fread(block.getBuffer(), 1, block.getSize(), f);

      printf("Additionnal information block #%d size=0x%02x (%02d) \n", blockNumber, additionalInfoBlockSize, additionalInfoBlockSize);
      printf("data=");

      const unsigned char * content = block.getBuffer();
      for(unsigned long i=sizeof(additionalInfoBlockSize); i<block.getSize(); i++)
      {
        const unsigned char & c = content[i];
        printf("%02x", c);
        if (i+1<block.getSize())
          printf(" ");
      }
      printf("\n");

      //read next additional block size
      fread(&additionalInfoBlockSize, 1, sizeof(additionalInfoBlockSize), f);
    }

    fclose(f);
    return true;
  }

  return false;
}

std::string getLinkCommand(const char * iFilePath)
{
  LinkInfo info;
  getLinkInfo(iFilePath, info);

  std::string value;

  //is target an absolute path ?
  std::string & target = info.target;
  if (target.size() >= 3 && target[1] == ':')
  {
    //path is absolute
    value = target;
  }
  else
  {
    //assuming path is relative
    value = info.workingDirectory;

    //add a folder separator at the end of the working directory
    const char & lastChar = value[value.size()-1];
    if (lastChar != '/' && lastChar != '\\')
    {
      value += '\\';
    }

    //is target starting as in .\folder1\folder2
    if (target.size() >= 2 &&
        target[0] == '.' &&
        (target[1] == '/' || target[1] == '\\')      )
    {
      const char * relativePath = &target[2];
      value += relativePath;
    }
    else
    {
      //target starts with a folder/file name
      value += target;
    }
  }

  if (info.arguments.size() > 0)
  {
    value += ' ';
    value += info.arguments;
  }

  return value;
}

}; //lnk
