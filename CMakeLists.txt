cmake_minimum_required(VERSION 3.0.0)
project(libidevice VERSION 0.1.0)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# libimobiledevice
if (NOT DEFINED ENV{IMB_PKG_CONFIG_PATH})
    message(FATAL_ERROR "not defined env var: IMB_PKG_CONFIG_PATH for imobiledevice") # "../libimobiledevice/built/darwin-arm64/lib/pkgconfig"
endif()
set(ENV{PKG_CONFIG_PATH} $ENV{IMB_PKG_CONFIG_PATH})
find_package(PkgConfig)
pkg_check_modules(imobiledevice REQUIRED IMPORTED_TARGET libimobiledevice-1.0)
pkg_check_modules(usbmuxd REQUIRED IMPORTED_TARGET libusbmuxd-2.0)
pkg_check_modules(plist REQUIRED IMPORTED_TARGET libplist-2.0)
include_directories(${imobiledevice_INCLUDE_DIRS} ${plist_INCLUDE_DIRS})

# libnskeyedarchiver
add_subdirectory("./libs/libnskeyedarchiver")
include_directories("./libs/libnskeyedarchiver/include")

# library: libidevice
include_directories("./include")
set(HEADERS
    include/idevice/blockingqueue.h
    include/idevice/bytebuffer.h
    include/idevice/idevice.h
    include/idevice/instrument.h
    include/idevice/iservice.h
    include/idevice/dtxmessenger.h
    include/idevice/dtxmessage.h
    include/idevice/dtxmessageparser.h
    include/idevice/dtxmessagetransmitter.h
    include/idevice/dtxconnection.h
    include/idevice/dtxchannel.h
    include/idevice/dtxtransport.h
    include/idevice/dtxprimitivearray.h
    include/idevice/macro_def.h
    include/idevice/macro_undef.h
)
set(SOURCES
    src/instrument.cpp
    src/idevice.cpp
    src/dtxmessage.cpp
    src/dtxmessageparser.cpp
    src/dtxmessagetransmitter.cpp
    src/dtxconnection.cpp
    src/dtxchannel.cpp
    src/dtxtransport.cpp
    src/dtxprimitivearray.cpp
)
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  nskeyedarchiver
  PkgConfig::imobiledevice
  PkgConfig::usbmuxd 
  PkgConfig::plist
)

# test
enable_testing()
add_executable(
  ${PROJECT_NAME}_test
  test/blockingqueue_test.cpp
  test/bytebuffer_test.cpp
  test/idevice_test.cpp
  test/instrument_test.cpp
  test/dtxprimitivearray_test.cpp
  test/dtxmessageparser_test.cpp
  test/dtxmessagetransmitter_test.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_test
  ${PROJECT_NAME}
  gtest_main
)
add_definitions(-DENABLE_NSKEYEDARCHIVE_TEST)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)

# fuzzer
#add_executable(
#  ${PROJECT_NAME}_fuzzer
#  test/dtxmessageparser_fuzzer.cpp
#)
#target_compile_options(${PROJECT_NAME}_fuzzer PUBLIC -g -O1 -fsanitize=fuzzer)
#set_target_properties(${PROJECT_NAME}_fuzzer PROPERTIES LINK_FLAGS -fsanitize=fuzzer)
#target_link_libraries(${PROJECT_NAME}_fuzzer ${PROJECT_NAME})

# tools: idevice
include_directories("./tools")
add_executable(
  idevice
  tools/argparser.hpp
  tools/argparser.cpp
  tools/decoder.hpp
  tools/decoder.cpp
  tools/instruments.hpp
  tools/instruments.cpp
  tools/main.cpp
)
target_link_libraries(
  idevice
  ${PROJECT_NAME}
)

# asan
#target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer)
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -fsanitize=address)
#target_compile_options(${PROJECT_NAME}_test PUBLIC -fsanitize=address -fno-omit-frame-pointer)
#set_target_properties(${PROJECT_NAME}_test PROPERTIES LINK_FLAGS -fsanitize=address)