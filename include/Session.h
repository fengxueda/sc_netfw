/*
 * Session.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSION_H_
#define SRC_NETWORK_SESSION_H_

#include <mutex>
#include <unistd.h>

namespace network {

class Session {
 public:
  Session()
      : remote_port_(0),
        sockfd_(-1) {
  }
  virtual ~Session() {
    if (sockfd_ > 0) {
      close(sockfd_);
      sockfd_ = -1;
    }
  }

  virtual void SendMessage(unsigned char* data, int size) = 0;

  const std::string& session_id() const {
    return session_id_;
  }
  void set_session_id(const std::string& session_id) {
    session_id_ = session_id;
  }
  const std::string& create_time() const {
    return create_time_;
  }
  void set_create_time(const std::string& create_time) {
    create_time_ = create_time;
  }
  const std::string& update_time() const {
    return update_time_;
  }
  void set_update_time(const std::string& update_time) {
    update_time_ = update_time;
  }
  const std::string& remote_ip() const {
    return remote_ip_;
  }
  void set_remote_ip(const std::string& remote_ip) {
    remote_ip_ = remote_ip;
  }
  unsigned short remote_port() const {
    return remote_port_;
  }
  void set_remote_port(const short remote_port) {
    remote_port_ = remote_port;
  }
  int sockfd() const {
    return sockfd_;
  }
  void set_sockfd(const int sockfd) {
    sockfd_ = sockfd;
  }

  const std::mutex& mutex() const {
    return mutex_;
  }

 private:
  std::mutex mutex_;
  std::string session_id_;
  std::string create_time_;
  std::string update_time_;
  std::string remote_ip_;
  unsigned short remote_port_;
  int sockfd_;
};

}

#endif //SRC_NETWORK_SESSION_H_

