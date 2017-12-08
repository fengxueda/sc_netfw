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

  void Start();
  void Join();
  void SetAcceptedNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);
  void SetDataRecvCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);

 private:
  void OnAcceptedCallback(int sockfd, int event, void *ctx);
  void OnDataRecvCallback(const std::shared_ptr<Session>& session, int event,
                          void *ctx);

  int listener_;
  std::function<void(const std::shared_ptr<Session>&)> accept_callback_;
  std::function<void(const std::shared_ptr<Session>&)> recv_callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_ACCEPTOR_H_ */
