/*
 * SessionDemutiplexor.cpp
 *
 *  Created on: 2017年12月8日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "Session.h"
#include "ServiceMessage.h"
#include "SubReactor.h"
#include "SessionDemutiplexor.h"

namespace network {

SessionDemutiplexor::SessionDemutiplexor(SessionManager* session_manager,
                                         int thread_count) {
  CHECK_NOTNULL(session_manager);
  for (int index = 0; index < thread_count; index++) {
    std::shared_ptr<SubReactor> reactor = std::make_shared<SubReactor>(
        session_manager);
    CHECK_NOTNULL(reactor.get());
    reactor->AddMainloopCallback(
        std::bind(&SessionDemutiplexor::OnSessionDispatch, this));
    sub_reactors_[reactor->reactor_id()] = reactor;
    reactor->Start();
  }
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

void SessionDemutiplexor::AddCallback(
    const std::function<void(const std::shared_ptr<ServiceMessage>&)>& callback) {
  callbacks_.push_back(callback);
}

void SessionDemutiplexor::OnPushSession(
    const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.push(session);
  cond_var_.notify_one();
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
      sub_reactors_[stream.str()]->OnSessionHandler(session);

    }
  }

}

} /* namespace network */

