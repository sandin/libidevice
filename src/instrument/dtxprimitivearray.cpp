#include "idevice/instrument/dtxprimitivearray.h"

#include <algorithm>  // std::max
#include <cassert>    // assert

#include "idevice/common/macro_def.h"  // IDEVICE_MEM_ALIGN, IDEVICE_ASSERT

using namespace idevice;

constexpr size_t kDTXPrimitiveArrayDefaultSize = 0x200;
constexpr size_t kDTXPrimitiveArrayHeaderSize = 0x10;
constexpr size_t kDTXPrimitiveArrayCapacityAlignment =
    kDTXPrimitiveArrayDefaultSize + kDTXPrimitiveArrayHeaderSize;  // 0x210
constexpr size_t kDTXPrimitiveArrayDefaultCapacity =
    kDTXPrimitiveArrayDefaultSize - kDTXPrimitiveArrayHeaderSize;  // 0x1F0, F0 01 00 00 00 00 00 00

// static
std::unique_ptr<DTXPrimitiveArray> DTXPrimitiveArray::Deserialize(const char* buffer,
                                                                  size_t buffer_size) {
  if (!buffer || buffer_size < kDTXPrimitiveArrayHeaderSize) {
    printf("Error: DTXPrimitiveArray unexpected bytes at %p of length %zu, returning nullptr.\n",
           buffer, buffer_size);
    return nullptr;
  }
  // clang-format off
  // DTXPrimitiveArray Memory Layout:
  // |-----------------------------------------------------------|
  // |  0  1  2  3  |  4  5  6  7  |  8  9  A  B  |  C  D  E  F  |
  // |-----------------------------------------------------------|
  // |  capacity                   |  size                       | // DTXPrimitiveArrayHeader, header of DTXPrimitiveArray
  // |  item_type=1 | str_length   |  buffer(size=str_length)    | // e.g.: element of type kString(char*)
  // |  ...                                                      |
  // |  item_type=2 | buf_length   |  buffer(size=buf_length)    | // e.g.: element of type kBuffer(NSKeyedArchiver)
  // |  ...                                                      |
  // |  item_type=3 | 32bit int    |  ...                        | // e.g.: element of type kSignedInt32(int32_t)
  // |  item_type=4 | 64bit int                   | ...          | // e.g.: element of type kSignedInt64(int64_t)
  // |  item_type=5 | 32bit float  |  ...                        | // e.g.: element of type kFloat32(float)
  // |  item_type=6 | 64bit float                 | ...          | // e.g.: element of type kFloat64(double)
  // |  ...                                                      |
  // |-----------------------------------------------------------|
  // clang-format on
  uint64_t capacity =
      *(uint64_t*)(buffer);  // capacity represents how many bytes of DTXPrimitiveArray
                             // struct(include the size of the DTXPrimitiveArrayHeader)
  uint64_t size = *(uint64_t*)(buffer + 0x8);  // size represents how many bytes of items
  if (size + kDTXPrimitiveArrayHeaderSize != buffer_size) {
    printf(
        "Error: DTXPrimitiveArray unexpected bytes at %p of length %zu, returning "
        "nullptr(length=%llu).\n",
        buffer, buffer_size, size);
    return nullptr;
  }

  std::unique_ptr<DTXPrimitiveArray> array = std::make_unique<DTXPrimitiveArray>();

  char* ptr = const_cast<char*>(buffer);
  size_t offset = kDTXPrimitiveArrayHeaderSize;
  while (offset < buffer_size) {
    uint32_t type = *(uint32_t*)(ptr + offset);
    offset += sizeof(uint32_t);

    uint32_t length = 0;
    switch (type) {
      case DTXPrimitiveValue::kString: {
        // length
        length = *(uint32_t*)(ptr + offset);
        offset += sizeof(uint32_t);
        // str
        const char* str = ptr + offset;  // without ending \0
        array->Append(DTXPrimitiveValue(str, length));
        offset += length;
        break;
      }
      case DTXPrimitiveValue::kBuffer: {
        // length
        length = *(uint32_t*)(ptr + offset);
        offset += sizeof(uint32_t);
        // buffer
        array->Append(DTXPrimitiveValue(ptr + offset, static_cast<size_t>(length)));
        offset += length;
        break;
      }
      case DTXPrimitiveValue::kSignedInt32: {
        int32_t i32 = *(int32_t*)(ptr + offset);
        offset += sizeof(int32_t);
        array->Append(DTXPrimitiveValue(i32));
        break;
      }
      case DTXPrimitiveValue::kSignedInt64: {
        int64_t i64 = *(int64_t*)(ptr + offset);
        offset += sizeof(int64_t);
        array->Append(DTXPrimitiveValue(i64));
        break;
      }
      case DTXPrimitiveValue::kFloat32: {
        float f = *(float*)(ptr + offset);
        offset += sizeof(float);
        array->Append(DTXPrimitiveValue(f));
        break;
      }
      case DTXPrimitiveValue::kFloat64: {
        double d = *(double*)(ptr + offset);
        offset += sizeof(double);
        array->Append(DTXPrimitiveValue(d));
        break;
      }
      case DTXPrimitiveValue::kInteger: {
        uint64_t u = *(uint64_t*)(ptr + offset);
        offset += sizeof(uint64_t);
        array->Append(DTXPrimitiveValue(u));
        break;
      }
      case DTXPrimitiveValue::kEmptyKey: {
        break;  // empty dictionary key, the keys are empty and we ignore them
      }
      default:
        IDEVICE_ASSERT(false, "unknown type %d\n", type);
        break;
    }
  }

  return array;
}

