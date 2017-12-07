/*
 * ServiceHandler.cpp
 *
 *  Created on: 2017年12月7日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "ServiceHandler.h"
#include "ServiceMessage.h"
#include "DataPacket.h"

namespace plugin {

ServiceHandler::ServiceHandler() {

}

ServiceHandler::~ServiceHandler() {
  callbacks_.clear();
}

void ServiceHandler::AddPluginCallback(
    const std::function<
        void(const std::shared_ptr<network::Session> &,
             const std::shared_ptr<network::ServiceMessage> &)>& callback) {
  callbacks_.push_back(callback);
}

void ServiceHandler::OnHandler(
    const std::shared_ptr<network::Session>& session,
    const std::shared_ptr<network::ServiceMessage>& message) {
  DLOG(INFO)<<"length " << message->datagram()->length()
  << ", message : " << std::string((char *)message->datagram()->data());
  for (auto callback : callbacks_) {
    callback(session, message);
  }
}

}
/* namespace plugin */
