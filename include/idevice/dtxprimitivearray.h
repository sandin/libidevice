#ifndef IDEVICE_DTXPRIMITIVE_ARRAY_H
#define IDEVICE_DTXPRIMITIVE_ARRAY_H

#include <cstdint> // int32_t, int64_t, uint64_t
#include <cstdlib> // malloc, free
#include <cstring> // memcpy
#include <vector>
#include <memory> // std::unique_ptr

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
    kMaxType = 10
  };

#define MOVE_DTXPRIMITIVE_VALUE(other) \
  t_ = other.t_; \
  s_ = other.s_; \
  switch (other.t_) { \
    case kNull: \
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
  
  // disallow copy and assign
  DTXPrimitiveValue(const DTXPrimitiveValue&) = delete;
  void operator=(const DTXPrimitiveValue&) = delete;
  
  // allow move and assign
  DTXPrimitiveValue(DTXPrimitiveValue&& other) : t_(other.t_) {
    MOVE_DTXPRIMITIVE_VALUE(other);
    other.t_ = kNull;
  }
  DTXPrimitiveValue& operator=(DTXPrimitiveValue&& other) {
    MOVE_DTXPRIMITIVE_VALUE(other);
    other.t_ = kNull;
    return *this;
  }
  
  explicit DTXPrimitiveValue(const char* str, size_t str_len) : t_(kString) {
    s_ = str_len + 1;
    d_.b = static_cast<char*>(malloc(s_));
    strncpy(d_.b, str, s_);
    d_.b[str_len] = '\0';
  }
  DTXPrimitiveValue(char* buffer, size_t size) : t_(kBuffer), s_(size) {
    d_.b = static_cast<char*>(malloc(size));
    memcpy(d_.b, buffer, size);
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
  
  char* ToStr() { return d_.b; }
  char* ToBuffer() { return d_.b; }
  int32_t ToSignedInt32() { return d_.i32; }
  int64_t ToSignedInt64() { return d_.i64; }
  float ToFloat32() { return d_.f; }
  double ToFloat64() { return d_.d; }
  uint64_t ToInteger() { return d_.u; }
  
  size_t Size() const { return s_; }
  Type GetType() const { return t_; }
  
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
  
#ifdef MOVE_DTXPRIMITIVE_VALUE
#undef MOVE_DTXPRIMITIVE_VALUE
#endif
};  // class DTXPrimitiveValue

class DTXPrimitiveArray {
 public:
  DTXPrimitiveArray() {}
  ~DTXPrimitiveArray() {}
  
  static std::unique_ptr<DTXPrimitiveArray> Deserialize(const char* buffer, size_t size);
  char* Serialize(size_t* size);
  
  void Append(DTXPrimitiveValue&& item) {
    items_.emplace_back(std::forward<DTXPrimitiveValue>(item));
  }
  
  DTXPrimitiveValue& At(size_t index) {
    return items_.at(index);
  }
  
  size_t Size() const { return items_.size(); }
  
 private:
  std::vector<DTXPrimitiveValue> items_;
};  // class DTXPrimitiveArray

}  // namespace idevice

#endif  // IDEVICE_DTXPRIMITIVE_ARRAY_H
