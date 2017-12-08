/*
 * MainReactor.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_MAINREACTOR_H_
#define SRC_NETWORK_MAINREACTOR_H_

#include <memory>

namespace network {

class Session;
class Acceptor;
class SessionManager;

class MainReactor {
 public:
  MainReactor(SessionManager* session_manager);
  virtual ~MainReactor();

  void Start();
  void Join();

  void AddDataRecvCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);

 private:
  void OnAcceptedNotify(const std::shared_ptr<Session>& session);
  void OnDataRecvNotify(const std::shared_ptr<Session>& session);

  static const int kServerPort = 8802;
  static const int kMaxListenCount = 2048;

  std::unique_ptr<Acceptor> acceptor_;
  std::vector<std::function<void(const std::shared_ptr<Session>&)>> recv_callbacks_;
};

}
/* namespace network */

#endif /* SRC_NETWORK_MAINREACTOR_H_ */
