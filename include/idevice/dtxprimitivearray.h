#ifndef IDEVICE_DTXPRIMITIVE_ARRAY_H
#define IDEVICE_DTXPRIMITIVE_ARRAY_H

#include <cstdint> // int32_t, int64_t, uint64_t
#include <cstdlib> // malloc, free
#include <cstring> // memcpy
#include <string>
#include <vector>
#include <memory> // std::unique_ptr

#include "idevice/idevice.h" // hexdump
#include "idevice/macro_def.h" // IDEVICE_DISALLOW_COPY_AND_ASSIGN
#include "nskeyedarchiver/nskeyedunarchiver.hpp"

namespace idevice {

class DTXPrimitiveValue {
 public:
  // we map each DTXPrimitiveType to the STL type
  enum Type {
    kNull = 0,
    kString = 1,       // (const)char*, size=n
    kBuffer = 2,       // char*, size=n
    kSignedInt32 = 3,  // int32_t, size=4
    kSignedInt64 = 4,  // int64_t, size=8
    kFloat32 = 5,      // float, size=4
    kFloat64 = 6,      // double, size=8
    kInteger = 9,      // uint64_t, size=8
    kEmptyKey = 10,    // empty key in dict, size=0
    kMaxType = 11
  };

#define DTXPRIMITIVEVALUE_MOVE_VALUE(other) \
  t_ = other.t_; \
  s_ = other.s_; \
  switch (other.t_) { \
    case kNull: \
    case kEmptyKey: \
      break; \
    case kString: \
    case kBuffer: \
      d_.b = other.d_.b; \
      break; \
    case kSignedInt32: \
      d_.i32 = other.d_.i32; \
      break; \
    case kSignedInt64: \
      d_.i64 = other.d_.i64; \
      break; \
    case kFloat32: \
      d_.f = other.d_.f; \
      break; \
    case kFloat64: \
      d_.d = other.d_.d; \
      break; \
    case kInteger: \
      d_.u = other.d_.u; \
    default: \
      break; \
  } \
  
  DTXPrimitiveValue() : t_(kNull), s_(0) {} // null
  ~DTXPrimitiveValue() {
    if (t_ == kString || t_ == kBuffer) {
      if (d_.b) {
        free(d_.b);
      }
    }
  }
  
  IDEVICE_DISALLOW_COPY_AND_ASSIGN(DTXPrimitiveValue);
  
  // allow move and assign
  DTXPrimitiveValue(DTXPrimitiveValue&& other) : t_(other.t_) {
    DTXPRIMITIVEVALUE_MOVE_VALUE(other);
    other.t_ = kNull;
  }
  DTXPrimitiveValue& operator=(DTXPrimitiveValue&& other) {
    DTXPRIMITIVEVALUE_MOVE_VALUE(other);
    other.t_ = kNull;
    return *this;
  }
  
  explicit DTXPrimitiveValue(const char* str, size_t str_len) : t_(kString), s_(str_len) {
    d_.b = static_cast<char*>(malloc(str_len + 1));
    strncpy(d_.b, str, str_len);
    d_.b[str_len] = '\0';
  }
  DTXPrimitiveValue(char* buffer, size_t size, bool should_copy = true) : t_(kBuffer), s_(size) {
    if (should_copy) {
      d_.b = static_cast<char*>(malloc(size));
      memcpy(d_.b, buffer, size);
    } else {
      d_.b = buffer;
    }
  }
  explicit DTXPrimitiveValue(int32_t i32) : t_(kSignedInt32), s_(sizeof(int32_t)) {
    d_.i32 = i32;
  }
  explicit DTXPrimitiveValue(int64_t i64) : t_(kSignedInt64), s_(sizeof(int64_t)) {
    d_.i64 = i64;
  }
  explicit DTXPrimitiveValue(float f) : t_(kFloat32), s_(sizeof(float)) {
    d_.f = f;
  }
  explicit DTXPrimitiveValue(double d) : t_(kFloat64), s_(sizeof(double)) {
    d_.d = d;
  }
  explicit DTXPrimitiveValue(uint64_t u) : t_(kInteger), s_(sizeof(uint64_t)) {
    d_.u = u;
  }
  
  static DTXPrimitiveValue CreateEmptyDictionaryKey() {
    DTXPrimitiveValue value;
    value.SetType(kEmptyKey);
    value.SetSize(0);
    return value;
  }
  
