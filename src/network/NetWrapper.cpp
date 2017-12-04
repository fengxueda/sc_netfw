/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>
#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "NetWrapper.h"
#include "Error.h"
#include "Session.h"
#include "DataPacket.h"
#include "Utils.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

int GetErrorCode(int socket_id) {
  int errcode;
  unsigned int size = sizeof(errcode);
  getsockopt(socket_id, SOL_SOCKET, SO_ERROR, (void *) &errcode, &size);
  return errcode;
}

void OnAccept(int listen_sd, short event, void* args) {
  EventBase* event_base = (EventBase *) args;
  CHECK_NOTNULL(event_base->base);
  struct sockaddr_in client;
  socklen_t size = sizeof(client);
  memset(&client, 0, sizeof(client));
  int client_sd = accept(listen_sd, (struct sockaddr *) &client, &size);
  CHECK_STATUS(WARNING, client_sd);
  CHECK(client_sd > 0);

  std::shared_ptr<Session> session = std::make_shared<Session>();
  session->set_session_id(std::to_string(GetCurrentTimestamp()));
  session->set_remote_port(ntohs(client.sin_port));
  session->set_socket(client_sd);
  session->set_create_time(GetLocalDate());
  session->set_update_time(GetLocalDate());
  session->set_remote_ip(inet_ntoa(client.sin_addr));
  NetWrapper* wrapper = (NetWrapper *) event_base->wrapper;
  CHECK_NOTNULL(wrapper);
  wrapper->AddSession(session);

  struct bufferevent* buffer_event = bufferevent_socket_new(
      event_base->base, client_sd, BEV_OPT_CLOSE_ON_FREE);
  CHECK_NOTNULL(buffer_event);
  bufferevent_setcb(buffer_event, OnRead, OnWrite, OnStatusReport, args);
  bufferevent_enable(buffer_event, EV_READ | EV_WRITE | EV_PERSIST);
}

void OnRead(struct bufferevent *buffer_event, void *ctx) {
#define MAXSIZE    4096
  int nbytes;
  unsigned char buffer[MAXSIZE];
  std::shared_ptr<DataPacket> message = std::make_shared<DataPacket>();
  evutil_socket_t read_sd = bufferevent_getfd(buffer_event);
  do {
    nbytes = bufferevent_read(buffer_event, buffer, MAXSIZE);
    switch (nbytes) {
      case 0: {
        DLOG(WARNING)<< "The connection is closed. Message : " << strerror(GetErrorCode(read_sd));
        bufferevent_free(buffer_event);
        return;
      }
      break;
      case -1: {
        int errcode = GetErrorCode(read_sd);
        if (EAGAIN != errcode) {
          DLOG(WARNING) << "The connection occurs error. Message : " << strerror(GetErrorCode(read_sd));
          bufferevent_free(buffer_event);
        }
        // TODO : Push the datagram to queue

        return;
      }
      break;
      default: {
        message->PushBack(buffer, nbytes);
      }
      break;
    }

  }while(nbytes > 0);

#undef MAXSIZE
}

void OnWrite(struct bufferevent *buffer_event, void *ctx) {
  //    bufferevent_write(bev, line, n);
}

void OnStatusReport(struct bufferevent *buffer_event, short what, void *ctx) {
  evutil_socket_t socket_id = bufferevent_getfd(buffer_event);
  DLOG(INFO)<< "Socket (" << socket_id << ") has new status to report : ";
  switch (what) {
    case BEV_EVENT_EOF: {
      DLOG(INFO)<< "This connection is closed.";
      bufferevent_free(buffer_event);
    }
    break;
    case BEV_EVENT_ERROR: {
      DLOG(INFO) << "Error occurs , message : " << strerror(GetErrorCode(socket_id));
      bufferevent_free(buffer_event);
    }
    break;
    case BEV_EVENT_TIMEOUT: {
      // no implement
    }
    break;
    default:
    break;
  }
}

NetWrapper::NetWrapper()
    : listen_sd_(-1),
      base_(nullptr) {
  LibeventInit();
  TcpServerInit();
  LibeventRegister();
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

void NetWrapper::LibeventRegister() {
  EventBase* event_base = new EventBase();
  event_base->base = base_;
  event_base->wrapper = this;
  struct event* listen_event = event_new(base_, listen_sd_,
  EV_READ | EV_PERSIST,
                                         OnAccept, (void *) event_base);
  event_add(listen_event, nullptr);
}

void NetWrapper::Launch() {
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

}
/* namespace network */

#undef CHECK_STATUS

