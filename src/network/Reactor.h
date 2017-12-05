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

struct event_base;

namespace network {

struct EventBase {
  void * reactor;
  struct event_base* base;
};
typedef struct EventBase EventBase;

class Session;
class SessionManager;

class Reactor {
 public:
  enum ReactorType {
    TYPE_UNDEFINED,
    TYPE_MAIN_REACTOR,
    TYPE_SUB_REACTOR,
  };

  Reactor(const SessionManager *session_manager);
  virtual ~Reactor();

  struct event_base* base() const {
    return base_;
  }
  void Start();
  void Join();
  void SetupMainReactor(int listen_sd, Reactor* sub_reactor);
  void SetupSubReactor();
  void CreateListener(int client_sd);

  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

 private:
  void RecvData(struct bufferevent *buffer_event, void *ctx);

  void ReactorMainloop();

  int type_;
  struct event_base *base_;

  bool running_;
  std::thread* reactor_thread_;
  Reactor* sub_reactor_;
  SessionManager *session_manager_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::list<std::shared_ptr<EventBase>> eventbases_;
};

} /* namespace network */

#endif /* SRC_NETWORK_REACTOR_H_ */
