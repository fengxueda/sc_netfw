/*
 * SubReactor.cpp
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
#include "SubReactor.h"
#include "Selector.h"
#include "Session.h"
#include "DataPacket.h"
#include "ServiceMessage.h"
#include "Utils.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

static unsigned short kLocalPort = 30000;

SubReactor::SubReactor(SessionManager* session_manager)
    : Reactor(session_manager),
      local_sockfd_(-1) {
  CHECK_NOTNULL(session_manager);
  selector_.reset(new Selector(session_manager));
  selector_->SetDataRecvCallback(
      std::bind(&SubReactor::OnDataRecv, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  // 该Accept回调没有任何作用，只为了让libevent的main-loop能运行起来
  selector_->SetAcceptedCallback(
      std::bind(&SubReactor::OnAccept, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  MainloopInit();
}

SubReactor::~SubReactor() {
  MainloopUninit();
}

void SubReactor::Start() {
  selector_->Start();
}

void SubReactor::Join() {
  selector_->Join();
}

void SubReactor::SetNotifyCallback(
    const std::function<
        void(const std::shared_ptr<Session> &,
             const std::shared_ptr<ServiceMessage>&)>& callback) {
  notify_callback_ = callback;
}

void SubReactor::SetNotifiedCallback(
    const std::function<
        void(const std::shared_ptr<Session> &,
             const std::shared_ptr<ServiceMessage>&)>& callback) {
  notified_callback_ = callback;
}

void SubReactor::OnNotify(const std::shared_ptr<Session>& session,
                          const std::shared_ptr<ServiceMessage>& ctx) {
  notify_callback_(session, ctx);
}

/* SubReactor增加一个读事件 */
void SubReactor::OnNotified(const std::shared_ptr<Session>& session,
                            const std::shared_ptr<ServiceMessage>& ctx) {
  std::shared_ptr<Selector::ListenEvent> event = std::make_shared<
      Selector::ListenEvent>();
  event->set_sockfd(session->sockfd());
  event->set_type(Selector::TYPE_READ);
  selector_->AddEvent(event);
//  notified_callback_(session, ctx);
}

#define MAXSIZE 4096
void SubReactor::OnDataRecv(std::shared_ptr<Session>& session, int event,
                            void* ctx) {
// TODO : Read data from remote
  CHECK_NOTNULL(session.get());
  std::shared_ptr<ServiceMessage> message = std::make_shared<ServiceMessage>();
  std::shared_ptr<DataPacket> datagram = std::make_shared<DataPacket>();
  unsigned char buffer[MAXSIZE];
  do {
    int nbyte = read(session->sockfd(), buffer, sizeof(buffer));
    if (nbyte == -1 && errno == EAGAIN) {
      break;
    }
    switch (nbyte) {
      case 0: {
        DLOG(WARNING)<< "The connection [" << session->session_id()
        << "] closed by the other side.";
        selector_->DeleteEvent(session->sockfd());
        selector_->ReleaseConnection(session);
        return;
      }
      break;
      case -1: {
        DLOG(WARNING) << "The connection occurs error, message : "
        << strerror(GetErrorCodeBySocket(session->sockfd()));
        selector_->ReleaseConnection(session);
        return;
      }
      break;
      default:
      break;
    }
    /* Push data to datagram */
    datagram->PushBack(buffer, nbyte);
  } while (1);

  message->set_session(session);
  message->set_datagram(datagram);

  OnNotify(session, message);
}
#undef MAXSIZE

void SubReactor::OnAccept(int sockfd, int event, void* ctx) {
// No implement, just for start up the main-loop
}

void SubReactor::MainloopInit() {
  local_sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  CHECK(local_sockfd_ > 0);
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(kLocalPort++);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  CHECK_STATUS(FATAL, evutil_make_listen_socket_reuseable(local_sockfd_));
  CHECK_STATUS(
      FATAL,
      bind(local_sockfd_, (struct sockaddr * ) &server,
           sizeof(struct sockaddr)));
  CHECK_STATUS(FATAL, listen(local_sockfd_, 1));
  CHECK_STATUS(FATAL, evutil_make_socket_nonblocking(local_sockfd_));

  std::shared_ptr<Selector::ListenEvent> event = std::make_shared<
      Selector::ListenEvent>();
  event->set_sockfd(local_sockfd_);
  event->set_type(Selector::TYPE_ACCEPT);
  selector_->AddEvent(event);
}

void SubReactor::MainloopUninit() {
  if (local_sockfd_ > 0) {
    evutil_closesocket(local_sockfd_);
  }
}

} /* namespace network */

#undef CHECK_STATUS

