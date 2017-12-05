/*
 * EventDemutiplexor.cpp
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "ServiceWorker.h"
#include "EventDemutiplexor.h"
#include "ServiceEvent.h"

namespace network {

EventDemutiplexor::EventDemutiplexor() {
  CreateServiceWorkers();
  RegisterEventDispatcher();
}

EventDemutiplexor::~EventDemutiplexor() {
  for (auto worker : workers_) {
    worker->Stop();
  }
  for (unsigned int index = 0; index < kThreadCount; index++) {
    events_.push(nullptr);
    cond_var_.notify_all();
  }
  for (auto worker : workers_) {
    worker.reset();
  }
  usleep(kThreadCount * 100 * 1000);
  DLOG(INFO)<< __FUNCTION__;
}

void EventDemutiplexor::CreateServiceWorkers() {
  for (int index = 0; index < kThreadCount; index++) {
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
    std::function<void(std::shared_ptr<ServiceEvent> &)>& callback) {
  callbacks_.push_back(callback);
}

void EventDemutiplexor::PushEventToDispatcher(
    const std::shared_ptr<ServiceEvent>& event) {
  std::lock_guard<std::mutex> lock(mutex_);
  events_.push(event);
  cond_var_.notify_one();
}

void EventDemutiplexor::EventDispatcher() {
  std::shared_ptr<ServiceEvent> event;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !events_.empty();});
    event = events_.front();
    events_.pop();
  }
  if (event == nullptr) {
    return;
  }

  std::lock_guard<std::mutex> lock(event->mutex());
  for (auto cb_function : callbacks_) {
    cb_function(event);
  }
}

} /* namespace network */
