/*
 * MessageDemutiplexor.cpp
 *
 *  Created on: 2017年12月8日
 *      Author: xueda
 */

#include <sys/sysinfo.h>
#include <glog/logging.h>
#include "ServiceWorker.h"
#include "MessageDemutiplexor.h"

namespace network {

MessageDemutiplexor::MessageDemutiplexor() {
  /* 顺序不能变 */
  CreateServiceWorkers();
  RegisterMessageDispatcher();
}

MessageDemutiplexor::~MessageDemutiplexor() {
  for (auto worker : workers_) {
    worker->Stop();
  }
  for (unsigned int index = 0; index < workers_.size(); index++) {
    messages_.push(nullptr);
    cond_var_.notify_all();
  }
  workers_.clear();
  DLOG(INFO)<< __FUNCTION__;
}

void MessageDemutiplexor::StartUp() {

}

void MessageDemutiplexor::OnPushMessage(
    const std::shared_ptr<ServiceMessage>& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  messages_.push(message);
  cond_var_.notify_one();
}

void MessageDemutiplexor::AddCallback(
    const std::function<void(const std::shared_ptr<ServiceMessage> &)>& callback) {
  callbacks_.push_back(callback);
}

void MessageDemutiplexor::CreateServiceWorkers() {
  for (int index = 0; index < get_nprocs_conf(); index++) {
    std::shared_ptr<ServiceWorker> worker = std::make_shared<ServiceWorker>();
    CHECK_NOTNULL(worker.get());
    workers_.push_back(worker);
  }
}

void MessageDemutiplexor::RegisterMessageDispatcher() {
  for (auto worker : workers_) {
    worker->AddCallback(
        std::bind(&MessageDemutiplexor::OnMessageDispatch, this));
    worker->Start();
  }
}

void MessageDemutiplexor::OnMessageDispatch() {
  std::shared_ptr<ServiceMessage> message;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !messages_.empty();});
    message = messages_.front();
    messages_.pop();
  }

  if (message != nullptr) {
    for (auto callback : callbacks_) {
      callback(message);
    }
  }

}

} /* namespace network */


