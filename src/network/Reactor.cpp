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
        DLOG(WARNING)<< "The connection is closed. Message : "
        << strerror(GetErrorCode(read_sd));
        bufferevent_free(buffer_event);
        return;
      }
      break;
      case -1: {
        int errcode = GetErrorCode(read_sd);
        if (EAGAIN != errcode) {
          DLOG(WARNING) << "The connection occurs error. Message : "
          << strerror(GetErrorCode(read_sd));
          bufferevent_free(buffer_event);
        }
        // TODO : Push the datagram to queue
        std::shared_ptr<ServiceEvent> service_event = std::make_shared<
        ServiceEvent>();
        service_event->set_datagram(message);
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
      DLOG(INFO) << "Error occurs , message : "
      << strerror(GetErrorCode(socket_id));
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

Reactor::Reactor(NetWrapper *wrapper)
    : wrapper_(nullptr) {
  wrapper_ = wrapper;
  CHECK_NOTNULL(wrapper_);
  RegisterAccptEvent();
}

Reactor::~Reactor() {

}

void Reactor::RegisterAccptEvent() {
  EventBase* event_base = new EventBase();
  event_base->base = wrapper_->base();
  event_base->wrapper = this;
  struct event* listen_event = event_new(wrapper_->base(),
                                         wrapper_->listen_sd(),
                                         EV_READ | EV_PERSIST,
                                         OnAccept, (void *) event_base);
  event_add(listen_event, nullptr);
}

} /* namespace network */

#undef CHECK_STATUS

