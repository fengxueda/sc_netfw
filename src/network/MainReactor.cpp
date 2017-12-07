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
      std::bind(&MainReactor::OnAcceptedNotify, this, std::placeholders::_1));
}

MainReactor::~MainReactor() {
}

void MainReactor::Start() {
  acceptor_->Start();
}

void MainReactor::Join() {
  acceptor_->Join();
}

void MainReactor::SetNotifyCallback(
    const std::function<
        void(const std::shared_ptr<Session>&,
             const std::shared_ptr<ServiceMessage>&)>& callback) {
  notify_callback_ = callback;
}

void MainReactor::SetNotifiedCallback(
    const std::function<
        void(const std::shared_ptr<Session>&,
             const std::shared_ptr<ServiceMessage>&)>& callback) {
  notified_callback_ = callback;
}

/* MainReactor通知SubReactor有客户端接入 */
void MainReactor::OnNotify(const std::shared_ptr<Session>& session,
                           const std::shared_ptr<ServiceMessage>& ctx) {
  notify_callback_(session, ctx);
}

void MainReactor::OnNotified(const std::shared_ptr<Session>& session,
                             const std::shared_ptr<ServiceMessage>& ctx) {
  notified_callback_(session, ctx);
}

void MainReactor::OnAcceptedNotify(const std::shared_ptr<Session>& session) {
  OnNotify(session, nullptr);
}

} /* namespace network */


