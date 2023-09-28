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
    include/idevice/common/idevice.h
    include/idevice/common/macro_def.h
    include/idevice/common/macro_undef.h

    include/idevice/utils/blockingqueue.h
    include/idevice/utils/bytebuffer.h

    include/idevice/service/iservice.h
    include/idevice/service/lockdownservice.h
    include/idevice/service/amfiservice.h

    include/idevice/instrument/instrument.h
    include/idevice/instrument/dtxmessenger.h
    include/idevice/instrument/dtxmessage.h
    include/idevice/instrument/dtxmessageparser.h
    include/idevice/instrument/dtxmessagetransmitter.h
    include/idevice/instrument/dtxconnection.h
    include/idevice/instrument/dtxchannel.h
    include/idevice/instrument/dtxtransport.h
    include/idevice/instrument/dtxprimitivearray.h
    include/idevice/instrument/kperf.h
)
set(SOURCES
    src/instrument/instrument.cpp
    src/instrument/dtxmessage.cpp
    src/instrument/dtxmessageparser.cpp
    src/instrument/dtxmessagetransmitter.cpp
    src/instrument/dtxconnection.cpp
    src/instrument/dtxchannel.cpp
    src/instrument/dtxtransport.cpp
    src/instrument/dtxprimitivearray.cpp
    src/instrument/kperf.cpp

    src/service/lockdownservice.cpp
    src/service/amfiservice.cpp
)
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
  nskeyedarchiver
  PkgConfig::imobiledevice
  PkgConfig::usbmuxd 
  PkgConfig::plist
)

# test
enable_testing()
add_executable(
  ${PROJECT_NAME}_test
  test/common/blockingqueue_test.cpp
  test/common/bytebuffer_test.cpp
  test/common/idevice_test.cpp
  test/instrument/dtxprimitivearray_test.cpp
  test/instrument/dtxmessageparser_test.cpp
  test/instrument/dtxmessagetransmitter_test.cpp
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
  tools/lockdown.hpp
  tools/lockdown.cpp
  tools/amfi.hpp
  tools/amfi.cpp
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