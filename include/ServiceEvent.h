/*
 * ServiceEvent.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef INCLUDE_SERVICEEVENT_H_
#define INCLUDE_SERVICEEVENT_H_

#include <memory>
#include <mutex>

namespace network {

class DataPacket;
class Session;

class ServiceEvent {
 public:
  const std::shared_ptr<DataPacket>& datagram() const {
    return datagram_;
  }
  void set_datagram(const std::shared_ptr<DataPacket>& datagram) {
    datagram_ = datagram;
  }

  const std::shared_ptr<Session>& session() const {
    return session_;
  }
  void set_session(std::shared_ptr<Session>& session) {
    session_ = session;
  }

  std::mutex& mutex() const {
    return mutex_;
  }

 private:
  mutable std::mutex mutex_;
  std::shared_ptr<DataPacket> datagram_;
  std::shared_ptr<Session> session_;
};

}

#endif /* INCLUDE_SERVICEEVENT_H_ */
