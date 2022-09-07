#ifndef IDEVICE_INSTRUMENT_DTXMESSAGE_H
#define IDEVICE_INSTRUMENT_DTXMESSAGE_H

#include <functional>
#include <memory>  // std::shared_ptr
#include <unordered_map>

#include "idevice/common/idevice.h"
#include "idevice/instrument/dtxprimitivearray.h"
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

/**
 * DTXMessage
 */
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

  /**
   * Constructor
   *
   * @param message_type type of this message
   */
  DTXMessage(uint32_t message_type = kSelectorMessageType) : message_type_(message_type) {}

  /**
   * Destructor
   *
   */
  virtual ~DTXMessage() {
    if (should_free_payload_buffer_ && payload_buffer_ != nullptr) {
      free(payload_buffer_);
    }
  }

  /**
   * Deserialize from bytes
   *
   * @param bytes incoming bytes
   * @param size size of bytes
   * @return ptr of new instance
   */
  static std::shared_ptr<DTXMessage> Deserialize(const char* bytes, size_t size);

  /**
   * Serialize to bytes
   *
   * @param serializer serialize function
   */
  bool SerializeTo(std::function<bool(const char*, size_t)> serializer);

  /**
   * Get size of serialized bytes
   */
  size_t SerializedLength();

  /**
   * Create a DTXMessage of selector type
   *
   * @param selector the name of the function
   * @param auxiliary parameters of the function
   */
  static std::shared_ptr<DTXMessage> CreateWithSelector(const char* selector);

  /**
   * Create a DTXMessage of buffer type
   */
  static std::shared_ptr<DTXMessage> CreateWithBuffer(const char* buffer, size_t size,
                                                      bool should_copy);

  /**
   * Create a reply DTXMessage
   */
  static std::shared_ptr<DTXMessage> NewReply(std::shared_ptr<DTXMessage> replyTo);

  /**
   * Get the type of this message
   *
   * @return uint32_t message type
   */
  uint32_t MessageType() const { return message_type_; }

  /**
   * Set the the of this Message
   *
   * @param message_type message type
   */
  void SetMessageType(uint32_t message_type) { message_type_ = message_type; }

  /**
   * Set the payload buffer
   *
   * @param buffer the payload buffer
   * @param size size of the payload buffer
   * @param should_copy should copy it or not
   */
  void SetPayloadBuffer(char* buffer, size_t size, bool should_copy);

  /**
   * Get the payload buffer
   *
   * @return char* the payload buffer
   */
  char* PayloadBuffer() const { return payload_buffer_; }

  /**
   * Get the size of the payload buffer
   *
   * @return size_t size of the payload buffer
   */
  size_t PayloadSize() const { return payload_size_; }

  /**
   * Set the payload object which will be serialized into the payload buffer when it is transmitted
   *
   * @param payload_object
   */
  void SetPayloadObject(std::unique_ptr<nskeyedarchiver::KAValue>&& payload_object) {
    payload_object_ = std::move(payload_object);
  }

  /**
   * Get the payload object
   *
   * @return const std::unique_ptr<nskeyedarchiver::KAValue>& the payload object
   */
  const std::unique_ptr<nskeyedarchiver::KAValue>& PayloadObject() const { return payload_object_; }

  /**
   * Get the auxiliary, list of arguments of the selector(function)
   *
   * @return const std::unique_ptr<DTXPrimitiveArray>& the auxiliary
   */
  const std::unique_ptr<DTXPrimitiveArray>& Auxiliary() const { return auxiliary_; }

  /**
   * Set the auxiliary, list of arguments of the selector(function)
   *
   * @param auxiliary the auxiliary
   */
  void SetAuxiliary(std::unique_ptr<DTXPrimitiveArray>&& auxiliary) {
    auxiliary_ = std::move(auxiliary);
  }

  /**
   * Append a primitive type argument to the auxiliary(list of arguments)
   *
   * @param aux primitive type argument
   */
  void AppendAuxiliary(DTXPrimitiveValue&& aux) {
    auxiliary_->Append(std::forward<DTXPrimitiveValue>(aux));
  }

  /**
   * Append a object type argument to the auxiliary(list of arguments)
   *
   * @param aux object type argument
   */
  void AppendAuxiliary(nskeyedarchiver::KAValue&& aux);

  /**
   * Set the identifier of this message
   *
   * @param identifier message identifier
   */
  void SetIdentifier(uint32_t identifier) { identifier_ = identifier; }

  /**
   * Get the identifier of this message
   *
   * @return uint32_t message identifier
   */
  uint32_t Identifier() const { return identifier_; }

  /**
   * Set the conversation index of this message
   *
   * @param conversation_index conversation index
   */
  void SetConversationIndex(uint32_t conversation_index) {
    conversation_index_ = conversation_index;
  }

  /**
   * Get the conversation index of this message
   * Conversation index always starts at 0 and it used to mark the number of messages within a
   * conversation.
   *
   * @return uint32_t conversation index
   */
  uint32_t ConversationIndex() const { return conversation_index_; }

  /**
   * Set the channel code of this message
   *
   * @param channel_code channel code
   */
  void SetChannelCode(uint32_t channel_code) { channel_code_ = channel_code; }

  /**
   * Get the channel code of this message
   *
   * @return uint32_t channel code
   */
  uint32_t ChannelCode() const { return channel_code_; }

  /**
   * Set the expects reply of this message
   * Set it when we need the server to reply to this message
   *
   * @param expects_reply
   */
  void SetExpectsReply(bool expects_reply) { expects_reply_ = expects_reply; }

  /**
   * Set the expects reply of this message
   *
   * @param expects_reply
   */
  bool ExpectsReply() const { return expects_reply_; }

  /**
   * Get the total size of message with the header
   */
  size_t CostSize() const { return cost_size_; }

  /**
   * Set the total size of message with the header
   *
   * @param cost_size
   */
  void SetCostSize(size_t cost_size) { cost_size_ = cost_size; }

  /**
   * Mark whether the message is deserialized
   *
   * @param deserialized
   */
  void SetDeserialized(bool deserialized) { deserialized_ = deserialized; }

  /**
   * Check whether it's deserialized or not
   * The messages we receive from the server are deserialized, while the messages we prepare to send
   * to the server are created directly.
   *
   * @return deserialized or not
   */
  bool Deserialized() const { return deserialized_; }

  /**
   * Dump ths message
   * Used for debugging
   *
   * @param dumphex dump the data as hex string
   */
  void Dump(bool dumphex = true) const;

 private:
  void MaybeSerializeAuxiliaryObjects();
  void MaybeSerializePayloadObject();

  std::unique_ptr<nskeyedarchiver::KAValue> payload_object_ = nullptr;
  char* payload_buffer_ = nullptr;
  bool should_free_payload_buffer_ = false;
  size_t cost_size_ = 0;
  size_t payload_size_ = 0;
  uint32_t message_type_ = kInterruptionMessage;
  std::unique_ptr<DTXPrimitiveArray> auxiliary_ = nullptr;
  std::unordered_map<size_t, nskeyedarchiver::KAValue> auxiliary_objects_;

  // DTXMessageRoutingInfo
  uint32_t identifier_ = 0;
  uint32_t conversation_index_ = 0;
  uint32_t channel_code_ = 0;
  bool expects_reply_ = false;

  bool deserialized_ = false;

};  // class DTXMessage

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_DTXMESSAGE_H
