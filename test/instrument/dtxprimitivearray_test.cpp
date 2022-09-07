#include "idevice/instrument/dtxprimitivearray.h"

#include <gtest/gtest.h>

#include <cstdlib>  // abs
#include <map>
#include <vector>

#include "idevice/utils/bytebuffer.h"
#include "idevice/common/idevice.h"
#ifdef ENABLE_NSKEYEDARCHIVE_TEST
#include "nskeyedarchiver/kamap.hpp"
#include "nskeyedarchiver/nskeyedarchiver.hpp"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"
#endif

using namespace idevice;

constexpr int auxiliary_header_size = 0x10;

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
//idevice::hexdump(buffer, buffer_size, 0);                              \

TEST(DTXPrimitiveArrayTest, Ctor) {
  DTXPrimitiveArray array;

  // 0: kNull
  array.Append(DTXPrimitiveValue());

  // 1: kString
  const char* str = "hello world";
  array.Append(DTXPrimitiveValue(str, strlen(str)));

  // 2: kBuffer
  char buffer[] = {0x01, 0x02, 0x03, 0x04};
  array.Append(DTXPrimitiveValue(buffer, sizeof(buffer)));

  // 3: kSignedInt32
  array.Append(DTXPrimitiveValue((int32_t)32));

  // 4: kSignedInt64
  array.Append(DTXPrimitiveValue((int64_t)64));

  // 5: kFloat32
  array.Append(DTXPrimitiveValue((float)3.2));

  // 6: kFloat64
  array.Append(DTXPrimitiveValue((double)6.4));

  // 7: kInteger
  array.Append(DTXPrimitiveValue((uint64_t)64));

  ASSERT_EQ(8, array.Size());

  DTXPrimitiveValue& value0 = array.At(0);
  ASSERT_EQ(DTXPrimitiveValue::kNull, value0.GetType());
  ASSERT_EQ(0, value0.Size());

  DTXPrimitiveValue& value1 = array.At(1);
  ASSERT_EQ(DTXPrimitiveValue::kString, value1.GetType());
  ASSERT_EQ(strlen(str)/* without ending \0 */, value1.Size());
  ASSERT_STREQ(str, value1.ToStr());

  DTXPrimitiveValue& value2 = array.At(2);
  ASSERT_EQ(DTXPrimitiveValue::kBuffer, value2.GetType());
  ASSERT_EQ(4, value2.Size());
  ASSERT_EQ(buffer[0], value2.ToBuffer()[0]);
  ASSERT_EQ(buffer[1], value2.ToBuffer()[1]);
  ASSERT_EQ(buffer[2], value2.ToBuffer()[2]);
  ASSERT_EQ(buffer[3], value2.ToBuffer()[3]);

  DTXPrimitiveValue& value3 = array.At(3);
  ASSERT_EQ(DTXPrimitiveValue::kSignedInt32, value3.GetType());
  ASSERT_EQ(4, value3.Size());
  ASSERT_EQ(32, value3.ToSignedInt32());

  DTXPrimitiveValue& value4 = array.At(4);
  ASSERT_EQ(DTXPrimitiveValue::kSignedInt64, value4.GetType());
  ASSERT_EQ(8, value4.Size());
  ASSERT_EQ(64, value4.ToSignedInt64());

  DTXPrimitiveValue& value5 = array.At(5);
  ASSERT_EQ(DTXPrimitiveValue::kFloat32, value5.GetType());
  ASSERT_EQ(4, value5.Size());
  ASSERT_FLOAT_EQ(3.2, value5.ToFloat32());

  DTXPrimitiveValue& value6 = array.At(6);
  ASSERT_EQ(DTXPrimitiveValue::kFloat64, value6.GetType());
  ASSERT_EQ(8, value6.Size());
  ASSERT_DOUBLE_EQ(6.4, value6.ToFloat64());

  DTXPrimitiveValue& value7 = array.At(7);
  ASSERT_EQ(DTXPrimitiveValue::kInteger, value7.GetType());
  ASSERT_EQ(8, value7.Size());
  ASSERT_EQ(64, value7.ToInteger());
}

