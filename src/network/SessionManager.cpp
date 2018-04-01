/*
 * SessionManager.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "Session.h"
#include "SessionManager.h"

namespace network {

SessionManager::SessionManager()
    : running_(true),
      monitor_(nullptr) {
  monitor_ = new std::thread(&SessionManager::MonitorThread, this);
  CHECK_NOTNULL(monitor_);
}

SessionManager::~SessionManager() {
  running_ = false;
  monitor_->join();
  delete monitor_;
  monitor_ = nullptr;
  sessions_.clear();
  DLOG(INFO)<< __FUNCTION__;
}

void SessionManager::Stop() {
  running_ = false;
}

bool SessionManager::Exist(const std::string& session_id) {
  if (nullptr != GetSession(session_id)) {
    return true;
  }
  return false;
}

void SessionManager::AddSession(const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (sessions_.end() != sessions_.find(session->session_id())) {
    return;
  }
  sessions_[session->session_id()] = session;
}

void SessionManager::DeleteSession(const std::string& session_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = sessions_.find(session_id);
  if (iter != sessions_.end()) {
    sessions_.erase(iter);
  }
}

std::shared_ptr<Session> SessionManager::GetSession(
    const std::string& session_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = sessions_.find(session_id);
  if (iter != sessions_.end()) {
    return sessions_[session_id];
  }
  return nullptr;
}

void SessionManager::MonitorThread() {
  while (running_) {
    // FIXME : No implement now
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

} /* namespace network */

