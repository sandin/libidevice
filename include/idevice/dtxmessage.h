#ifndef IDEVICE_DTXMESSAGE_H
#define IDEVICE_DTXMESSAGE_H

#include <functional>
#include <memory>  // std::shared_ptr
#include <unordered_map>

#include "idevice/dtxprimitivearray.h"
#include "idevice/idevice.h"
#include "nskeyedarchiver/kavalue.hpp"

namespace idevice {

struct DTXMessageHeader {
  uint32_t magic;                // +0x00, len=4
  uint32_t message_header_size;  // +0x04, len=4
  uint16_t fragment_index;       // +0x08, len=2
  uint16_t fragment_count;       // +0x0a, len=2
  uint32_t length;               // +0x0c, len=4
  uint32_t identifier;           // +0x10, len=4
  uint32_t conversation_index;   // +0x14, len=4
  uint32_t channel_code;         // +0x18, len=4
  uint32_t expects_reply;        // +0x1c, len=4
};
constexpr uint32_t kDTXMessageHeaderMagic = 0x1F3D5B79;
constexpr uint32_t kDTXMessageHeaderSize = sizeof(DTXMessageHeader);

struct DTXMessageRoutingInfo {
  uint32_t msg_identifier;
  uint32_t conversation_index;
  uint32_t channel_code;
  uint32_t expects_reply;
};

class DTXMessage {
 public:
  enum MessageType {
    kInterruptionMessage = 0,
    kDataMessageType = 1,
    kSelectorMessageType = 2,
    kUnknownMessageType = 3,  //  TODO
    kErrorMessageType = 4,
    kBarrierMessageType = 5,
    kPrimitiveMessage = 6,
    kCompressedMessageType = 7,
    kNestedMessageType = 8,
  };

  DTXMessage(uint32_t message_type = kSelectorMessageType) : message_type_(message_type) {}
  virtual ~DTXMessage() {
    if (should_free_payload_buffer_ && payload_buffer_ != nullptr) {
      free(payload_buffer_);
    }
  }

  /**
   * Deserialize from bytes
   * @param bytes incoming bytes
   * @param size size of bytes
   * @return ptr of new instance
   */
  static std::shared_ptr<DTXMessage> Deserialize(const char* bytes, size_t size);

  /**
   * Serialize to bytes
   * @param serializer serialize function
   */
  bool SerializeTo(std::function<bool(const char*, size_t)> serializer);

  /**
   * Get size of serialized bytes
   */
  size_t SerializedLength();

  /**
   * Create a DTXMessage of selector type
   * @param selector the name of the function
   * @param auxiliary parameters of the function
   */
  static std::shared_ptr<DTXMessage> CreateWithSelector(const char* selector);

  /**
   * Create a DTXMessage of buffer type
   */
  static std::shared_ptr<DTXMessage> CreateWithBuffer(const char* buffer, size_t size,
                                                      bool should_copy);

  uint32_t MessageType() const { return message_type_; }
  void SetMessageType(uint32_t message_type) { message_type_ = message_type; }

  void SetPayloadBuffer(char* buffer, size_t size, bool should_copy);
  char* PayloadBuffer() const { return payload_buffer_; }
  size_t PayloadSize() const { return payload_size_; }

  void SetPayloadObject(std::unique_ptr<nskeyedarchiver::KAValue>&& payload_object) {
    payload_object_ = std::move(payload_object);
  }
  const std::unique_ptr<nskeyedarchiver::KAValue>& PayloadObject() const { return payload_object_; }

  const std::unique_ptr<DTXPrimitiveArray>& Auxiliary() const { return auxiliary_; }
  void SetAuxiliary(std::unique_ptr<DTXPrimitiveArray>&& auxiliary) {
    auxiliary_ = std::move(auxiliary);
  }
  void AppendAuxiliary(DTXPrimitiveValue&& aux) {
    auxiliary_->Append(std::forward<DTXPrimitiveValue>(aux));
  }
  void AppendAuxiliary(nskeyedarchiver::KAValue&& aux);

  void SetIdentifier(uint32_t identifier) { identifier_ = identifier; }
  uint32_t Identifier() const { return identifier_; }

  void SetConversationIndex(uint32_t conversation_index) {
    conversation_index_ = conversation_index;
  }
  uint32_t ConversationIndex() const { return conversation_index_; }

  void SetChannelCode(uint32_t channel_code) { channel_code_ = channel_code; }
  uint32_t ChannelCode() const { return channel_code_; }

  void SetExpectsReply(bool expects_reply) { expects_reply_ = expects_reply; }
  bool ExpectsReply() const { return expects_reply_; }

  /**
   * Get size of message with the header
   */
  size_t CostSize() const { return cost_size_; }
  void SetCostSize(size_t cost_size) { cost_size_ = cost_size; }

  void SetDeserialized(bool deserialized) { deserialized_ = deserialized; }
  bool Deserialized() const { return deserialized_; }

  void Dump(bool dumphex = true) const;

 private:
  void MaybeSerializeAuxiliaryObjects();
  void MaybeSerializePayloadObject();

  std::unique_ptr<nskeyedarchiver::KAValue> payload_object_ = nullptr;
  char* payload_buffer_ = nullptr;
  bool should_free_payload_buffer_ = false;
  size_t cost_size_ = 0;
  size_t payload_size_ = 0;
  uint32_t message_type_ = 0;
  std::unique_ptr<DTXPrimitiveArray> auxiliary_ = nullptr;
  std::unordered_map<size_t, nskeyedarchiver::KAValue> auxiliary_objects_;

  // DTXMessageRoutingInfo
  uint32_t identifier_ = 0;
  uint32_t conversation_index_ = 0;
  uint32_t channel_code_ = 0;
  bool expects_reply_ = true;

  bool deserialized_ = false;

};  // class DTXMessage

}  // namespace idevice

#endif  // IDEVICE_DTXMESSAGE_H
