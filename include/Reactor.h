/*
 * Reactor.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef INCLUDE_REACTOR_H_
#define INCLUDE_REACTOR_H_

#include <glog/logging.h>
#include <memory>

namespace network {

class Session;
class SessionManager;

class Reactor {
 public:
  Reactor(SessionManager* session_manager)
      : session_manager_(nullptr) {
    CHECK_NOTNULL(session_manager);
    session_manager_ = session_manager;
  }
  virtual ~Reactor() {
  }

  /* 设置 This Reactor 通知回调函数 */
  virtual void SetNotifyCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback) = 0;
  /* 设置 This Reactor 被通知回调函数 */
  virtual void SetNotifiedCallback(
      const std::function<void(const std::shared_ptr<Session>&)>& callback) = 0;

 private:
  SessionManager* session_manager_;
};

}
// namespace network

#endif /* INCLUDE_REACTOR_H_ */
