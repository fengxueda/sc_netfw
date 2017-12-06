/*
 * Reactor.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_REACTOR_H_
#define SRC_NETWORK_REACTOR_H_

#include <mutex>
#include <thread>
#include <list>
#include <condition_variable>
#include <functional>

struct event_base;

namespace network {

class ReactorBase;
class Session;
class SessionManager;
class ServiceEvent;

class Reactor {
 public:
  enum ReactorType {
    TYPE_UNDEFINED,
    TYPE_MAIN_REACTOR,
    TYPE_SUB_REACTOR,
  };

  Reactor(const SessionManager *session_manager);
  virtual ~Reactor();

  ReactorBase* reactor_base() const {
    return reactor_base_.get();
  }
  void Start();
  void Join();
  void SetupMainReactor(int listen_sd, ReactorBase* sub_reactor_base);
  void SetupSubReactor();
  void CreateEventMonitor(const std::shared_ptr<Session>& session);

  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

  void SetServiceHandlerCallback(
      const std::function<void(std::shared_ptr<ServiceEvent>&)>& callback);
  void OnServiceEventHandler(std::shared_ptr<ServiceEvent>& service_event);

 private:
  void ReactorMainloop();

  int type_;
  std::unique_ptr<ReactorBase> reactor_base_;
  ReactorBase* sub_reactor_base_;

  bool running_;
  struct event* event_;
  std::thread* reactor_thread_;
  SessionManager *session_manager_;
  std::function<void(std::shared_ptr<ServiceEvent>&)> handler_callback_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
};

class ReactorBase {
 public:
  ReactorBase()
      : reactor_(0),
        event_base_(nullptr) {
  }
  virtual ~ ReactorBase() {
  }

  void* reactor() const {
    return reactor_;
  }
  void set_reactor(const void* reactor) {
    CHECK_NOTNULL(reactor);
    reactor_ = const_cast<void*>(reactor);
  }
  struct event_base* event_base() const {
    return event_base_;
  }
  void set_event_base(const struct event_base* event_base) {
    CHECK_NOTNULL(event_base);
    event_base_ = const_cast<struct event_base*>(event_base);
  }

  void * reactor_;
  struct event_base* event_base_;
};

} /* namespace network */

#endif /* SRC_NETWORK_REACTOR_H_ */