  char* ToStr() { return d_.b; }
  char* ToBuffer() { return d_.b; }
  int32_t ToSignedInt32() { return d_.i32; }
  int64_t ToSignedInt64() { return d_.i64; }
  float ToFloat32() { return d_.f; }
  double ToFloat64() { return d_.d; }
  uint64_t ToInteger() { return d_.u; }
  
  const void* RawData() { return reinterpret_cast<const void*>(&d_.b); }
  
  size_t Size() const { return s_; }
  void SetSize(size_t s) { s_ = s; }
  
  Type GetType() const { return t_; }
  void SetType(Type t) { t_ = t; }
  
  void Dump(bool dumphex = true) const {
    switch (t_) {
      case kNull:
        printf("[type=kNull, size=0, value=]\n");
      case kEmptyKey:
        printf("[type=kEmptyKey, size=0, value=]\n");
        break;
      case kString: {
        size_t str_len = Size();
        char* str = static_cast<char*>(malloc(str_len + 1));
        strncpy(str, d_.b, str_len);
        str[str_len] = '\0';
        printf("[type=kString, size=%zu, value=%s]\n", Size(), str);
        free(str);
        break;
      }
      case kBuffer: {
        nskeyedarchiver::KAValue value = nskeyedarchiver::NSKeyedUnarchiver::UnarchiveTopLevelObjectWithData(d_.b, Size());
        printf("[type=kBuffer, size=%zu, value=%s]\n", Size(), value.ToJson().c_str());
        if (dumphex) {
          hexdump(d_.b, Size(), 0);
        }
        break;
      }
      case kSignedInt32:
        printf("[type=kSignedInt32, size=%zu, value=%d]\n", Size(), d_.i32);
        break;
      case kSignedInt64:
        printf("[type=kSignedInt64, size=%zu, value=%lld]\n", Size(), d_.i64);
        break;
      case kFloat32:
        printf("[type=kFloat32, size=%zu, value=%f]\n", Size(), d_.f);
        break;
      case kFloat64:
        printf("[type=kFloat64, size=%zu, value=%f]\n", Size(), d_.d);
        break;
      case kInteger:
        printf("[type=kSignedInt64, size=%zu, value=%llu]\n", Size(), d_.u);
      default:
        break;
    }
  }
  
 private:
  union {
    char* b;  // kString or kBuffer
    int32_t i32;
    int64_t i64;
    float f;
    double d;
    uint64_t u;
  } d_;
  size_t s_ = 0;
  Type t_ = kNull;
  
#undef DTXPRIMITIVEVALUE_MOVE_VALUE
};  // class DTXPrimitiveValue

/**
 * DTXPrimitiveArray
 */
class DTXPrimitiveArray {
 public:
  DTXPrimitiveArray(bool as_dict = true) : as_dict_(as_dict) {}
  ~DTXPrimitiveArray() {}
  
  static std::unique_ptr<DTXPrimitiveArray> Deserialize(const char* buffer, size_t size);
  
  size_t SerializedLength() const;
  bool SerializeTo(std::function<bool(const char*, size_t)> serializer);
  
  void Append(DTXPrimitiveValue&& item) {
    items_.emplace_back(std::forward<DTXPrimitiveValue>(item));
  }
  
  DTXPrimitiveValue& At(size_t index) {
    return items_.at(index);
  }
  
  DTXPrimitiveValue& operator[](size_t index) {
    return items_[index];
  }
  
  size_t Size() const { return items_.size(); }
  
  void Dump(bool dumphex = true) const;
  
 private:
  std::vector<DTXPrimitiveValue> items_;
  bool as_dict_ = false;
};  // class DTXPrimitiveArray

// Actually we didn't implement the DTXPrimitiveDictionary, but implement the DTXPrimitiveArray instead
// and use it as the DTXPrimitiveDictionary, 'cause they both have very similar memory layouts.
// for now all DTXPrimitiveDictionary we've seen only have empty keys, so we just ignore them.
//
// DTXPrimitiveDictionary Memory layout:
// |-----------------------------------------|
// | header | key | value | key | value | ...
// |-----------------------------------------|
//
// DTXPrimitiveArray Memory layout:
// |------------------------------|
// | header | value | value | ...
// |------------------------------|
//
// key/value:
// |type|payload|
using DTXPrimitiveDictionary = DTXPrimitiveArray;

}  // namespace idevice

#include "idevice/macro_undef.h"

#endif  // IDEVICE_DTXPRIMITIVE_ARRAY_H
