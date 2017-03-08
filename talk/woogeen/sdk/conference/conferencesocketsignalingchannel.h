//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#ifndef conference_ConferenceSocketSignalingChannel_h
#define conference_ConferenceSocketSignalingChannel_h

#include <memory>
#include <future>
#include <queue>
#include <random>
#include <unordered_map>
#include "talk/woogeen/include/sio_client.h"
#include "talk/woogeen/include/sio_message.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceclient.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/user.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/conferenceexception.h"

namespace woogeen {
namespace conference {

class ConferenceSocketSignalingChannel
    : public std::enable_shared_from_this<ConferenceSocketSignalingChannel> {
 public:
  explicit ConferenceSocketSignalingChannel();
  virtual ~ConferenceSocketSignalingChannel();

  virtual void AddObserver(ConferenceSocketSignalingChannelObserver& observer);
  virtual void RemoveObserver(
      ConferenceSocketSignalingChannelObserver& observer);
  virtual void Connect(
      const std::string& token,
      std::function<void(sio::message::ptr room_info)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  // Send publish or subscribe message to MCU.
  // If it publishes a stream, label should be MediaStream's label.
  // If it subscribe a stream, label should be nullptr.
  virtual void SendInitializationMessage(
      sio::message::ptr options,
      std::string publish_stream_label,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendSdp(
      sio::message::ptr message,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendStreamEvent(
      const std::string& event,
      const std::string& stream_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendCustomMessage(
      const std::string& message,
      const std::string& receiver,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SendStreamControlMessage(
      const std::string& stream_id,
      const std::string& action,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void GetRegion(
      const std::string& stream_id,
      std::function<void(std::string)> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void SetRegion(
      const std::string& stream_id,
      const std::string& region_id,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);
  virtual void Disconnect(
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 protected:
  virtual void OnEmitAck(
      sio::message::list const& msg,
      std::function<void()> on_success,
      std::function<void(std::unique_ptr<ConferenceException>)> on_failure);

 private:
  class SioMessage final {
   public:
    explicit SioMessage(
        const int id,
        const std::string& name,
        const sio::message::list& message,
        const std::function<void(sio::message::list const&)> ack,
        const std::function<void(std::unique_ptr<ConferenceException>)>
            on_failure)
        : id(id),
          name(name),
          message(message),
          ack(ack),
          on_failure(on_failure){};
    const int id;
    const std::string name;
    const sio::message::list message;
    const std::function<void(sio::message::list const&)> ack;
    const std::function<void(std::unique_ptr<ConferenceException>)> on_failure;
  };
  /// Fires upon a new ticket is received.
  void OnReconnectionTicket(const std::string& ticket);
  void RefreshReconnectionTicket();
  void TriggerOnServerDisconnected();
  void Emit(const std::string& name,
            const sio::message::list& message,
            const std::function<void(sio::message::list const&)> ack,
            const std::function<void(std::unique_ptr<ConferenceException>)>
                on_failure);
  // Clean message queue and triggered failure callback for all queued messages.
  void DropQueuedMessages();
  // Re-emit queued message.
  void DrainQueuedMessages();

  sio::client* socket_client_;
  std::vector<ConferenceSocketSignalingChannelObserver*> observers_;
  std::function<void(std::unique_ptr<ConferenceException>)>
      connect_failure_callback_;
  std::function<void()> disconnect_complete_;
  std::string reconnection_ticket_;
  int reconnection_attempted_;
  bool is_reconnection_;
  // Messages may be lost if during Socket.IO reconnection. We maintain a
  // message queue here so we can emit un-acked messages after connected.
  std::queue<SioMessage> outgoing_messages_;
  int outgoing_message_id_;
  std::mutex outgoing_message_mutex_;
};
}
}

#endif
