include_directories(${CMAKE_SOURCE_DIR}/common)

link_directories(${LIBRARY_OUTPUT_PATH})

add_library(libLNK STATIC libLNK.cpp libLNK.h MemoryBuffer.cpp MemoryBuffer.h ItemID.cpp ItemID.h)

if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(libLNK debug     common.lib)
target_link_libraries(libLNK optimized common.lib)
