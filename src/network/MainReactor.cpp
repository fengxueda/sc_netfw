/*
 * MainReactor.cpp
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "MainReactor.h"
#include "Acceptor.h"
#include "Session.h"
#include "Selector.h"

namespace network {

MainReactor::MainReactor(SessionManager* session_manager) {
  CHECK_NOTNULL(session_manager);
  acceptor_.reset(new Acceptor(session_manager, kServerPort, kMaxListenCount));
  acceptor_->SetAcceptedNotifyCallback(
      std::bind(&MainReactor::OnAcceptedNotify, this, std::placeholders::_1));
  acceptor_->SetDataRecvCallback(
      std::bind(&MainReactor::OnDataRecvNotify, this, std::placeholders::_1));
}

MainReactor::~MainReactor() {

}

void MainReactor::Start() {
  acceptor_->Start();
}

void MainReactor::Join() {
  acceptor_->Join();
}

void MainReactor::AddDataRecvCallback(
    const std::function<void(const std::shared_ptr<Session>&)>& callback) {
  recv_callbacks_.push_back(callback);
}

void MainReactor::OnAcceptedNotify(const std::shared_ptr<Session>& session) {
  DLOG(INFO)<< "Accept remote client : " << session->session_id();
}

void MainReactor::OnDataRecvNotify(const std::shared_ptr<Session>& session) {
  DLOG(INFO)<<__FUNCTION__;
  for (auto callback : recv_callbacks_) {
    callback(session);
  }
}

}
/* namespace network */

