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
#include "Session.h"

namespace plugin {

ServiceHandler::ServiceHandler() {

}

ServiceHandler::~ServiceHandler() {
  callbacks_.clear();
}

void ServiceHandler::AddPluginCallback(
    const std::function<void(const std::shared_ptr<network::ServiceContext> &)>& callback) {
  callbacks_.push_back(callback);
}

void ServiceHandler::OnHandler(
    const std::shared_ptr<network::ServiceContext>& ctx) {
  DLOG(INFO)<<"Datagram length : " << ctx->datagram()->length();
  ctx->session()->SendMessage(ctx->datagram()->data(), ctx->datagram()->length());
  for (auto callback : callbacks_) {
    callback(ctx);
  }
}

}
/* namespace plugin */