TEST(DTXPrimitiveArrayTest, Deserialize) {
  char* buffer = nullptr;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("dtxmsg_notifyofpublishedcapabilities.bin");

  uint32_t message_type = *(uint32_t*)(buffer);
  uint32_t auxiliary_length = *(uint32_t*)(buffer + 0x04);
  uint64_t total_length = *(uint64_t*)(buffer + 0x08);
  uint64_t payload_length = total_length - auxiliary_length;
  ASSERT_EQ(2, message_type);
  ASSERT_EQ(7667, auxiliary_length);
  ASSERT_EQ(7838, total_length);
  ASSERT_EQ(171, payload_length);
  buffer += 0x10;

  std::unique_ptr<DTXPrimitiveArray> auxiliaries =
      DTXPrimitiveArray::Deserialize(buffer, auxiliary_length);
  ASSERT_TRUE(auxiliaries != nullptr);
  buffer += auxiliary_length;
  ASSERT_EQ(1, auxiliaries->Size());

  DTXPrimitiveValue& value0 = auxiliaries->At(0);
  ASSERT_EQ(DTXPrimitiveValue::kBuffer, value0.GetType());
  ASSERT_EQ(auxiliary_length - auxiliary_header_size - 4 - 4 - 4, value0.Size());
  printf("=================================\n");
  idevice::hexdump(value0.ToBuffer(), value0.Size(), 0);

#ifdef ENABLE_NSKEYEDARCHIVE_TEST
  // clang-format off
  nskeyedarchiver::KAValue obj = nskeyedarchiver::NSKeyedUnarchiver::UnarchiveTopLevelObjectWithData(value0.ToBuffer(), (uint32_t)value0.Size());
  printf("%s\n", obj.ToJson().c_str());
  ASSERT_EQ(nskeyedarchiver::KAValue::DataType::Object, obj.GetDataType());
  const nskeyedarchiver::KAMap& root = obj.AsObject<nskeyedarchiver::KAMap>();
  ASSERT_STREQ(root.ClassName().c_str(), "NSMutableDictionary");
  ASSERT_EQ(97, root.Size());
  ASSERT_EQ(root.at("com.apple.dt.Instruments.inlineCapabilities").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.dt.Xcode.WatchProcessControl").ToInteger(), 3);
  ASSERT_EQ(root.at("com.apple.dt.services.capabilities.vmtracking").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.ConditionInducer").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.LocationSimulation").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.activitytracetap").ToInteger(), 6);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.activitytracetap.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.activitytracetap.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.activitytracetap.windowed").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.assets").ToInteger(), 4);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.assets.response").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.config").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.multipleTimeTriggers").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.pmc").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.pmi").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coreprofilesessiontap.windowed").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.coresampling").ToInteger(), 10);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.device.applictionListing").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.device.xpccontrol").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo").ToInteger(), 113);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.app-life-cycle").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.condition-inducer").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.devicesymbolication").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.dyld-tracing").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.energytracing.location").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.gcd-perf").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.gpu-allocation").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.metal").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.recordOptions").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.scenekit-tracing").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.deviceinfo.systemversion").ToInteger(), 150500);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.filetransfer").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.filetransfer.debuginbox").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu.counters").ToInteger(), 4);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu.performancestate").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.gpu.shaderprofiler").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.coreanimation").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.coreanimation.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.coreanimation.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.opengl").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.opengl.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.graphics.opengl.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.httparchiverecording").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.mobilenotifications").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.networking").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.networking.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.networking.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.objectalloc").ToInteger(), 5);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.objectalloc.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.objectalloc.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.objectalloc.zombies").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.processcontrol").ToInteger(), 107);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.processcontrol.capability.memorylimits").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.processcontrol.capability.signal").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.processcontrol.feature.deviceio").ToInteger(), 103);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.processcontrolbydictionary").ToInteger(), 4);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.remoteleaks").ToInteger(), 8);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.remoteleaks.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.remoteleaks.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sampling").ToInteger(), 11);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sampling.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sampling.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.screenshot").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.storekit").ToInteger(), 4);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap").ToInteger(), 3);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap.processes").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap.system").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.sysmontap.windowed").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.ultraviolet.agent-pipe").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.ultraviolet.preview").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.ultraviolet.renderer").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.vmtracking").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.vmtracking.deferred").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.server.services.vmtracking.immediate").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.instruments.target.ios").ToInteger(), 150500);
  ASSERT_EQ(root.at("com.apple.instruments.target.logical-cpus").ToInteger(), 6);
  ASSERT_EQ(root.at("com.apple.instruments.target.mtb.denom").ToInteger(), 3);
  ASSERT_EQ(root.at("com.apple.instruments.target.mtb.numer").ToInteger(), 125);
  ASSERT_EQ(root.at("com.apple.instruments.target.physical-cpus").ToInteger(), 6);
  ASSERT_EQ(root.at("com.apple.instruments.target.user-page-size").ToInteger(), 16384);
  ASSERT_EQ(root.at("com.apple.private.DTXBlockCompression").ToInteger(), 2);
  ASSERT_EQ(root.at("com.apple.private.DTXConnection").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.Energy").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.NetworkStatistics").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.SceneKit").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.SpriteKit").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.procinfo").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.debug-gauge-data-providers.resources").ToInteger(), 1);
  ASSERT_EQ(root.at("com.apple.xcode.resource-control").ToInteger(), 1);
  // clang-format on

  nskeyedarchiver::KAValue payload =
      nskeyedarchiver::NSKeyedUnarchiver::UnarchiveTopLevelObjectWithData(buffer, payload_length);
  printf("%s\n", payload.ToJson().c_str());
  ASSERT_EQ(nskeyedarchiver::KAValue::DataType::Str, payload.GetDataType());
  ASSERT_STREQ(payload.ToStr(), "_notifyOfPublishedCapabilities:");
