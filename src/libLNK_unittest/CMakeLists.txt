include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/libLNK)

link_directories(${LIBRARY_OUTPUT_PATH})

include_directories(${CMAKE_SOURCE_DIR}/../third_party/googletest/include)
link_directories(${CMAKE_SOURCE_DIR}/../third_party/googletest/msvc2010)

add_executable(libLNK_unittest
  gtesthelper.cpp
  gtesthelper.h
  TestLNK.cpp
  TestLNK.h
  TestStringFunc.cpp
  TestStringFunc.h
  TestEnvironmentFunc.cpp
  TestEnvironmentFunc.h
  TestFilesystemFunc.cpp
  TestFilesystemFunc.h
  main.cpp
)

#Copy test files in project directory to be able to run tests from Visual Studio
add_custom_command(TARGET libLNK_unittest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/libLNK_unittest/tests ${CMAKE_BINARY_DIR}/libLNK_unittest/tests)

#Copy test files to Visual Studio $OutDir to be able to run tests by launching output files directly
add_custom_command(TARGET libLNK_unittest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/libLNK_unittest/tests $<TARGET_FILE_DIR:libLNK_unittest>/tests)

if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(libLNK_unittest debug     libLNK.lib common.lib gtest.lib)
target_link_libraries(libLNK_unittest optimized libLNK.lib common.lib gtest.lib)
