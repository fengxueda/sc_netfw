/*
 * ServiceContext.h
 *
 *  Created on: 2017年12月7日
 *      Author: xueda
 */

#ifndef INCLUDE_SERVICECONTEXT_H_
#define INCLUDE_SERVICECONTEXT_H_

#include <mutex>
#include <memory>

namespace network {

class Session;
class DataPacket;

class ServiceContext {
 public:
  ServiceContext() {

  }
  virtual ~ServiceContext() {

  }

  const std::shared_ptr<Session>& session() const {
    return session_;
  }
  void set_session(const std::shared_ptr<Session>& session) {
    session_ = session;
  }
  const std::shared_ptr<DataPacket>& datagram() const {
    return datagram_;
  }
  void set_datagram(const std::shared_ptr<DataPacket>& datagram) {
    datagram_ = datagram;
  }

  std::mutex& mutex() const {
    return mutex_;
  }

 private:
  mutable std::mutex mutex_;
  std::shared_ptr<Session> session_;
  std::shared_ptr<DataPacket> datagram_;
};

}

#endif /* INCLUDE_SERVICECONTEXT_H_ */
