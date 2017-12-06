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

namespace network {

class Session;
class Selector;
class SessionManager;

class Acceptor {
 public:
  Acceptor(SessionManager* session_manager);
  virtual ~Acceptor();

  void SetAcceptNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);

 private:
  void OnAcceptCallback(int sockfd, int event, void *ctx);
  void OnNotifySubReactor(const std::shared_ptr<Session>& session);

  static const int kServerPort = 8802;
  static const int kMaxListenCount = 2048;

  int listener_;
  SessionManager* session_manager_;
  std::unique_ptr<Selector> selector_;
  std::function<void(const std::shared_ptr<Session>&)> callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_ACCEPTOR_H_ */
