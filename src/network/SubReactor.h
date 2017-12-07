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

class SubReactor : public Reactor {
 public:
  SubReactor(SessionManager* session_manager);
  virtual ~SubReactor();

  void SetNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);
  void SetNotifiedCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);

 private:
  void OnNotify(const std::shared_ptr<Session>& session);
  void OnNotified(const std::shared_ptr<Session>& session);
  void OnDataRecv(std::shared_ptr<Session> &session, int event, void* ctx);

  std::unique_ptr<Selector> selector_;
  std::function<void(const std::shared_ptr<Session>&)> notify_callback_;
  std::function<void(const std::shared_ptr<Session>&)> notified_callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SUBREACTOR_H_ */
