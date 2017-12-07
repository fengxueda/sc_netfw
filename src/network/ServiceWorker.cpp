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

void ServiceWorker::Stop() {
  running_ = false;
}

void ServiceWorker::AddCallback(const std::function<void()>& callback) {
  callbacks_.push_back(callback);
}

void ServiceWorker::ServiceProcessor() {
  while (running_) {
    for (auto function_cb : callbacks_) {
      function_cb();
    }
  }
}

} /* namespace network */


