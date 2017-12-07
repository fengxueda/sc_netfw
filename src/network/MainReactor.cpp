/*
 * MainReactor.cpp
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "MainReactor.h"
#include "Acceptor.h"

namespace network {

MainReactor::MainReactor(SessionManager* session_manager)
    : Reactor(session_manager) {
  CHECK_NOTNULL(session_manager);
  acceptor_.reset(new Acceptor(session_manager, kServerPort, kMaxListenCount));
  acceptor_->SetAcceptedNotifyCallback(
      std::bind(&MainReactor::OnNotify, this, std::placeholders::_1));
}

MainReactor::~MainReactor() {
}

void MainReactor::SetNotifyCallback(
    const std::function<void(const std::shared_ptr<Session>&)>& callback) {
  notify_callback_ = callback;
}

void MainReactor::SetNotifiedCallback(
    const std::function<void(const std::shared_ptr<Session> &)>& callback) {
  notified_callback_ = callback;
}

/* MainReactor通知SubReactor有客户端接入 */
void MainReactor::OnNotify(const std::shared_ptr<Session>& session) {
  notify_callback_(session);
}

void MainReactor::OnNotified(const std::shared_ptr<Session>& session) {
  notified_callback_(session);
}

} /* namespace network */
