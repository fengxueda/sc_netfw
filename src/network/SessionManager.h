/*
 * SessionManager.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSIONMANAGER_H_
#define SRC_NETWORK_SESSIONMANAGER_H_

#include <thread>
#include <mutex>
#include <unordered_map>

namespace network {

class Session;

class SessionManager {
 public:
  SessionManager();
  virtual ~SessionManager();

  void Stop();
  bool Exist(const std::string& session_id);
  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

 private:
  void MonitorThread();

  bool running_;
  std::thread* monitor_;
  std::mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SESSIONMANAGER_H_ */
