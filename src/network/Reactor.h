/*
 * Reactor.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_REACTOR_H_
#define SRC_NETWORK_REACTOR_H_

struct event_base;

namespace network {

struct EventBase {
  void * reactor;
  struct event_base* base;
};
typedef struct EventBase EventBase;

class NetWrapper;

class Reactor {
 public:
  enum ReactorType {
    TYPE_UNDEFINED,
    TYPE_MAIN_REACTOR,
    TYPE_SUB_REACTOR,
  };

  Reactor(NetWrapper *wrapper);
  virtual ~Reactor();

  struct event_base* base() const {
    return base_;
  }
  void SetupMainReactor(int listen_sd, Reactor* sub_reactor);
  void SetupSubReactor();
  void CreateListener(int client_sd);

 private:
  void RecvData(struct bufferevent *buffer_event, void *ctx);
  void SendData(struct bufferevent *buffer_event, void *ctx);

  int type_;
  struct event_base *base_;
  NetWrapper* wrapper_;
  Reactor* sub_reactor_;
};

} /* namespace network */

#endif /* SRC_NETWORK_REACTOR_H_ */
