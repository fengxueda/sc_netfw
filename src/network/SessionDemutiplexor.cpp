/*
 * SessionDemutiplexor.cpp
 *
 *  Created on: 2017年12月8日
 *      Author: xueda
 */

#include <sys/sysinfo.h>
#include <glog/logging.h>
#include "Session.h"
#include "ServiceMessage.h"
#include "SessionManager.h"
#include "MainReactor.h"
#include "SubReactor.h"
#include "SessionDemutiplexor.h"

namespace network {

SessionDemutiplexor::SessionDemutiplexor() {
  CreateSessionManager();
  CreateMainReactor();
  CreateSubReactors();
  MakeRelationship();
  StartUpReactors();
}

SessionDemutiplexor::~SessionDemutiplexor() {
  for (auto sub_reactor : sub_reactors_) {
    sub_reactor.second->Stop();
  }
  for (unsigned int index = 0; index < sub_reactors_.size(); index++) {
    sessions_.push(nullptr);
    cond_var_.notify_all();
  }
  sub_reactors_.clear();
  DLOG(INFO)<< __FUNCTION__;
}

void SessionDemutiplexor::StartUp() {
  main_reactor_->Join();
}

void SessionDemutiplexor::AddPushMessageCallback(
    const std::function<void(const std::shared_ptr<ServiceMessage>&)>& callback) {
  push_msg_callbacks_.push_back(callback);
}

void SessionDemutiplexor::OnPushSession(
    const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.push(session);
  cond_var_.notify_one();
}

void SessionDemutiplexor::OnPushMessage(
    const std::shared_ptr<ServiceMessage>& message) {
  for (auto callback : push_msg_callbacks_) {
    callback(message);
  }
}

void SessionDemutiplexor::OnSessionDispatch() {
  std::shared_ptr<Session> session;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !sessions_.empty();});
    session = sessions_.front();
    sessions_.pop();
  }

  if (session != nullptr) {
    std::stringstream stream;
    stream << std::this_thread::get_id();
    if (sub_reactors_.find(stream.str()) != sub_reactors_.end()) {
      sub_reactors_[stream.str()]->OnDataRecv(session);
    }
  }
}

void SessionDemutiplexor::CreateSessionManager() {
  session_manager_.reset(new SessionManager());
  CHECK_NOTNULL(session_manager_.get());
}

void SessionDemutiplexor::CreateMainReactor() {
  main_reactor_.reset(new MainReactor(session_manager_.get()));
  CHECK_NOTNULL(main_reactor_.get());
}

void SessionDemutiplexor::CreateSubReactors() {
  for (int index = 0; index < get_nprocs_conf(); index++) {
    std::shared_ptr<SubReactor> reactor = std::make_shared<SubReactor>(
        session_manager_.get());
    CHECK_NOTNULL(reactor.get());
    sub_reactors_[reactor->reactor_id()] = reactor;
  }
}

void SessionDemutiplexor::MakeRelationship() {
  main_reactor_->AddPushSessionCallback(
      std::bind(&SessionDemutiplexor::OnPushSession, this,
                std::placeholders::_1));
  for (auto sub_reactor : sub_reactors_) {
    sub_reactor.second->AddEventActionCallback(
        std::bind(&MainReactor::OnEventAction, main_reactor_.get(),
                  std::placeholders::_1, std::placeholders::_2));
    sub_reactor.second->AddMainloopCallback(
        std::bind(&SessionDemutiplexor::OnSessionDispatch, this));
    sub_reactor.second->AddPushMessageCallback(
        std::bind(&SessionDemutiplexor::OnPushMessage, this,
                  std::placeholders::_1));
  }
}

void SessionDemutiplexor::StartUpReactors() {
  main_reactor_->Start();
  for (auto sub_reactor : sub_reactors_) {
    sub_reactor.second->Start();
  }
}

} /* namespace network */

