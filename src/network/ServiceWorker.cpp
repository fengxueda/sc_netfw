/*
 * ServiceWorker.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "ServiceWorker.h"

namespace network {

ServiceWorker::ServiceWorker()
    : running_(true),
      worker_(nullptr) {
  worker_ = new std::thread(&ServiceWorker::ServiceProcessor, this);
  CHECK_NOTNULL(worker_);
  DLOG(INFO) << "Service worker [" << worker_->get_id() << "] start up.";
}

ServiceWorker::~ServiceWorker() {
  running_ = false;
  worker_->join();
  delete worker_;
  worker_ = nullptr;
  DLOG(INFO) << __FUNCTION__;
}

void ServiceWorker::Start() {
  cond_var_.notify_one();
}

void ServiceWorker::Stop() {
  running_ = false;
}

void ServiceWorker::AddCallback(const std::function<void()>& callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  callbacks_.push_back(callback);
}

void ServiceWorker::ServiceProcessor() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_var_.wait(lock, [this]{return !callbacks_.empty();});
  while (running_) {
    for (auto function_cb : callbacks_) {
      function_cb();
    }
  }
}

} /* namespace network */


