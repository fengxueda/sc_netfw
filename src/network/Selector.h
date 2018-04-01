/*
 * Selector.h
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SELECTOR_H_
#define SRC_NETWORK_SELECTOR_H_

#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <unordered_map>
#include <condition_variable>

struct event_base;
struct event;

namespace network {

class Session;
class SessionManager;

class Selector {
 public:
  enum ListenEventType {
    TYPE_UNDEFINED,
    TYPE_READ,
    TYPE_WRITE,
    TYPE_ACCEPT,
    TYPE_SIGNAL,
  };

  class ListenEvent {
   public:
    ListenEvent()
        : sockfd_(-1),
          type_(TYPE_UNDEFINED) {
    }
    ~ListenEvent() {
    }

    const int sockfd() const {
      return sockfd_;
    }
    void set_sockfd(const int sockfd) {
      sockfd_ = sockfd;
    }
    const int type() const {
      return type_;
    }
    void set_type(const int type) {
      type_ = type;
    }

   private:
    int sockfd_;
    int type_;
  };

  Selector(SessionManager* session_manager);
  virtual ~Selector();

  void Start();
  void Join();

  void SetAcceptedCallback(
      const std::function<void(int, int, void *)>& callback);
  void SetDataRecvCallback(
      const std::function<void(const std::shared_ptr<Session>&, int, void *)>& callback);
  void SetDataSendCallback(
      const std::function<void(const std::shared_ptr<Session>&, int, void *)>& callback);
  void SetSignalCallback(
      const std::function<void(const std::shared_ptr<Session>&, int, void *)>& callback);

  void OnAcceptCallback(int sockfd, int event, void *ctx);
  void OnSignalCallback(const std::shared_ptr<Session>& session, int event,
                        void *ctx);
  void OnDataRecvCallback(const std::shared_ptr<Session>& session, int event,
                          void *ctx);
  void OnDataSendCallback(const std::shared_ptr<Session>& session, int event,
                          void *ctx);

  bool IsExistSession(const std::string& session_id);
  void AddSession(const std::shared_ptr<Session>& session);
  void DeleteSession(const std::string& session_id);
  std::shared_ptr<Session> GetSession(const std::string& session_id);

  void AddEvent(const std::shared_ptr<ListenEvent>& listen_event);
  void DeleteEvent(int sockfd);

 private:
  void SelectorMainloop();

  bool running_;
  std::thread* selector_;
  struct event_base* base_;
  SessionManager* session_manager_;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::map<int, struct event*> events_;

  std::function<void(int, int, void *)> callback_accept_;
  std::function<void(const std::shared_ptr<Session>&, int, void *)> callback_recv_;
  std::function<void(const std::shared_ptr<Session>&, int, void *)> callback_send_;
  std::function<void(const std::shared_ptr<Session>&, int, void *)> callback_signal_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SELECTOR_H_ */
