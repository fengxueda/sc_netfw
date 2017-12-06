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

  virtual void SetNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback);
  virtual void OnNotify(const std::shared_ptr<Session>& session);
  void OnDataRecv(std::shared_ptr<Session> &session, int event, void* ctx);

 private:
  std::unique_ptr<Selector> selector_;
  std::function<void(const std::shared_ptr<Session>&)> callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SUBREACTOR_H_ */
