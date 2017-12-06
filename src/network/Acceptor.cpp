/*
 * Acceptor.cpp
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "Acceptor.h"
#include "Selector.h"
#include "Utils.h"
#include "SessionImpl.h"
#include "SessionManager.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

Acceptor::Acceptor(SessionManager* session_manager)
    : listener_(-1),
      session_manager_(nullptr) {
  listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  CHECK(listener_ > 0);

  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(kServerPort);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  CHECK_STATUS(FATAL, evutil_make_listen_socket_reuseable(listener_));
  CHECK_STATUS(
      FATAL,
      bind(listener_, (struct sockaddr * ) &server, sizeof(struct sockaddr)));
  CHECK_STATUS(FATAL, listen(listener_, kMaxListenCount));
  CHECK_STATUS(FATAL, evutil_make_socket_nonblocking(listener_));
  DLOG(INFO)<< "Bind port(" << kServerPort << ") successful. Listening...";

  CHECK_NOTNULL(session_manager);
  session_manager_ = session_manager;
  selector_.reset(new Selector(session_manager_));
  selector_->SetAcceptCallback(
      std::bind(&Acceptor::OnAcceptCallback, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  std::shared_ptr<Selector::ListenEvent> event = std::make_shared<
      Selector::ListenEvent>();
  event->set_sockfd(listener_);
  event->set_type(Selector::TYPE_ACCEPT);
  selector_->AddEvent(event);
}

Acceptor::~Acceptor() {
  if (listener_ > 0) {
    CHECK_STATUS(WARNING, evutil_closesocket(listener_));
  }
  DLOG(INFO)<< __FUNCTION__;
}

void Acceptor::SetAcceptNotifyCallback(
    const std::function<void(const std::shared_ptr<Session> &)>& callback) {
  callback_ = callback;
}

void Acceptor::OnAcceptCallback(int sockfd, int event, void *ctx) {
  struct sockaddr_in client;
  socklen_t size = sizeof(client);
  memset(&client, 0, sizeof(client));
  int client_sd = accept(listener_, (struct sockaddr *) &client, &size);
  CHECK_STATUS(WARNING, client_sd);
  CHECK(client_sd > 0);
  CHECK_STATUS(FATAL, evutil_make_socket_nonblocking(client_sd));

  std::shared_ptr<Session> session = std::make_shared<SessionImpl>();
  session->set_remote_port(ntohs(client.sin_port));
  session->set_remote_ip(inet_ntoa(client.sin_addr));
  session->set_sockfd(client_sd);
  session->set_create_time(GetLocalDate());
  session->set_update_time(GetLocalDate());
  session->set_session_id(
      session->remote_ip() + ":" + std::to_string(session->remote_port()));
  DLOG(INFO)<< "Accept remote client : " << session->session_id();
  session_manager_->AddSession(session);
  OnNotifySubReactor(session_manager_->GetSession(session->session_id()));
}

void Acceptor::OnNotifySubReactor(const std::shared_ptr<Session>& session) {
  if (session != nullptr) {
    callback_(session);
  }
}

} /* namespace network */

#undef CHECK_STATUS
