/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "NetWrapper.h"
#include "Error.h"
#include "Session.h"
#include "ServiceWorker.h"
#include "Reactor.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

NetWrapper::NetWrapper()
    : listen_sd_(-1),
      base_(nullptr) {
  LibeventInit();
  TcpServerInit();
}

NetWrapper::~NetWrapper() {
  LibeventDestory();
  TcpServerDestory();
}

void NetWrapper::TcpServerInit() {
  listen_sd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  CHECK(listen_sd_ > 0);

  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(kServerPort);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  CHECK_STATUS(FATAL, evutil_make_listen_socket_reuseable(listen_sd_));
  CHECK_STATUS(
      FATAL,
      bind(listen_sd_, (struct sockaddr * )&server, sizeof(struct sockaddr)));
  CHECK_STATUS(FATAL, listen(listen_sd_, kMaxListenCount));
  evutil_make_socket_nonblocking(listen_sd_);
  DLOG(INFO)<< "Bind port(" << kServerPort << ") successful. Listening...";
}

void NetWrapper::TcpServerDestory() {
  if (listen_sd_ > 0) {
    CHECK_STATUS(WARNING, evutil_closesocket(listen_sd_));
  }
}

void NetWrapper::LibeventInit() {
  struct event_config* config = event_config_new();
  CHECK_NOTNULL(config);
  event_config_avoid_method(config, "select");
  event_config_require_features(config, EV_FEATURE_ET);
  base_ = event_base_new_with_config(config);
  CHECK_NOTNULL(base_);
  event_config_free(config);
  DLOG(INFO)<<"Current method of I/O checking : " << event_base_get_method(base_);

  /*
   * 可选设置优先级数目，然后通过event_priority_set设置事件的优先级
   * 0为最高，n_priority-1为最低，此后创建的事件默认优先级为中间优先级
   */
  event_base_priority_init(base_, 3);
}

void NetWrapper::LibeventDestory() {
  CHECK_NOTNULL(base_);
  event_base_free(base_);
}

void NetWrapper::Launch() {
  CreateReactor();
  CreateServiceWorkers();
  CHECK_NOTNULL(base_);
  event_base_dispatch(base_);
}

void NetWrapper::AddSession(const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (sessions_.end() == sessions_.find(session->session_id())) {
    return;
  }
  sessions_[session->session_id()] = session;
  cond_var_.notify_one();
}

void NetWrapper::DeleteSession(const std::string& session_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = sessions_.find(session_id);
  if (iter != sessions_.end()) {
    sessions_.erase(iter);
  }
  cond_var_.notify_one();
}

std::shared_ptr<Session> NetWrapper::GetSession(const std::string& session_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_var_.wait(lock, [this] {return !sessions_.empty();});
  auto iter = sessions_.find(session_id);
  if (iter != sessions_.end()) {
    return sessions_[session_id];
  }
  return nullptr;
}

void NetWrapper::CreateReactor() {
  reactor_.reset(new Reactor(this));
  CHECK_NOTNULL(reactor_.get());
}

void NetWrapper::CreateServiceWorkers() {
  for (int index = 0; index < kThreadCount; index++) {
    std::shared_ptr<ServiceWorker> worker = std::make_shared<ServiceWorker>(
        this);
    CHECK_NOTNULL(worker.get());
    service_workers_.push_back(worker);
  }
}

}
/* namespace network */

#undef CHECK_STATUS

