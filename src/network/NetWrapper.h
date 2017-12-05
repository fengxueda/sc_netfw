/*
 * NetWrapper.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_NETWRAPPER_H_
#define SRC_NETWORK_NETWRAPPER_H_

#include <memory>

namespace network {

class Reactor;
class EventDemutiplexor;
class SessionManager;

class NetWrapper {
 public:
  NetWrapper();
  virtual ~NetWrapper();

  void Launch();

 private:
  void TcpServerInit();
  void TcpServerDestory();

  void CreateReactors();
  void CreateDemutiplexor();
  void CreateSessionManager();
  void ReleaseComponents();

  static const unsigned int kServerPort = 8802; // FIXME : For debug
  static const int kMaxListenCount = 2048;  // FIXME : For debug

  int listen_sd_;
  std::unique_ptr<Reactor> main_reactor_;
  std::unique_ptr<Reactor> sub_reactor_;
  std::unique_ptr<EventDemutiplexor> event_demutiplexor_;
  std::unique_ptr<SessionManager> session_manager_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
