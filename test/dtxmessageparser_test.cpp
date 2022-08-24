#include "idevice/dtxmessageparser.h"

#include <map>
#include <cstdlib> // abs
#include <vector>
#include <gtest/gtest.h>

#include "idevice/idevice.h"
#include "idevice/bytebuffer.h"
#ifdef ENABLE_NSKEYEDARCHIVE_TEST
#include "nskeyedarchiver/nskeyedunarchiver.hpp"
#include "nskeyedarchiver/kamap.hpp"
#endif

using namespace idevice;

#define TEST_DIR "../../test/data/"
#define READ_CONTENT_FROM_FILE(filename)                                   \
  do {                                                                     \
    FILE* f = fopen(TEST_DIR filename, "rb");                              \
    if (!f) {                                                              \
      printf("can not open `%s` file\n", TEST_DIR filename);               \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    struct stat filestats;                                                 \
    stat(TEST_DIR filename, &filestats);                                   \
    buffer_size = filestats.st_size;                                       \
    if (buffer_size == 0) {                                                \
      printf("can not get `%s` file size\n", TEST_DIR filename);           \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    buffer = (char*)malloc(buffer_size);                                   \
    fread(buffer, 1, buffer_size, f);                                      \
    fclose(f);                                                             \
                                                                           \
    if (buffer == nullptr) {                                               \
      printf("can not read `%s` file\n", TEST_DIR filename);               \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    printf("filename=%s, buffer=%p, buffer_size=%zu\n", TEST_DIR filename, \
           static_cast<void*>(buffer), buffer_size);                       \
    idevice::hexdump(buffer, buffer_size, 0);                              \
  } while (0)

TEST(DTXMessageParserTest, ParseIncomingBytes_0) {
  char* buffer = nullptr;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("dtxmsg_enableexpiredpidtracking.bin");
  
  DTXMessageParser parser;
  bool ret = parser.ParseIncomingBytes(buffer, buffer_size);
  ASSERT_TRUE(ret);
}
