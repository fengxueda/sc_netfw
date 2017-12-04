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
#include <unordered_map>
#include <condition_variable>

namespace network {

struct EventBase {
  void * wrapper;
  struct event_base* base;
};
typedef struct EventBase EventBase;

static int GetErrorCode(int socket_id);
static void OnAccept(int listen_sd, short event, void *args);
static void OnRead(struct bufferevent *buffer_event, void *ctx);
static void OnWrite(struct bufferevent *buffer_event, void *ctx);
static void OnStatusReport(struct bufferevent *buffer_event, short what,
                           void *ctx);

class Session;

class NetWrapper {
 public:
  NetWrapper();
  virtual ~NetWrapper();

  void TcpServerInit();
  void TcpServerDestory();

  void LibeventInit();
  void LibeventDestory();
  void LibeventRegister();

  void Launch();

  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

 private:
  static const unsigned int kServerPort = 8802;
  static const int kMaxListenCount = 2048;

  int listen_sd_;
  struct event_base* base_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
