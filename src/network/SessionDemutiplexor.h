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
class MainReactor;

class SessionDemutiplexor {
 public:
  SessionDemutiplexor();
  virtual ~SessionDemutiplexor();

  void StartUp();
  void AddCallback(
      const std::function<void(const std::shared_ptr<ServiceMessage>&)>& callback);
  void OnPushSession(const std::shared_ptr<Session>& session);
  void OnSessionDispatch();

 private:
  void CreateSessionManager();
  void CreateMainReactor();
  void CreateSubReactors();
  void MakeRelationship();
  void StartUpReactors();

  std::unique_ptr<MainReactor> main_reactor_;
  std::unique_ptr<SessionManager> session_manager_;
  std::map<std::string, std::shared_ptr<SubReactor>> sub_reactors_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::queue<std::shared_ptr<Session>> sessions_;
  std::vector<std::function<void(const std::shared_ptr<ServiceMessage>&)>> callbacks_;
};

}
/* namespace network */

#endif /* SRC_NETWORK_SESSIONDEMUTIPLEXOR_H_ */
