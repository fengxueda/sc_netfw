/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <functional>
#include <glog/logging.h>
#include "Error.h"
#include "NetWrapper.h"
#include "SessionDemutiplexor.h"
#include "MessageDemutiplexor.h"
#include "plugin/ServiceHandler.h"

#define CHECK_STATUS(_level_,_expr_)                                                \
    do {                                                                            \
    if ((_expr_) < 0) {                                                             \
      DLOG(_level_) << "Error code: " << errno << " message : " << strerror(errno); \
      return ;                                                                      \
  }                                                                                 \
} while(0)

namespace network {

NetWrapper::NetWrapper() {
  CreateSessionDemutiplexor();
  CreateMessageDemutiplexor();
  CreateServiceHandler();
  CreateRelationShip();
}

NetWrapper::~NetWrapper() {
  DLOG(INFO)<< __FUNCTION__;
}

void NetWrapper::Launch() {
  // TODO : Message demutiplexor must startup before session demutiplexor
  message_demutiplexor_->StartUp();
  session_demutiplexor_->StartUp();
}

void NetWrapper::CreateSessionDemutiplexor() {
  session_demutiplexor_.reset(new SessionDemutiplexor());
  CHECK_NOTNULL(session_demutiplexor_.get());
  DLOG(INFO)<< "Create session demutiplexor successful.";
}

void NetWrapper::CreateMessageDemutiplexor() {
  message_demutiplexor_.reset(new MessageDemutiplexor());
  CHECK_NOTNULL(message_demutiplexor_.get());
  DLOG(INFO)<< "Create message demutiplexor successful.";
}

void NetWrapper::CreateServiceHandler() {
  service_handler_.reset(new plugin::ServiceHandler());
  CHECK_NOTNULL(service_handler_.get());
  DLOG(INFO)<< "Create service handler successful.";
}

void NetWrapper::CreateRelationShip() {
  session_demutiplexor_->AddCallback(
      std::bind(&MessageDemutiplexor::OnPushMessage,
                message_demutiplexor_.get(), std::placeholders::_1));
  message_demutiplexor_->AddCallback(
      std::bind(&plugin::ServiceHandler::OnHandler, service_handler_.get(),
                std::placeholders::_1));
}

}

/* namespace network */

#undef CHECK_STATUS

