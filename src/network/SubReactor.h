/*
 * SubReactor.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SUBREACTOR_H_
#define SRC_NETWORK_SUBREACTOR_H_

#include <memory>
#include "Reactor.h"

namespace network {

class Session;
class Selector;
class SessionManager;
class ServiceMessage;

class SubReactor : public Reactor<ServiceMessage> {
 public:
  SubReactor(SessionManager* session_manager);
  virtual ~SubReactor();

  void Start();
  void Join();

  void SetNotifyCallback(
      const std::function<
          void(const std::shared_ptr<Session> &,
               const std::shared_ptr<ServiceMessage>&)>& callback);
  void SetNotifiedCallback(
      const std::function<
          void(const std::shared_ptr<Session> &,
               const std::shared_ptr<ServiceMessage>&)>& callback);

  void OnNotified(const std::shared_ptr<Session>& session,
                  const std::shared_ptr<ServiceMessage>& ctx);

 private:
  void OnNotify(const std::shared_ptr<Session>& session,
                const std::shared_ptr<ServiceMessage>& ctx);
  void OnDataRecv(std::shared_ptr<Session> &session, int event, void* ctx);
  void OnAccept(int sockfd, int event, void *ctx);

  void MainloopInit();
  void MainloopUninit();

  int local_sockfd_;
  std::unique_ptr<Selector> selector_;
  std::function<
      void(const std::shared_ptr<Session>&,
           const std::shared_ptr<ServiceMessage>&)> notify_callback_;
  std::function<
      void(const std::shared_ptr<Session>&,
           const std::shared_ptr<ServiceMessage>&)> notified_callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SUBREACTOR_H_ */
