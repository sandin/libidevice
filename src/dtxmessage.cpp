#include "idevice/dtxmessage.h"

#include <string>
#include <fstream>
#include <memory> // std::make_unique

#include "idevice/macro_def.h"
#include "nskeyedarchiver/nskeyedarchiver.hpp"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"

using namespace idevice;

static const int kDTXMessagePayloadHeaderSize = 0x10;

static inline void write_buffer_to_file(std::string filename, const char* buffer, uint64_t size)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    file.write(buffer, size);
    file.close();
}

// static
std::shared_ptr<DTXMessage> DTXMessage::Create(const char* selector) {
  std::shared_ptr<DTXMessage> message = std::make_shared<DTXMessage>();
  message->SetMessageType(kNormalMessageType);
  message->SetPayloadObject(std::make_unique<nskeyedarchiver::KAValue>(selector) /* as NSString */);
  message->SetAuxiliary(std::make_unique<DTXPrimitiveArray>());
  return message;
}

// static
std::shared_ptr<DTXMessage> DTXMessage::Deserialize(const char* bytes, size_t size) {
  /* ONLY FOR DEBUG
  static int count = 0;
  count++;
  std::string filename = std::string("dtxmessage_") + std::to_string(count) + ".bin";
  write_buffer_to_file(filename, bytes, size);
  */
  
  // DTXMessagePayload Memory Layout:
  // |-----------------------------------------------------------|
  // |  0  1  2  3  |  4  5  6  7  |  8  9  A  B  |  C  D  E  F  |
  // |-----------------------------------------------------------|
  // |  msg_type    |  aux_len     |  total_len                  | // `DTXMessagePayloadHeader header`, header of DTXMessagePayload, size=0x10
  // |  auxiliary (size=aux_len)                                 | // `DTXPrimitiveArray auxiliary`, parameters of the function
  // |  ...                                                      |
  // |  payload   (size=total_len - aux_len)                     | // `NSKeyedArchiver selector`, name of the function(or return type)
  // |  ...                                                      |
  // |-----------------------------------------------------------|
  uint32_t message_type = *(uint32_t*)(bytes);
  uint32_t auxiliary_length = *(uint32_t*)(bytes + 0x04);
  uint64_t total_length = *(uint64_t*)(bytes + 0x08);
  uint64_t payload_length = total_length - auxiliary_length;
#if IDEVICE_DEBUG
  printf("message_type: %d\n", message_type);
  printf("auxiliary_length: %d\n", auxiliary_length);
  printf("total_length: %llu\n", total_length);
  printf("payload_length: %llu\n", payload_length);
#endif
  
  const char* auxiliary_ptr = bytes + kDTXMessagePayloadHeaderSize;
  const char* payload_ptr = bytes + kDTXMessagePayloadHeaderSize + auxiliary_length;
  
  std::shared_ptr<DTXMessage> message = std::make_shared<DTXMessage>(message_type);
  if (auxiliary_length > 0) {
    message->SetAuxiliary(DTXPrimitiveArray::Deserialize(bytes + kDTXMessagePayloadHeaderSize, auxiliary_length));
  }
  if (payload_length > 0) {
    message->SetPayloadBuffer(const_cast<char*>(payload_ptr), payload_length, true);
    nskeyedarchiver::KAValue value = nskeyedarchiver::NSKeyedUnarchiver::UnarchiveTopLevelObjectWithData(payload_ptr, payload_length);
    message->SetPayloadObject(std::make_unique<nskeyedarchiver::KAValue>(std::move(value)));
  }
  if (message_type == 7) {
    printf("TODO: DecompressedData(), use zlib\n"); // TODO
  }
  
  message->SetDeserialized(true);
  return message;
}

size_t DTXMessage::SerializedLength() {
  size_t length = kDTXMessagePayloadHeaderSize;
  
  MaybeSerializeAuxiliaryObjects();
  length += auxiliary_->SerializedLength();
  
  MaybeSerializePayloadObject();
  length += payload_size_;
  
  return length;
}

