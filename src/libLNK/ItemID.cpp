#include "ItemID.h"
#include <stdint.h>
#include <assert.h>

namespace lnk
{

  MemoryBuffer getLinkTargetIDList(const ItemIDList & iItemIDList)
  {
    //assert iItemIDList already contains TerminalID
    assert (iItemIDList.size() != 0);
    assert (iItemIDList[iItemIDList.size()-1].getSize() == 2);

    //build LinkTargetIDList
    MemoryBuffer buffer;

    //compute total size
    uint16_t IDListSize = 0;
    for(size_t i=0; i<iItemIDList.size(); i++)
    {
      const MemoryBuffer & ItemID = iItemIDList[i];
      IDListSize += (size_t)ItemID.getSize();
    }
    serialize(IDListSize, buffer);

    //serialize all ItemID
    for(size_t i=0; i<iItemIDList.size(); i++)
    {
      const MemoryBuffer & ItemID = iItemIDList[i];
      serialize(ItemID.getBuffer(), ItemID.getSize(), buffer);
    }

    return buffer;
  }

  MemoryBuffer getTerminalItemId()
  {
    static const uint16_t terminalId = 0;

    MemoryBuffer buffer;
    serialize(terminalId, buffer);

    return buffer;
  }

  MemoryBuffer getComputerItemId()
  {
    static const uint8_t computer[] = {0x14, 0x00, 0x1f, 0x50, 0xe0, 0x4f, 0xd0, 0x20, 0xea, 0x3a, 0x69, 0x10, 0xa2, 0xd8, 0x08, 0x00, 0x2b, 0x30, 0x30, 0x9d};

    MemoryBuffer buffer;
    serialize(computer, sizeof(computer), buffer);

    return buffer;
  }

  MemoryBuffer getDriveItemId(char iDriveLetter)
  {
    static const uint8_t drive[] = {0x19, 0x00, 0x2f, 'C', ':', '\\', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    MemoryBuffer buffer;
    serialize(drive, sizeof(drive), buffer);

    //fix drive letter
    uint8_t & letter = buffer.getBuffer()[3];
    letter = (uint8_t)iDriveLetter;

    return buffer;
  }

  MemoryBuffer getFileItemId(const std::string & iShortName, const std::string & iLongName, const FILE_ATTRIBUTES & iAttributes)
  {
    //validation
    assert( iAttributes == FA_NORMAL ||
            iAttributes == FA_DIRECTORY);

    ItemIDHeader header;
    header.size = 0;
    header.type = ( (iAttributes == FA_DIRECTORY) ? 0x31 : 0x32 );
    header.fileAttributes = ( (iAttributes == FA_DIRECTORY) ? 0x0010 : 0x0020 );

    MemoryBuffer buffer;

    serialize(header.size, buffer);
    serialize(header.type, buffer);
    serialize((uint8_t)0x00, buffer); // unknown1
    serialize((uint8_t)0x00, buffer); // unknown1
    serialize((uint8_t)0x00, buffer); // unknown1
    serialize((uint8_t)0x00, buffer); // unknown1
    serialize((uint8_t)0x00, buffer); // unknown1
    serialize((uint8_t)0x3A, buffer); // unknown1  0x34
    serialize((uint8_t)0x3E, buffer); // unknown1  0x3E
    if (iAttributes == FA_DIRECTORY)
    {
      serialize((uint8_t)0x0E, buffer); // unknown1  0xDD
    }
    else
    {
      serialize((uint8_t)0x13, buffer); // unknown1  0xDD
    }
    serialize((uint8_t)0x6B, buffer); // unknown1  0x60
    serialize(header.fileAttributes, buffer);

    //shortname
    serialize((const uint8_t *)iShortName.c_str(), iShortName.size()+1, buffer);

    //padding ?
    serialize((uint8_t)0x00, buffer);

    //add ItemIDEx
    MemoryBuffer itemIdExBuffer = getWinXpItemIdEx(iLongName, iAttributes);
    serialize(itemIdExBuffer.getBuffer(), itemIdExBuffer.getSize(), buffer);

    //fix size
    uint16_t * size = (uint16_t *)buffer.getBuffer();
    *size = (uint16_t)buffer.getSize();

    return buffer;
  }

  MemoryBuffer getWinXpItemIdEx(const std::string & iLongName, const FILE_ATTRIBUTES & iAttributes)
  {
    MemoryBuffer buffer;

    ItemIDEx header;
    header.size = 0;
    header.type = 0x00040003; //WinXP style

    serialize(header.size, buffer);
    serialize(header.type, buffer);
    serialize((uint8_t)0xEF, buffer); //always
    serialize((uint8_t)0xBE, buffer); //always

    serialize((uint8_t)0x3A, buffer); // 0x77
    serialize((uint8_t)0x3E, buffer); // 0x35

    if (iAttributes == FA_DIRECTORY)
    {
      serialize((uint8_t)0x0E, buffer); // 0x1C
    }
    else
    {
      serialize((uint8_t)0x13, buffer); // 0x1C
    }

    serialize((uint8_t)0x6B, buffer); // 0x37
    serialize((uint8_t)0x3A, buffer); // 0x34
    serialize((uint8_t)0x3E, buffer); // 0x3E

    if (iAttributes == FA_DIRECTORY)
    {
      serialize((uint8_t)0x0E, buffer); // 0xE6
    }
    else
    {
      serialize((uint8_t)0x13, buffer); // 0x1C
    }

    serialize((uint8_t)0x6B, buffer); // 0x60
    serialize((uint8_t)0x14, buffer);
    serialize((uint8_t)0x00, buffer);
    serialize((uint8_t)0x00, buffer);
    serialize((uint8_t)0x00, buffer);

    //long name
    for(size_t i=0; i<=iLongName.size(); i++) //include NULL character
    {
      uint16_t c = iLongName[i];
      serialize(c, buffer);
    }

    serialize((uint8_t)0x18, buffer); //0x12, 0x14, 0x16, 0x18 or 0x1A
    serialize((uint8_t)0x00, buffer); //always

    //fix size
    uint16_t * size = (uint16_t *)buffer.getBuffer();
    *size = (uint16_t)buffer.getSize();

    return buffer;
  }

}; //lnk
