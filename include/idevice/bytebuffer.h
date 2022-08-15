#ifndef IDEVICE_BYTE_BUFFER_H
#define IDEVICE_BYTE_BUFFER_H

#include <cstdint>
#include <vector>
#include <memory> // std::unique_ptr

namespace idevice {

#define IDEVICE_MEM_ALIGN(v, a) (((v) + (a)-1) & ~((a)-1))

class BufferMemory {
 public:
  BufferMemory() {
  }
  ~BufferMemory() {
    if (buffer_) {
      free(buffer_);
    }
  }
  
  char* Allocate(size_t req_size) {
    if (capacity_ - size_ < req_size) {
      if (!Reserve(size_ + req_size)) {
        printf("Error: can not allocate memory, size=%zu\n", req_size);
        return nullptr;
      }
    }
    
    char* ptr = buffer_ + size_;
    size_ += req_size;
    return ptr;
  }
  
  char* GetPtr(size_t offset) {
    return buffer_ + offset;
  }
  
  size_t Size() const { return size_; }
  void SetSize(size_t size) { size_ = size; }
  
 private:
  bool Reserve(size_t capacity) {
    size_t new_capacity = IDEVICE_MEM_ALIGN(capacity, 128);
    if (buffer_ == nullptr) {
      buffer_ = static_cast<char*>(malloc(new_capacity));
    } else {
      buffer_ = static_cast<char*>(realloc(buffer_, new_capacity));
    }
    if (buffer_ != nullptr) {
      capacity_ = new_capacity;
      return true;
    }
    return false;
  }
  
  char* buffer_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
};

/**
 * Bytes buffer
 */
class ByteBuffer {
 public:
  using DataType = uint8_t;
  using DataArray = std::vector<DataType>;
  
  /**
   * constructor
   * @param reserve_size reserve size
   */
  ByteBuffer(size_t reserve_size) : buffer_(std::make_unique<DataArray>()) {
    buffer_->reserve(reserve_size);
  };
  ~ByteBuffer() {}
  
  // disallow copy and assign
  ByteBuffer(const ByteBuffer&) = delete;
  void operator=(const ByteBuffer&) = delete;
  
  // allow move and assign
  ByteBuffer(ByteBuffer&& other) : buffer_(std::move(other.buffer_)) {}
  ByteBuffer& operator=(ByteBuffer&& other) {
    buffer_ = std::unique_ptr<DataArray>(std::move(other.buffer_));
    return *this;
  }
  
  /**
   * get a mutable pointer to the buffer
   * @param offset offset from head
   * @return the pointer to the buffer
   */
  void* GetBuffer(size_t offset) {
    return reinterpret_cast<uint8_t*>(buffer_->data()) + offset;
  }
  
  /**
   * get unmutable pointer to the buffer
   * @param offset offset from head
   * @return the pointer to the buffer
   */
  const void* GetReadOnlyBuffer(size_t offset) {
    return reinterpret_cast<const uint8_t*>(buffer_->data()) + offset;
  }
  
  /**
   * read(copy) some data from the buffer
   * @param buffer the dest buffer
   * @param size_to_read size of the dest buffer
   * @param offset offset to start reading
   * @param size_read out param, how many bytes are written to the dest buffer
   * @return return true if the read is successful
   */
  bool Read(void* buffer, size_t size_to_read, size_t offset, size_t* size_read) {
    size_t buf_size = Size();
    if (offset < buf_size) {
      size_t remaining = buf_size - offset;
      if (size_to_read >= remaining) {
        size_to_read = remaining;
      }
      const void* src_buffer = GetReadOnlyBuffer(offset);
      if (!src_buffer) {
        return false;
      }
      memcpy(buffer, src_buffer, size_to_read);
    } else {
      size_to_read = 0;
    }
    if (size_read) {
      *size_read = size_to_read;
    }
    return true;
  }
  
  /**
   * write(copy) some data to the specified offset position of the buffer
   * @param buffer the src buffer
   * @param size_to_write  size of the src buffer
   * @param offset offset to start writing
   * @param size_read out param, how many bytes are written to the dest buffer
   * @return return true if the write is successful
   */
  bool Write(const void* buffer, size_t size_to_write, size_t offset, size_t* size_written) {
    if (size_to_write) {
      size_t new_size = offset + size_to_write;
      if (Size() < new_size && !Resize(new_size)) {
        return false;
      }
      void* ptr = GetBuffer(offset);
      if (!ptr) {
        return false;
      }
      memcpy(ptr, buffer, size_to_write);
      if (size_written) {
        *size_written = size_to_write;
      }
    }
    return true;
  }
  
  /**
   * write(copy) some data to the end of the buffer
   * @param buffer the src buffer
   * @param size_to_write  size of the src buffer
   * @return return true if all data in the src buffer will be written successfully
   */
  bool Append(const void* buffer, size_t buffer_size) {
    size_t size_written;
    return Write(buffer, buffer_size, Size(), &size_written) && buffer_size == size_written;
  }
  
  /**
   * resize the buffer
   * @param new_size the new size
   * @return return true if successful
   */
  bool Resize(size_t new_size) {
    buffer_->resize(new_size);
    return true;
  }
  
  /**
   * get the current size of buffer
   * @return the size
   */
  size_t Size() const {
    return buffer_->size();
  }

 private:
  uint32_t alignment_;
  std::unique_ptr<DataArray> buffer_;
}; // class ByteBuffer

}  // namespace idevice

#endif // IDEVICE_BYTE_BUFFER_H
