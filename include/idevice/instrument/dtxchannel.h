#ifndef IDEVICE_INSTRUMENT_DTXCHANNEL_H
#define IDEVICE_INSTRUMENT_DTXCHANNEL_H

#include <cstdint>  // uint32_t
#include <string>

#include "idevice/instrument/dtxmessenger.h"

namespace idevice {

/**
 * A channel of the Instrument service.
 *
 * The Instrument service on the phone provides a number of channels, each channel is responsible
 * for providing different data, For example: "com.apple.instruments.server.services.deviceinfo"
 * channel is responsible for providing information about device, and
 *  "com.apple.instruments.server.services.networking" is responsible for providing information
 * about networking, and so on.
 *
 * In order for the client and server to identify a channel, we need to specify a code to identify
 * it when we open a channel. When the channel is opened, the client and server can communicate
 * individually on this channel to interact with each other, and when the client does not need this
 * channel, it needs to be closed manually.
 */
class DTXChannel /* : public DTXMessenger */ {
 public:
  /**
   * Constructor
   * 
   * @param connection the connection
   * @param label channel label, e.g.: "com.apple.instruments.server.services.deviceinfo"
   * @param channel_identifier the channel code to identify this channel
   */
  DTXChannel(DTXMessenger* connection, const std::string& label, uint32_t channel_identifier)
      : connection_(connection), label_(label), channel_identifier_(channel_identifier) {}
  DTXChannel() : DTXChannel(nullptr, "", 0) {}

  /**
   * Destructor
   */
  virtual ~DTXChannel() {}

  /**
   * Send message synchronously
   *
   * @param msg message to be sent
   * @param timeout_ms timeout in milliseconds
   * @return std::shared_ptr<DTXMessage> the response message
   */
  std::shared_ptr<DTXMessage> SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                              uint32_t timeout_ms = -1);

  /**
   * Send message asynchronously
   *
   * @param msg message to be sent
   * @param callback callback for response message
   */
  void SendMessageAsync(std::shared_ptr<DTXMessage> msg, DTXMessenger::ReplyHandler callback);

  /**
   * Get the handler for response messages of this channel.
   * When the message does not have a specific handler, it is routed to the channel's handler.
   * 
   * @return const DTXMessenger::ReplyHandler& the handler
   */
  const DTXMessenger::ReplyHandler& MessageHandler() const { return message_handler_; };

  /**
   * Set the handler for response messages of this channel.
   * 
   * @param handler the handler
   */
  void SetMessageHandler(DTXMessenger::ReplyHandler&& handler) {
    message_handler_ = std::move(handler);
  };
 
  /**
   * Cancel this channel. 
   * It internally sends a message to the service to cancel the channel and blocks until the service returns a response.
   */
  void Cancel();

  /**
   * Check whether it's canceled or not 
   * 
   * @return canceled or not 
   */
  bool IsCanceled() const { return canceled_; }

  /**
   * Get the label of this channel 
   * 
   * @return const std::string& the label
   */
  const std::string& Label() const { return label_; }

  /**
   * Get the code of this channel 
   * 
   * @return uint32_t the channel code
   */
  uint32_t ChannelIdentifier() const { return channel_identifier_; }

 private:
  bool canceled_ = false;
  std::string label_ = "";
  uint32_t channel_identifier_ = 0;
  DTXMessenger* connection_ = nullptr;
  DTXMessenger::ReplyHandler message_handler_;

};  // class DTXChannel

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_DTXCHANNEL_H
