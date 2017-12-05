/*
 * NetWrapper.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_NETWRAPPER_H_
#define SRC_NETWORK_NETWRAPPER_H_

#include <memory>
#include <mutex>
#include <thread>
#include <list>
#include <unordered_map>
#include <condition_variable>

namespace network {

class Reactor;
class Session;
class EventDemutiplexor;

class NetWrapper {
 public:
  NetWrapper();
  virtual ~NetWrapper();

  void Launch();
  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

  struct event_base* base() {
    return base_;
  }
  const int listen_sd() const {
    return listen_sd_;
  }

 private:
  void TcpServerInit();
  void TcpServerDestory();
  void CreateReactors();
  void CreateDemutiplexor();

  static const unsigned int kServerPort = 8802;
  static const int kMaxListenCount = 2048;

  int listen_sd_;
  struct event_base* base_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::unique_ptr<Reactor> main_reactor_;
  std::unique_ptr<Reactor> sub_reactor_;
  std::unique_ptr<EventDemutiplexor> event_demutiplexor_;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
