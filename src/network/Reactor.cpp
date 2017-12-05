/*
 * Reactor.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "Session.h"
#include "Reactor.h"
#include "DataPacket.h"
#include "NetWrapper.h"
#include "ServiceEvent.h"
#include "Utils.h"
#include "Error.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

static int GetErrorCode(int socket_id);
static void OnAccept(int listen_sd, short event, void *args);
static void OnRead(struct bufferevent *buffer_event, void *ctx);
static void OnWrite(struct bufferevent *buffer_event, void *ctx);
static void OnStatusReport(struct bufferevent *buffer_event, short what,
                           void *ctx);

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
  session->set_remote_port(ntohs(client.sin_port));
  session->set_remote_ip(inet_ntoa(client.sin_addr));
  session->set_socket(client_sd);
  session->set_create_time(GetLocalDate());
  session->set_update_time(GetLocalDate());
  session->set_session_id(
      session->remote_ip() + ":" + std::to_string(session->remote_port()));
  Reactor* reactor = (Reactor *) event_base->reactor;
  CHECK_NOTNULL(reactor);
  reactor->CreateListener(client_sd);
//  wrapper->AddSession(session);
}

void OnRead(struct bufferevent *buffer_event, void *ctx) {
  // TODO : Push the event to thread-pool
  std::shared_ptr<ServiceEvent> service_event =
      std::make_shared<ServiceEvent>();
  service_event->set_type(ServiceEvent::TPYE_READ);
  service_event->set_buffer_event(buffer_event);
  service_event->set_ctx(ctx);
}

void OnWrite(struct bufferevent *buffer_event, void *ctx) {
  // FIXME : No implement now
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
      DLOG(INFO) << "Error occurs , message : "
      << strerror(GetErrorCode(socket_id));
      bufferevent_free(buffer_event);
    }
    break;
    case BEV_EVENT_TIMEOUT: {
      // FIXME : no implement
    }
    break;
    default:
    break;
  }
}

Reactor::Reactor(NetWrapper *wrapper)
    : type_(TYPE_UNDEFINED),
      base_(nullptr),
      wrapper_(nullptr),
      sub_reactor_(nullptr) {
  wrapper_ = wrapper;
  CHECK_NOTNULL(wrapper_);
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

Reactor::~Reactor() {
  CHECK_NOTNULL(base_);
  event_base_free(base_);
}

void Reactor::SetupMainReactor(int listen_sd, Reactor* sub_reactor) {
  CHECK(type_ == TYPE_UNDEFINED);
  DLOG(INFO)<< "Setup main reactor.";
  EventBase* event_base = new EventBase();
  event_base->base = base_;
  event_base->reactor = this;
  struct event* listen_event = event_new(base_, listen_sd, EV_READ | EV_PERSIST,
                                         OnAccept, (void *) event_base);
  event_add(listen_event, nullptr);
  sub_reactor_ = sub_reactor;
  CHECK_NOTNULL(sub_reactor_);
  type_ = TYPE_MAIN_REACTOR;
}

void Reactor::SetupSubReactor() {
  CHECK(type_ == TYPE_UNDEFINED);
  DLOG(INFO)<< "Setup subclass reactor.";
  type_ = TYPE_SUB_REACTOR;
}

void Reactor::CreateListener(int client_sd) {
  CHECK(type_ == TYPE_MAIN_REACTOR);
  struct bufferevent* buffer_event = bufferevent_socket_new(
      sub_reactor_->base(), client_sd, BEV_OPT_CLOSE_ON_FREE);
  CHECK_NOTNULL(buffer_event);
  bufferevent_setcb(buffer_event, OnRead, OnWrite, OnStatusReport,
                    sub_reactor_->base());
  bufferevent_enable(buffer_event, EV_READ | EV_WRITE | EV_PERSIST);
}

void Reactor::RecvData(struct bufferevent *buffer_event, void *ctx) {
#define MAXSIZE    4096
  int nbytes;
  unsigned char buffer[MAXSIZE];
  std::shared_ptr<DataPacket> message = std::make_shared<DataPacket>();
  evutil_socket_t read_sd = bufferevent_getfd(buffer_event);
  do {
    nbytes = bufferevent_read(buffer_event, buffer, MAXSIZE);
    switch (nbytes) {
      case 0: {
        DLOG(WARNING)<< "The connection is closed. Message : "
        << strerror(GetErrorCode(read_sd));
        bufferevent_free(buffer_event);
        return;
      }
      break;
      case -1: {
        int errcode = GetErrorCode(read_sd);
        if (EAGAIN != errcode) {
          DLOG(WARNING) << "The connection occurs error. Message : " << strerror(errcode);
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

  }while (nbytes > 0);

#undef MAXSIZE
}

void Reactor::SendData(struct bufferevent *buffer_event, void *ctx) {
  // FIXME : No implement now
}

} /* namespace network */

#undef CHECK_STATUS

