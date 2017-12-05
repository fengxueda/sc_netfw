/*
 * Reactor.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_REACTOR_H_
#define SRC_NETWORK_REACTOR_H_

namespace network {

struct EventBase {
  void * wrapper;
  struct event_base* base;
};
typedef struct EventBase EventBase;

class NetWrapper;

class Reactor {
 public:
  Reactor(NetWrapper *wrapper);
  virtual ~Reactor();

 private:
  void RegisterAccptEvent();

  NetWrapper* wrapper_;
};

} /* namespace network */

#endif /* SRC_NETWORK_REACTOR_H_ */