bool DTXMessage::SerializeTo(std::function<bool(const char*, size_t)> serializer) {
  // Serialize DTXMessagePayloadHeader
  serializer(reinterpret_cast<const char*>(&message_type_), sizeof(uint32_t));
  
  MaybeSerializeAuxiliaryObjects();
  uint32_t auxiliary_length = auxiliary_->SerializedLength();
  serializer(reinterpret_cast<const char*>(&auxiliary_length), sizeof(uint32_t));
  
  MaybeSerializePayloadObject();
  uint64_t total_length = auxiliary_length + payload_size_;
  serializer(reinterpret_cast<const char*>(&total_length), sizeof(uint64_t));
  
#if IDEVICE_DEBUG
  printf("message_type: %d\n", message_type_);
  printf("auxiliary_length: %d\n", auxiliary_length);
  printf("total_length: %llu\n", total_length);
#endif
  
  // Serialize auxiliary
  if (auxiliary_length > 0) {
    if (!auxiliary_->SerializeTo(serializer)) {
      return false;
    }
  }

  // Serialize payload
  if (payload_size_ > 0) {
    serializer(payload_buffer_, payload_size_);
  }
  return true;
}

void DTXMessage::MaybeSerializePayloadObject() {
  if (payload_buffer_ == nullptr) {
    payload_size_ = 0;
    nskeyedarchiver::NSKeyedArchiver::ArchivedData(*payload_object_, &payload_buffer_, &payload_size_,
                                                 nskeyedarchiver::NSKeyedArchiver::OutputFormat::Binary);
  }
}

void DTXMessage::MaybeSerializeAuxiliaryObjects() {
  // At first we just save objects of auxiliary in the `auxiliary_objects_` map, and place a placeholder inside the `auxiliary_` array.
  // but when we want to serialize all auxiliaries of the DTXMessage, we have to replace these placeholders with serialized bytes first.
  if (auxiliary_objects_.size() > 0) {
    for (const auto &it : auxiliary_objects_) {
      size_t index = it.first;
      const nskeyedarchiver::KAValue& object = it.second;
      char* buffer = nullptr;
      size_t buffer_size = 0;
      // serialize object to bytes
      nskeyedarchiver::NSKeyedArchiver::ArchivedData(object, &buffer, &buffer_size,
                                                     nskeyedarchiver::NSKeyedArchiver::OutputFormat::Binary);
      (*auxiliary_)[index] = DTXPrimitiveValue(buffer, buffer_size, false /* move the buffer pointer, do not copy it */);
    }
    auxiliary_objects_.clear();
  }
}

void DTXMessage::SetPayloadBuffer(char* buffer, size_t size, bool should_copy) {
  payload_size_ = size;
  
  if (should_copy) {
    payload_buffer_ = static_cast<char*>(malloc(size));
    memcpy(payload_buffer_, buffer, size);
  } else {
    payload_buffer_ = buffer;
  }
}

void DTXMessage::AppendAuxiliary(nskeyedarchiver::KAValue&& aux) {
  size_t index = auxiliary_->Size();
  auxiliary_->Append(DTXPrimitiveValue() /* as placeholder */);
  auxiliary_objects_.insert(std::make_pair(index, std::move(aux)));
}

void DTXMessage::Dump(bool dumphex) const {
  printf("==== DTXMessage ====\n");
  printf("message_type: %d\n", message_type_);
  printf("identifier: %d\n", identifier_);
  printf("conversation_index: %d\n", conversation_index_);
  printf("channel_code: %d\n", channel_code_);
  printf("expects_reply: %d\n", expects_reply_);
  printf("auxiliary:\n");
  if (auxiliary_ != nullptr) {
    auxiliary_->Dump(dumphex);
  } else {
    printf("none\n");
  }
  printf("payload(size=%zu):\n", payload_size_);
  if (dumphex && payload_buffer_ != nullptr && payload_size_ > 0) {
    hexdump(payload_buffer_, payload_size_, 0);
  }
  if (payload_object_ != nullptr) {
    printf("%s\n", payload_object_->ToJson().c_str());
  }
  printf("==== /DTXMessage ====\n");
}
