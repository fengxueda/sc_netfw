/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "Error.h"
#include "NetWrapper.h"
#include "Session.h"
#include "EventDemutiplexor.h"
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
  TcpServerInit();
}

NetWrapper::~NetWrapper() {
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

void NetWrapper::Launch() {
  CreateReactors();
  CreateDemutiplexor();
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

void NetWrapper::CreateReactors() {
  sub_reactor_.reset(new Reactor(this));
  CHECK_NOTNULL(sub_reactor_.get());
  sub_reactor_->SetupSubReactor();
  main_reactor_.reset(new Reactor(this));
  CHECK_NOTNULL(main_reactor_.get());
  main_reactor_->SetupMainReactor(listen_sd_, sub_reactor_.get());
}

void NetWrapper::CreateDemutiplexor() {
  event_demutiplexor_.reset(new EventDemutiplexor());
  CHECK_NOTNULL(event_demutiplexor_.get());
}

}
/* namespace network */

#undef CHECK_STATUS

