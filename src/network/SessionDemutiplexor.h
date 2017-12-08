/*
 * SessionDemutiplexor.h
 *
 *  Created on: 2017年12月8日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSIONDEMUTIPLEXOR_H_
#define SRC_NETWORK_SESSIONDEMUTIPLEXOR_H_

#include <mutex>
#include <queue>
#include <memory>
#include <map>
#include <condition_variable>
#include <functional>

namespace network {

class Session;
class SubReactor;
class SessionManager;
class ServiceMessage;

class SessionDemutiplexor {
 public:
  SessionDemutiplexor(SessionManager* session_manager, int thread_count);
  virtual ~SessionDemutiplexor();

  void AddCallback(
      const std::function<void(const std::shared_ptr<ServiceMessage>&)>& callback);
  void OnPushSession(const std::shared_ptr<Session>& session);
  void OnSessionDispatch();

 private:
  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::queue<std::shared_ptr<Session>> sessions_;
  std::map<std::string, std::shared_ptr<SubReactor>> sub_reactors_;
  std::vector<std::function<void(const std::shared_ptr<ServiceMessage>&)>> callbacks_;
};

}
/* namespace network */

#endif /* SRC_NETWORK_SESSIONDEMUTIPLEXOR_H_ */
