#include "idevice/dtxprimitivearray.h"

#include <cassert> // assert

using namespace idevice;

static const int kDTXPrimitiveArrayHeaderSize = 0x10;
static const int kDTXPrimitiveArrayMagic = 0x1DF0; // TODO: 0x1f0?

// static
std::unique_ptr<DTXPrimitiveArray> DTXPrimitiveArray::Deserialize(const char* buffer, size_t size) {
  if (!buffer || size < kDTXPrimitiveArrayHeaderSize) {
    printf("Error: DTXPrimitiveArray unexpected bytes at %p of length %zu, returning nullptr.\n", buffer, size);
    return nullptr;
  }
  
  // DTXPrimitiveArray Memory Layout:
  // |-----------------------------------------------------------|
  // |  0  1  2  3  |  4  5  6  7  |  8  9  A  B  |  C  D  E  F  |
  // |-----------------------------------------------------------|
  // |  magic                      |  length                     | // DTXPrimitiveArrayHeader, header of DTXPrimitiveArray
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
  uint64_t magic = *(uint64_t*)(buffer);
  uint64_t length = *(uint64_t*)(buffer + 0x8);
  if (magic != kDTXPrimitiveArrayMagic || length + kDTXPrimitiveArrayHeaderSize != size) {
    printf("Error: DTXPrimitiveArray unexpected bytes at %p of length %zu, returning nullptr.\n", buffer, size);
    return nullptr;
  }
  
  std::unique_ptr<DTXPrimitiveArray> array = std::make_unique<DTXPrimitiveArray>();
  
  char* ptr = const_cast<char*>(buffer);
  size_t offset = kDTXPrimitiveArrayHeaderSize;
  while (offset < size) {
    uint32_t type = *(uint32_t*)(ptr + offset);
    offset += sizeof(uint32_t);
    
    uint32_t length = 0;
    switch (type) {
      case DTXPrimitiveValue::kString: {
        // length
        length = *(uint32_t*)(ptr + offset);
        offset += sizeof(uint32_t);
        // str
        const char* str = ptr + offset; // without ending \0
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
      case 10: {
        break; // dictionary key. for arrays, the keys are empty and we ignore them
      }
      default:
        assert(false); // TODO
        break;
    }
  }
  
  return array;
}

char* DTXPrimitiveArray::Serialize(size_t* size) {
  return nullptr; // TODO
}