size_t DTXPrimitiveArray::SerializedLength() const {
  if (!items_.empty()) {
    size_t length = kDTXPrimitiveArrayHeaderSize;
    for (const auto& item : items_) {
      if (item.GetType() == DTXPrimitiveValue::kNull) {
        continue;
      }

      if (as_dict_) {
        length += sizeof(uint32_t);  // size of kEmptyKey type
      }
      length += sizeof(uint32_t);  // size of type
      if (item.GetType() == DTXPrimitiveValue::kBuffer ||
          item.GetType() == DTXPrimitiveValue::kString) {
        length += sizeof(uint32_t);  // size of value payload
      }
      length += item.Size();  // size of value
    }
    return length;
  }
  return 0;
}

bool DTXPrimitiveArray::SerializeTo(std::function<bool(const char*, size_t)> serializer) {
  // Serialize DTXPrimitiveArrayHeader
  size_t size = SerializedLength() - kDTXPrimitiveArrayHeaderSize;
  uint64_t capacity = IDEVICE_MEM_ALIGN(std::max(kDTXPrimitiveArrayDefaultCapacity, size),
                                        kDTXPrimitiveArrayCapacityAlignment);
  serializer(reinterpret_cast<const char*>(&capacity), sizeof(uint64_t));
  serializer(reinterpret_cast<const char*>(&size), sizeof(uint64_t));

  const uint32_t empty_key_type = DTXPrimitiveValue::kEmptyKey;
  // Serialize items
  for (auto& item : items_) {
    uint32_t type = item.GetType();
    if (type == DTXPrimitiveValue::kNull) {
      continue;
    }

    if (as_dict_) {
      // insert an empty key for DTXPrimitiveDictionary
      serializer(reinterpret_cast<const char*>(&empty_key_type), sizeof(uint32_t));
    }

    serializer(reinterpret_cast<const char*>(&type), sizeof(uint32_t));
    if (item.Size() == 0) {
      continue;
    }

    switch (type) {
      case DTXPrimitiveValue::kString: {
        // length
        uint32_t length = item.Size();
        serializer(reinterpret_cast<const char*>(&length), sizeof(uint32_t));
        // str
        const char* ptr = item.ToStr();
        serializer(ptr, length);
        break;
      }
      case DTXPrimitiveValue::kBuffer: {
        // length
        uint32_t length = item.Size();
        serializer(reinterpret_cast<const char*>(&length), sizeof(uint32_t));
        // buffer
        const char* ptr = item.ToBuffer();
        serializer(ptr, length);
        break;
      }
      case DTXPrimitiveValue::kSignedInt32:
      case DTXPrimitiveValue::kSignedInt64:
      case DTXPrimitiveValue::kFloat32:
      case DTXPrimitiveValue::kFloat64:
      case DTXPrimitiveValue::kInteger: {
        serializer(reinterpret_cast<const char*>(item.RawData()), item.Size());
        break;
      }
      case DTXPrimitiveValue::kEmptyKey: {
        break;  // empty dictionary key, the keys are empty and we ignore them
      }
      default:
        IDEVICE_ASSERT(false, "unknown type %d\n", type);
        break;
    }
  }
  return true;
}

void DTXPrimitiveArray::Dump(bool dumphex) const {
  printf("DTXPrimitiveArray, size=%zu: \n", Size());
  size_t i = 0;
  for (const auto& item : items_) {
    printf("\titem #%zu: ", i);
    item.Dump(dumphex);
    i++;
  }
}