#endif  // ENABLE_NSKEYEDARCHIVE_TEST
}

#ifdef ENABLE_NSKEYEDARCHIVE_TEST
TEST(DTXPrimitiveArrayTest, Serialize2) {
  DTXPrimitiveArray array(true /* as dict */);

  // type=3, value=1
  array.Append(DTXPrimitiveValue((int32_t)1));

  // type=2, value=bplist, type=str
  const char* str = "com.apple.instruments.server.services.deviceinfo";
  nskeyedarchiver::KAValue kavalue = nskeyedarchiver::KAValue(str);
  printf("kavalue: %s\n", kavalue.ToJson().c_str());
  char* buffer = nullptr;
  size_t buffer_size = 0;
  nskeyedarchiver::NSKeyedArchiver::ArchivedData(
      kavalue, &buffer, &buffer_size, nskeyedarchiver::NSKeyedArchiver::OutputFormat::Binary);
  ASSERT_TRUE(buffer_size > 0);
  array.Append(DTXPrimitiveValue(buffer, buffer_size, true /* copy */));

  ByteBuffer serialized(8192);
  array.SerializeTo([&](const char* data, size_t size) -> bool {
    serialized.Append(data, size);
    return true;
  });
  idevice::hexdump(serialized.GetBuffer(0), serialized.Size(), 0);
  printf("=================================\n");

  // clang-format off
  const unsigned char expect_data[228] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
      0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | capacity=496                                        | size=212                             |
      0x0A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  //  | type=10               | type=3                | value=1               | type=10            |
      0x02, 0x00, 0x00, 0x00, 0xBC, 0x00, 0x00, 0x00, 0x62, 0x70, 0x6C, 0x69, 0x73, 0x74, 0x30, 0x30,
  //  | type=2                | value_len=188         | offset=0x28, value=bplist...
      0xD4, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0A, 0x58, 0x24, 0x76, 0x65, 0x72, 0x73, 0x69,
      0x6F, 0x6E, 0x59, 0x24, 0x61, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x72, 0x54, 0x24, 0x74, 0x6F,
      0x70, 0x58, 0x24, 0x6F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x73, 0x12, 0x00, 0x01, 0x86, 0xA0, 0x5F,
      0x10, 0x0F, 0x4E, 0x53, 0x4B, 0x65, 0x79, 0x65, 0x64, 0x41, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65,
      0x72, 0xD1, 0x08, 0x09, 0x54, 0x72, 0x6F, 0x6F, 0x74, 0x80, 0x01, 0xA2, 0x0B, 0x0C, 0x55, 0x24,
      0x6E, 0x75, 0x6C, 0x6C, 0x5F, 0x10, 0x30, 0x63, 0x6F, 0x6D, 0x2E, 0x61, 0x70, 0x70, 0x6C, 0x65,
      0x2E, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x73, 0x2E, 0x73, 0x65, 0x72,
      0x76, 0x65, 0x72, 0x2E, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x73, 0x2E, 0x64, 0x65, 0x76,
      0x69, 0x63, 0x65, 0x69, 0x6E, 0x66, 0x6F, 0x08, 0x11, 0x1A, 0x24, 0x29, 0x32, 0x37, 0x49, 0x4C,
      0x51, 0x53, 0x56, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x8F
  };
  // clang-format on
  idevice::hexdump((void*)&expect_data, sizeof(expect_data), 0);

  // check range [0, 0x28)
  ASSERT_EQ(serialized.Size(), sizeof(expect_data));
  char* acutal_ptr = reinterpret_cast<char*>(serialized.GetBuffer(0));
  constexpr size_t plist_offset = 0x28;
  for (int i = 0; i < plist_offset; ++i) {
    ASSERT_EQ((char)*(acutal_ptr + i), (char)expect_data[i]);
  }

  // check range [0x28, end]
  char* bplist_ptr = acutal_ptr + plist_offset;
  size_t bplist_size = serialized.Size() - plist_offset;
  nskeyedarchiver::KAValue serialized_value2 =
      nskeyedarchiver::NSKeyedUnarchiver::UnarchiveTopLevelObjectWithData(bplist_ptr, bplist_size);
  ASSERT_STREQ(str, serialized_value2.ToStr());
}
#endif  // ENABLE_NSKEYEDARCHIVE_TEST

