link_directories(${LIBRARY_OUTPUT_PATH})

add_library(libLNK STATIC libLNK.cpp libLNK.h)

if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
