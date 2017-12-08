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
class SessionDemutiplexor;
class MessageDemutiplexor;
class SessionManager;

class NetWrapper {
 public:
  NetWrapper();
  virtual ~NetWrapper();

  void Launch();

 private:
  void CreateSessionDemutiplexor();
  void CreateMessageDemutiplexor();
  void CreateServiceHandler();
  void CreateRelationShip();

  static const int kThreadCount = 4;      // FIXME : For debug
  static const int kSubReactorCount = 3;  // FIXME : For debug

  std::unique_ptr<MessageDemutiplexor> message_demutiplexor_;
  std::unique_ptr<SessionDemutiplexor> session_demutiplexor_;
  std::unique_ptr<plugin::ServiceHandler> service_handler_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