TEST(DTXPrimitiveArrayTest, Serialize) {
  DTXPrimitiveArray array(true /* as dict */);

  // 0: kNull
  array.Append(DTXPrimitiveValue());

  // 1: kString
  const char* str = "hello world";
  array.Append(DTXPrimitiveValue(str, strlen(str)));

  // 2: kBuffer
  char buffer[] = {0x01, 0x02, 0x03, 0x04};
  array.Append(DTXPrimitiveValue(buffer, sizeof(buffer)));

  // 3: kSignedInt32
  array.Append(DTXPrimitiveValue((int32_t)32));

  // 4: kSignedInt64
  array.Append(DTXPrimitiveValue((int64_t)64));

  // 5: kFloat32
  array.Append(DTXPrimitiveValue((float)3.2));

  // 6: kFloat64
  array.Append(DTXPrimitiveValue((double)6.4));

  // 7: kInteger
  array.Append(DTXPrimitiveValue((uint64_t)64));

  ASSERT_EQ(8, array.Size());

  ByteBuffer serialized(8192);
  array.SerializeTo([&](const char* data, size_t size) -> bool {
    serialized.Append(data, size);
    return true;
  });
  printf("=================================\n");
  idevice::hexdump(serialized.GetBuffer(0), serialized.Size(), 0);

  // clang-format off
  const unsigned char expect_data[] = {
    0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // capacity=496
    0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size=111 = (0) + (4 + 4 + 4 + 11) + (4 + 4 + 4 + 4) + (4 + 4 + 4) + (4 + 4 + 8) + (4 + 4 + 4) + (4 + 4 + 8) + (4 + 4 + 8)
    
    // type=kNull, size=0                           (0)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x01, 0x00, 0x00, 0x00, // type=kString         (4)
    0x0B, 0x00, 0x00, 0x00, // size=11(strlen)      (4)
    0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, // data="hello world", without ending \0  (11)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x02, 0x00, 0x00, 0x00, // type=kBuffer         (4)
    0x04, 0x00, 0x00, 0x00, // size=4               (4)
    0x01, 0x02, 0x03, 0x04, // data=bytes           (4)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x03, 0x00, 0x00, 0x00, // type=kSignedInt32    (4)
    0x20, 0x00, 0x00, 0x00, // size=4, data=32      (4)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x04, 0x00, 0x00, 0x00, // type=kSignedInt64    (4)
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size=8, data=64 （8）
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x05, 0x00, 0x00, 0x00, // type=kFloat32        (4)
    0xCD, 0xCC, 0x4C, 0x40, // size=4, data=3.2     (4)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x06, 0x00, 0x00, 0x00, // type=kFloat64        (4)
    0x9A, 0x99, 0x99, 0x99, 0x99, 0x99, 0x19, 0x40, // size=8, data=6.4 (8)
    
    0x0A, 0x00, 0x00, 0x00, // type=kEmptyKey       (4)
    0x09, 0x00, 0x00, 0x00, // type=kInteger        (4)
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size=8, data=64  (8)
  };
  // clang-format on
  printf("=================================\n");
  idevice::hexdump((void*)&expect_data, sizeof(expect_data), 0);
  printf("expect size: %zu\n", sizeof(expect_data));

  ASSERT_EQ(serialized.Size(), sizeof(expect_data));
  char* acutal_ptr = reinterpret_cast<char*>(serialized.GetBuffer(0));
  for (int i = 0; i < serialized.Size(); ++i) {
    ASSERT_EQ((char)*(acutal_ptr + i), (char)expect_data[i]);
  }
}

TEST(DTXPrimitiveArrayTest, DeserializeAndSerialize) {
  char* buffer = nullptr;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("dtxmsg_notifyofpublishedcapabilities.bin");
  buffer = buffer + auxiliary_header_size;  // skip the header

  uint32_t auxiliary_length = 7667;
  std::unique_ptr<DTXPrimitiveArray> auxiliaries =
      DTXPrimitiveArray::Deserialize(buffer, auxiliary_length);

  ByteBuffer serialized(8192);
  auxiliaries->SerializeTo([&](const char* data, size_t size) -> bool {
    serialized.Append(data, size);
    return true;
  });

  ASSERT_EQ(serialized.Size(), auxiliary_length);
  printf("=================================\n");
  idevice::hexdump(serialized.GetBuffer(0), serialized.Size(), auxiliary_header_size);
}
