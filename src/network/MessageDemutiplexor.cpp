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
  for (const auto& worker : workers_) {
    worker->Stop();
  }
  for (unsigned int index = 0; index < workers_.size(); index++) {
    queue_.push(nullptr);
    cond_var_.notify_all();
  }
  workers_.clear();
  DLOG(INFO)<< __FUNCTION__;
}

void MessageDemutiplexor::StartUp() {

}

void MessageDemutiplexor::OnPushMessage(
    const std::shared_ptr<ServiceContext>& context) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(context);
  cond_var_.notify_one();
}

void MessageDemutiplexor::AddCallback(
    const std::function<void(const std::shared_ptr<ServiceContext> &)>& callback) {
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
  for (const auto& worker : workers_) {
    worker->AddCallback(
        std::bind(&MessageDemutiplexor::OnMessageDispatch, this));
    worker->Start();
  }
}

void MessageDemutiplexor::OnMessageDispatch() {
  std::shared_ptr<ServiceContext> context;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !queue_.empty();});
    context = queue_.front();
    queue_.pop();
  }

  if (context != nullptr) {
    /* CALLBACK : plugin::ServiceHandler::OnHandler
     * MessageHandler, threadpool get this datagram for processing.
     */
    for (const auto& callback : callbacks_) {
      callback(context);
    }
    context.reset();
  }

}

} /* namespace network */


