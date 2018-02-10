add_library(common STATIC environmentfunc.cpp environmentfunc.h filesystemfunc.cpp filesystemfunc.h stringfunc.cpp stringfunc.h)

if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
