#include "idevice/dtxmessageparser.h"

#include <map>
#include <cstdlib> // abs
#include <vector>
#include <unordered_map>
#include <gtest/gtest.h>

#include "idevice/idevice.h"
#include "idevice/bytebuffer.h"
#ifdef ENABLE_NSKEYEDARCHIVE_TEST
#include "nskeyedarchiver/nskeyedunarchiver.hpp"
#include "nskeyedarchiver/kamap.hpp"
#include "nskeyedarchiver/kaarray.hpp"
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
  } while (0)
//  idevice::hexdump(buffer, buffer_size, 0);                              \

TEST(DTXMessageParserTest, ParseIncomingBytes_SingleFragment) {
  char* buffer = nullptr;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("dtxmsg_enableexpiredpidtracking.bin");
  
  DTXMessageParser parser;
  bool ret = parser.ParseIncomingBytes(buffer, buffer_size);
  ASSERT_TRUE(ret);
  
  std::vector<std::shared_ptr<DTXMessage>> messages = parser.PopAllParsedMessages();
  ASSERT_EQ(1, messages.size());
  
  std::shared_ptr<DTXMessage> msg = messages.at(0);
  ASSERT_EQ(3221, msg->Identifier());
  ASSERT_EQ(347 + 0x20, msg->CostSize());
  ASSERT_EQ(0, msg->ConversationIndex());
  ASSERT_EQ(1, msg->ChannelCode());
  ASSERT_EQ(true, msg->ExpectsReply());
  ASSERT_EQ(2, msg->MessageType());
  
  const std::unique_ptr<DTXPrimitiveArray>& auxiliary = msg->Auxiliary();
  ASSERT_EQ(1, auxiliary->Size());
  ASSERT_EQ(2, auxiliary->At(0).GetType());
  ASSERT_EQ(138, auxiliary->At(0).Size());
  
  ASSERT_EQ(331 - 166, msg->PayloadSize());
  ASSERT_EQ(165, msg->PayloadSize());
  const std::unique_ptr<nskeyedarchiver::KAValue>& payload = msg->PayloadObject();
  ASSERT_TRUE(payload != nullptr);
  ASSERT_EQ(nskeyedarchiver::KAValue::Str, payload->GetDataType());
  ASSERT_STREQ("enableExpiredPidTracking:", payload->ToStr());
}

TEST(DTXMessageParserTest, ParseIncomingBytes_MultipleFragments) {
  char* buffer = nullptr;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("dtxmsg_runningprocesses.bin");
  
  DTXMessageParser parser;
  bool ret = parser.ParseIncomingBytes(buffer, buffer_size);
  ASSERT_TRUE(ret);
  
  std::vector<std::shared_ptr<DTXMessage>> messages = parser.PopAllParsedMessages();
  ASSERT_EQ(1, messages.size());
  
  std::shared_ptr<DTXMessage> msg = messages.at(0);
  ASSERT_EQ(3, msg->Identifier());
  ASSERT_EQ(96806 + 0x20, msg->CostSize());
  ASSERT_EQ(1, msg->ConversationIndex());
  ASSERT_EQ(1, msg->ChannelCode());
  ASSERT_EQ(false, msg->ExpectsReply());
  ASSERT_EQ(3, msg->MessageType());  // TODO
  
  const std::unique_ptr<DTXPrimitiveArray>& auxiliary = msg->Auxiliary();
  ASSERT_EQ(nullptr, auxiliary);
  
  ASSERT_EQ(96806 - 0x10 /* payloadHeader */, msg->PayloadSize()); // the `length` in the first fragment of the message
  const std::unique_ptr<nskeyedarchiver::KAValue>& payload = msg->PayloadObject();
  ASSERT_TRUE(payload != nullptr);
  ASSERT_EQ(nskeyedarchiver::KAValue::Object, payload->GetDataType());
  const nskeyedarchiver::KAArray& array = payload->AsObject<nskeyedarchiver::KAArray>();
  ASSERT_EQ(356, array.Size());
  
  std::unordered_map<std::string, nskeyedarchiver::KAValue> processes_by_name;
  for (const nskeyedarchiver::KAValue& item : array.ToArray()) {
    printf("item: %s\n", item.ToJson().c_str());
    const nskeyedarchiver::KAMap& process_info = item.AsObject<nskeyedarchiver::KAMap>();
    std::string process_name = process_info.at("name").ToStr();
    processes_by_name.insert(std::make_pair(process_name, process_info /* copy */));
  }
  ASSERT_EQ(27315, processes_by_name.find("Evernote")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(20440, processes_by_name.find("JD4iPhone")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(14121, processes_by_name.find("QQ")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(27314, processes_by_name.find("QQMusic")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(24283, processes_by_name.find("homed")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(27220, processes_by_name.find("com.apple.dt.instruments.dtsecurity")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(309, processes_by_name.find("Weather")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(26597, processes_by_name.find("AlipayWallet")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(21584, processes_by_name.find("iostest")->second.AsObject<nskeyedarchiver::KAMap>().at("pid").ToInteger());
  ASSERT_EQ(true, processes_by_name.find("iostest")->second.AsObject<nskeyedarchiver::KAMap>().at("isApplication").ToBool());
  ASSERT_STREQ("/var/containers/Bundle/Application/647EAEA6-A92D-44CF-A5C5-27360BA8AFFD/iostest.app/iostest",
            processes_by_name.find("iostest")->second.AsObject<nskeyedarchiver::KAMap>().at("realAppName").ToStr());
}
