/*
 * SessionImpl.h
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSIONIMPL_H_
#define SRC_NETWORK_SESSIONIMPL_H_

#include <memory>
#include "Session.h"

struct bufferevent;

namespace network {

class SessionImpl : public Session {
 public:
  SessionImpl();
  virtual ~SessionImpl();

  void SetBufferEvent(const struct bufferevent* buffer_event);
  void SendMessage(unsigned char *data, int size);

 private:
  struct bufferevent* buffer_event_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SESSIONIMPL_H_ */
