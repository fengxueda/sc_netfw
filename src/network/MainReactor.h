/*
 * MainReactor.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_MAINREACTOR_H_
#define SRC_NETWORK_MAINREACTOR_H_

#include <memory>
#include "Reactor.h"

namespace network {

class Session;
class Acceptor;
class SessionManager;
class ServiceMessage;

class MainReactor : public Reactor<ServiceMessage> {
 public:
  MainReactor(SessionManager* session_manager);
  virtual ~MainReactor();

  void Start();
  void Join();

  void SetNotifyCallback(
      const std::function<
          void(const std::shared_ptr<Session>&,
               const std::shared_ptr<ServiceMessage>&)>& callback);
  void SetNotifiedCallback(
      const std::function<
          void(const std::shared_ptr<Session>&,
               const std::shared_ptr<ServiceMessage>&)>& callback);
  void OnNotified(const std::shared_ptr<Session>& session,
                  const std::shared_ptr<ServiceMessage>& ctx);

 private:
  void OnNotify(const std::shared_ptr<Session>& session,
                const std::shared_ptr<ServiceMessage>& ctx);
  void OnAcceptedNotify(const std::shared_ptr<Session>& session);

  static const int kServerPort = 8802;
  static const int kMaxListenCount = 2048;

  std::unique_ptr<Acceptor> acceptor_;
  std::function<
      void(const std::shared_ptr<Session>&,
           const std::shared_ptr<ServiceMessage>&)> notify_callback_;
  std::function<
      void(const std::shared_ptr<Session>&,
           const std::shared_ptr<ServiceMessage>&)> notified_callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_MAINREACTOR_H_ */
