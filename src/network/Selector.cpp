/*
 * Selector.cpp
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "Selector.h"
#include "Session.h"
#include "SessionManager.h"
#include "Utils.h"

namespace network {

static void OnDataRecv(evutil_socket_t sockfd, short event, void *ctx);
static void OnDataSend(evutil_socket_t sockfd, short event, void *ctx);
static void OnAccept(evutil_socket_t sockfd, short event, void *ctx);
static void OnSignal(evutil_socket_t sockfd, short event, void *ctx);

Selector::Selector(SessionManager* session_manager)
    : running_(false),
      selector_(nullptr),
      base_(nullptr),
      session_manager_(nullptr) {
  CHECK_NOTNULL(session_manager);
  session_manager_ = session_manager;
  struct event_config* config = event_config_new();
  CHECK_NOTNULL(config);
  event_config_avoid_method(config, "select");
  event_config_require_features(config, EV_FEATURE_ET);
  base_ = event_base_new_with_config(config);
  event_config_free(config);
  DLOG(INFO)<< "Current method of I/O checking : "<< event_base_get_method(base_);
  /*
   * 可选设置优先级数目，然后通过event_priority_set设置事件的优先级
   * 0为最高，n_priority-1为最低，此后创建的事件默认优先级为中间优先级
   */
  event_base_priority_init(base_, 3);
  selector_ = new std::thread(&Selector::SelectorMainloop, this);
  CHECK_NOTNULL(selector_);
}

Selector::~Selector() {
  running_ = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    LOG(INFO) << "events.size() = " << events_.size();
    for (auto event : events_) {
      event_del(event.second);
      event_free(event.second);
    }
    events_.clear();
  }
  event_base_loopbreak(base_);
  event_base_free(base_);
  if (selector_->joinable()) {
    selector_->join();
  }
  delete selector_;
  selector_ = nullptr;
  DLOG(INFO)<< __FUNCTION__;
}

void Selector::Start() {
  running_ = true;
  cond_var_.notify_one();
  DLOG(INFO)<< "Try to start up selector.";
}

void Selector::Join() {
  selector_->join();
}

void Selector::SetAcceptedCallback(
    const std::function<void(int, int, void*)>& callback) {
  callback_accept_ = callback;
}

void Selector::SetDataRecvCallback(
    const std::function<void(const std::shared_ptr<Session> &, int, void*)>& callback) {
  callback_recv_ = callback;
}

void Selector::SetDataSendCallback(
    const std::function<void(const std::shared_ptr<Session> &, int, void*)>& callback) {
  callback_send_ = callback;
}

void Selector::SetSignalCallback(
    const std::function<void(const std::shared_ptr<Session> &, int, void*)>& callback) {
  callback_signal_ = callback;
}

void Selector::OnAcceptCallback(int sockfd, int event, void* ctx) {
  callback_accept_(sockfd, event, ctx);
}

void Selector::OnSignalCallback(const std::shared_ptr<Session>& session, int event,
                                void* ctx) {
  callback_signal_(session, event, ctx);
}

void Selector::OnDataRecvCallback(const std::shared_ptr<Session>& session, int event,
                                  void* ctx) {
  switch (event & 0xF0) {
    case EV_ET:
      DeleteEvent(session->sockfd());
      break;
    default:
      break;
  }
  callback_recv_(session, event, ctx);
}

void Selector::OnDataSendCallback(const std::shared_ptr<Session>& session, int event,
                                  void* ctx) {
  callback_send_(session, event, ctx);
}

bool Selector::IsExistSession(const std::string& session_id) {
  return session_manager_->Exist(session_id);
}

void Selector::AddSession(const std::shared_ptr<Session>& session) {
  session_manager_->AddSession(session);
}

void Selector::DeleteSession(const std::string& session_id) {
  session_manager_->DeleteSession(session_id);
}
std::shared_ptr<Session> Selector::GetSession(const std::string& session_id) {
  return session_manager_->GetSession(session_id);
}

