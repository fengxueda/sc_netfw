/*
 * Acceptor.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_ACCEPTOR_H_
#define SRC_NETWORK_ACCEPTOR_H_

#include <memory>
#include <functional>
#include "Selector.h"

namespace network {

class Session;
class SessionManager;

class Acceptor : private Selector {
 public:
  Acceptor(SessionManager* session_manager, unsigned short port,
           int listen_count);
  virtual ~Acceptor();

  void SetAcceptedNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);

 private:
  void OnAcceptedCallback(int sockfd, int event, void *ctx);

  int listener_;
  std::function<void(const std::shared_ptr<Session>&)> callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_ACCEPTOR_H_ */
