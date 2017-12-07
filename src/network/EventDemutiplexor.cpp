/*
 * EventDemutiplexor.cpp
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "ServiceWorker.h"
#include "EventDemutiplexor.h"
#include "ServiceMessage.h"
#include "Session.h"

namespace network {

EventDemutiplexor::EventDemutiplexor(int thread_count)
    : worker_count_(0) {
  worker_count_ = thread_count;
  /* 顺序不能变 */
  CreateServiceWorkers();
  RegisterEventDispatcher();
}

EventDemutiplexor::~EventDemutiplexor() {
  for (auto worker : workers_) {
    worker->Stop();
  }
  for (int index = 0; index < worker_count_; index++) {
    messages_.push(nullptr);
    cond_var_.notify_all();
  }
  for (auto worker : workers_) {
    worker.reset();
  }
  DLOG(INFO)<< __FUNCTION__;
}

void EventDemutiplexor::CreateServiceWorkers() {
  for (int index = 0; index < worker_count_; index++) {
    std::shared_ptr<ServiceWorker> worker = std::make_shared<ServiceWorker>();
    CHECK_NOTNULL(worker.get());
    workers_.push_back(worker);
  }
}

void EventDemutiplexor::RegisterEventDispatcher() {
  for (auto worker : workers_) {
    worker->AddCallback(std::bind(&EventDemutiplexor::EventDispatcher, this));
  }
}

void EventDemutiplexor::AddCallback(
    const std::function<
        void(const std::shared_ptr<Session> &,
             const std::shared_ptr<ServiceMessage> &)>& callback) {
  callbacks_.push_back(callback);
}

void EventDemutiplexor::PushEventToDispatcher(
    const std::shared_ptr<Session>& session,
    const std::shared_ptr<ServiceMessage>& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  messages_.push(message);
  cond_var_.notify_one();
}

void EventDemutiplexor::EventDispatcher() {
  std::shared_ptr<ServiceMessage> message;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !messages_.empty();});
    message = messages_.front();
    messages_.pop();
  }
  if (message == nullptr) {
    return;
  }

  std::lock_guard<std::mutex> lock(message->mutex());
  for (auto cb_function : callbacks_) {
    cb_function(nullptr ,message);
  }
}

} /* namespace network */