void Selector::AddEvent(const std::shared_ptr<ListenEvent>& listen_event) {
  if (listen_event->type() < TYPE_UNDEFINED
      || listen_event->type() > TYPE_SIGNAL) {
    return;
  }
  struct event* event = nullptr;
  // TODO : You have to lock before to use the event operation interface.(e.g. event_new, event_add)
  // Because the libevent is not thread safety.
  std::lock_guard<std::mutex> lock(mutex_);
  switch (listen_event->type()) {
    case TYPE_READ: {
      event = event_new(base_, listen_event->sockfd(), EV_READ | EV_ET,
                        OnDataRecv, this);
    }
      break;
    case TYPE_WRITE: {
      event = event_new(base_, listen_event->sockfd(), EV_WRITE | EV_ET,
                        OnDataSend, this);
    }
      break;
    case TYPE_ACCEPT: {
      event = event_new(base_, listen_event->sockfd(), EV_READ | EV_PERSIST,
                        OnAccept, this);
    }
      break;
    case TYPE_SIGNAL: {
      event = event_new(base_, listen_event->sockfd(), EV_SIGNAL | EV_PERSIST,
                        OnSignal, this);
    }
      break;
    default:
      break;
  }
  CHECK_NOTNULL(event);
  CHECK(0 == event_add(event, nullptr));
  if (events_.end() == events_.find(listen_event->sockfd())) {
//    DLOG(INFO)<< "Add event : type = " << listen_event->type() << ", sockfd = "
//    << listen_event->sockfd();
    events_[listen_event->sockfd()] = event;
  }
}

void Selector::DeleteEvent(int sockfd) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = events_.find(sockfd);
  if (iter != events_.end()) {
    CHECK(0 == event_del(events_[sockfd]));
    event_free(events_[sockfd]);
    events_.erase(iter);
  }
}

void Selector::ReleaseConnection(const std::shared_ptr<Session>& session) {
  evutil_closesocket(session->sockfd());
  DeleteSession(session->session_id());
}

void Selector::SelectorMainloop() {
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  cond_var_.wait(lock, [this] {return running_;});
  DLOG(INFO)<< "Selector ["<< this << "] start up successful, come to main loop.";
  /* There is some bug of using the api : event_base_dispatch */
//  event_base_dispatch(base_);
  // Using event_base_loop now , flag was set to EVLOOP_NO_EXIT_ON_EMPTY
  event_base_loop(base_, EVLOOP_NO_EXIT_ON_EMPTY);
  DLOG(INFO)<< "Selector ["<< this << "] exit main loop.";
}

static void OnDataRecv(evutil_socket_t sockfd, short event, void *ctx) {
  Selector* selector = (Selector *) ctx;
  std::shared_ptr<Session> session = selector->GetSession(
      GetIpAdressBySocket(sockfd) + ":" + GetPortBySocket(sockfd));
  CHECK_NOTNULL(session.get());
  selector->OnDataRecvCallback(session, event, ctx);
}

static void OnDataSend(evutil_socket_t sockfd, short event, void *ctx) {
  Selector* selector = (Selector *) ctx;
  std::shared_ptr<Session> session = selector->GetSession(
      GetIpAdressBySocket(sockfd) + ":" + GetPortBySocket(sockfd));
  selector->OnDataSendCallback(session, event, ctx);
}

static void OnAccept(evutil_socket_t sockfd, short event, void *ctx) {
  Selector* selector = (Selector *) ctx;
  selector->OnAcceptCallback(sockfd, event, ctx);
}

static void OnSignal(evutil_socket_t sockfd, short event, void *ctx) {
  Selector* selector = (Selector *) ctx;
  std::shared_ptr<Session> session = selector->GetSession(
      GetIpAdressBySocket(sockfd) + ":" + GetPortBySocket(sockfd));
  selector->OnSignalCallback(session, event, ctx);
}

} /* namespace network */

