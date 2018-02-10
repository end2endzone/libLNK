#pragma once

namespace lnk
{

class MemoryBuffer
{
public:
  MemoryBuffer(void);
  MemoryBuffer(unsigned long iSize);
  MemoryBuffer(const MemoryBuffer & arg);
  virtual ~MemoryBuffer(void);

  //----------------
  // public methods
  //----------------
  void clear();
  unsigned char * getBuffer();
  const unsigned char * getBuffer() const;
  bool allocate(unsigned long iSize);
  bool reallocate(unsigned long iSize);
  unsigned long getSize() const;
  bool loadFile(const char * iFilePath);

private:
  unsigned char* mBuffer;
  unsigned long mSize;
};

}; //lnk
