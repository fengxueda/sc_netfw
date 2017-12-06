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

class MainReactor : public Reactor {
 public:
  MainReactor(SessionManager* session_manager);
  virtual ~MainReactor();

  virtual void SetNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>& )>& callback);
  virtual void OnNotify(const std::shared_ptr<Session>& session);

 private:
  std::unique_ptr<Acceptor> acceptor_;
  std::function<void(const std::shared_ptr<Session>& session)> notify_callback_;
};

} /* namespace network */

#endif /* SRC_NETWORK_MAINREACTOR_H_ */
