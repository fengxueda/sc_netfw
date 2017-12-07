/*
 * NetWrapper.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_NETWRAPPER_H_
#define SRC_NETWORK_NETWRAPPER_H_

#include <vector>
#include <memory>

namespace plugin {
class ServiceHandler;
}

namespace network {

class MainReactor;
class SubReactor;
class EventDemutiplexor;
class SessionManager;

class NetWrapper {
 public:
  NetWrapper();
  virtual ~NetWrapper();

  void Launch();

 private:
  void CreateReactors();
  void CreateDemutiplexor();
  void CreateSessionManager();
  void CreateServiceHandler();
  void ReleaseComponents();

  static const int kThreadCount = 4;      // FIXME : For debug
  static const int kSubReactorCount = 3;  // FIXME : For debug

  int listen_sd_;
  std::unique_ptr<MainReactor> main_reactor_;
  std::vector<std::shared_ptr<SubReactor>> sub_reactors_;
  std::unique_ptr<EventDemutiplexor> event_demutiplexor_;
  std::unique_ptr<SessionManager> session_manager_;
  std::unique_ptr<plugin::ServiceHandler> service_handler_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
