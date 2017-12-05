/*
 * ServiceEvent.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef INCLUDE_SERVICEEVENT_H_
#define INCLUDE_SERVICEEVENT_H_

#include <memory>
#include <mutex>

struct bufferevent;

namespace network {

class ServiceEvent {
 public:
  enum EventType {
    TPYE_READ = 1,
    TYPE_WRITE = 2,
  };

  ServiceEvent()
      : type_(0),
        ctx_(nullptr),
        buffer_event_(nullptr) {

  }
  virtual ~ServiceEvent() {

  }

  const int type() const {
    return type_;
  }
  void set_type(const int type) {
    type_ = type;
  }
  const void* ctx() const {
    return ctx_;
  }
  void set_ctx(const void* ctx) {
    ctx_ = const_cast<void*>(ctx);
  }
  const struct bufferevent* buffer_event() const {
    return buffer_event_;
  }
  void set_buffer_event(const struct bufferevent* buffer_event) {
    buffer_event_ = const_cast<struct bufferevent*>(buffer_event);
  }

  std::mutex& mutex() const {
    return mutex_;
  }

 private:
  int type_;
  void *ctx_;
  struct bufferevent* buffer_event_;
  mutable std::mutex mutex_;
};

}

#endif /* INCLUDE_SERVICEEVENT_H_ */
