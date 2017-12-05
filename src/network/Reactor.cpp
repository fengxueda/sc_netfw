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
#include "SessionImpl.h"
#include "Reactor.h"
#include "DataPacket.h"
#include "ServiceEvent.h"
#include "SessionManager.h"
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

static void OnAccept(int listen_sd, short event, void *args);
static void OnRead(struct bufferevent *buffer_event, void *ctx);
static void OnStatusReport(struct bufferevent *buffer_event, short event,
                           void *ctx);

Reactor::Reactor(const SessionManager *session_manager)
    : type_(TYPE_UNDEFINED),
      base_(nullptr),
      running_(false),
      listen_event_(nullptr),
      reactor_thread_(nullptr),
      sub_reactor_(nullptr),
      session_manager_(nullptr) {
  session_manager_ = const_cast<SessionManager*>(session_manager);
  CHECK_NOTNULL(session_manager_);
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

  reactor_thread_ = new std::thread(&Reactor::ReactorMainloop, this);
  CHECK_NOTNULL(reactor_thread_);
}

Reactor::~Reactor() {
  CHECK_NOTNULL(base_);
  if (type_ == TYPE_MAIN_REACTOR) {
    event_del(listen_event_);
  }
  event_base_loopbreak(base_);
  event_base_free(base_);

  reactor_thread_->join();
  delete reactor_thread_;
  reactor_thread_ = nullptr;
  DLOG(INFO)<< __FUNCTION__;
}

void Reactor::Start() {
  running_ = true;
  cond_var_.notify_one();
  DLOG(INFO)<< "Begin to start up reactor.";
}

void Reactor::Join() {
  reactor_thread_->join();
}

void Reactor::SetupMainReactor(int listen_sd, Reactor* sub_reactor) {
  CHECK(type_ == TYPE_UNDEFINED);
  DLOG(INFO)<< "Setup main reactor.";
  std::shared_ptr<EventBase> event_base = std::make_shared<EventBase>();
  event_base->base = base_;
  event_base->reactor = this;
  eventbases_.push_back(event_base);
  listen_event_ = event_new(base_, listen_sd, EV_READ | EV_PERSIST, OnAccept,
                            event_base.get());
  CHECK_NOTNULL(listen_event_);
  event_add(listen_event_, nullptr);
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
  std::shared_ptr<EventBase> event_base = std::make_shared<EventBase>();
  event_base->base = base_;
  event_base->reactor = sub_reactor_;
  eventbases_.push_back(event_base);
  struct bufferevent* buffer_event = bufferevent_socket_new(
      sub_reactor_->base(), client_sd, BEV_OPT_CLOSE_ON_FREE);
  CHECK_NOTNULL(buffer_event);
  bufferevent_setcb(buffer_event, OnRead, nullptr, OnStatusReport,
                    event_base.get());
  bufferevent_enable(buffer_event, EV_READ | EV_PERSIST);
}

void Reactor::AddSession(const std::shared_ptr<Session>& session) {
  session_manager_->AddSession(session);
}

void Reactor::DeleteSession(const std::string& session_id) {
  session_manager_->DeleteSession(session_id);
}

std::shared_ptr<Session> Reactor::GetSession(const std::string& session_id) {
  return session_manager_->GetSession(session_id);
}

void Reactor::SetServiceHandlerCallback(
    const std::function<void(std::shared_ptr<ServiceEvent>&)>& callback) {
  CHECK(type_ == TYPE_SUB_REACTOR);
  handler_callback_ = callback;
}

void Reactor::OnServiceEventHandler(
    std::shared_ptr<ServiceEvent>& service_event) {
  CHECK(type_ == TYPE_SUB_REACTOR);
  handler_callback_(service_event);
}

void Reactor::ReactorMainloop() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_var_.wait(lock, [this] {return running_;});
  DLOG(INFO)<< "Reactor start up successful, come to main loop.";
  event_base_dispatch(base_);
}

static void OnAccept(int listen_sd, short event, void* args) {
  EventBase* event_base = (EventBase *) args;
  CHECK_NOTNULL(event_base);
  CHECK_NOTNULL(event_base->base);
  struct sockaddr_in client;
  socklen_t size = sizeof(client);
  memset(&client, 0, sizeof(client));
  int client_sd = accept(listen_sd, (struct sockaddr *) &client, &size);
  CHECK_STATUS(WARNING, client_sd);
  CHECK(client_sd > 0);

  std::shared_ptr<Session> session = std::make_shared<SessionImpl>();
  session->set_remote_port(ntohs(client.sin_port));
  session->set_remote_ip(inet_ntoa(client.sin_addr));
  session->set_sockfd(client_sd);
  session->set_create_time(GetLocalDate());
  session->set_update_time(GetLocalDate());
  session->set_session_id(
      session->remote_ip() + ":" + std::to_string(session->remote_port()));
  Reactor* reactor = (Reactor *) event_base->reactor;
  CHECK_NOTNULL(reactor);
  reactor->AddSession(session);
  reactor->CreateListener(client_sd);
}

static void OnRead(struct bufferevent *buffer_event, void *ctx) {
  EventBase* event_base = (EventBase *) ctx;
  CHECK_NOTNULL(event_base);
  CHECK_NOTNULL(event_base->base);
  std::shared_ptr<ServiceEvent> service_event =
      std::make_shared<ServiceEvent>();
  service_event->set_type(ServiceEvent::TPYE_READ);
  service_event->set_buffer_event(buffer_event);
  service_event->set_ctx(ctx);
  // TODO : Push the event to thread-pool
  Reactor* reactor = (Reactor *) event_base->reactor;
  reactor->OnServiceEventHandler(service_event);
}

static void OnStatusReport(struct bufferevent *buffer_event, short event,
                           void *ctx) {
  EventBase* event_base = (EventBase *) ctx;
  CHECK_NOTNULL(event_base);
  CHECK_NOTNULL(event_base->base);
  evutil_socket_t sockfd = bufferevent_getfd(buffer_event);
  DLOG(INFO)<< "Socket (" << sockfd << ") has new status to report : ";
  switch (event) {
    case BEV_EVENT_EOF:
      DLOG(INFO)<<"This connection is closed.";
      break;
      case BEV_EVENT_ERROR:
      DLOG(INFO) << "Error occurs , message : "
      << strerror(GetErrorCodeBySocket(sockfd));
      break;
      case BEV_EVENT_TIMEOUT:
      // FIXME : no implement
      break;
      default:
      break;
    }
  bufferevent_free(buffer_event);
  Reactor* reactor = (Reactor *) event_base->reactor;
  CHECK_NOTNULL(reactor);
  reactor->DeleteSession(
      GetIpAdressBySocket(sockfd) + ":" + GetPortBySocket(sockfd));
}

}

/* namespace network */

#undef CHECK_STATUS

