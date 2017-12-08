/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <functional>
#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "Error.h"
#include "NetWrapper.h"
#include "SessionDemutiplexor.h"
#include "MessageDemutiplexor.h"
#include "SessionManager.h"
#include "MainReactor.h"
#include "SubReactor.h"
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

}

NetWrapper::~NetWrapper() {
  ReleaseComponents();
  DLOG(INFO)<< __FUNCTION__;
}

/* 创建顺序不能变 */
void NetWrapper::Launch() {
  CreateSessionManager();
  CreateMainReactor();
  CreateDemutiplexors();
  CreateServiceHandler();
  CreateRelationShip();

  main_reactor_->Start();
  main_reactor_->Join();
}

void NetWrapper::CreateMainReactor() {
  CHECK_NOTNULL(session_manager_.get());
  main_reactor_.reset(new MainReactor(session_manager_.get()));
  CHECK_NOTNULL(main_reactor_.get());
}

void NetWrapper::CreateDemutiplexors() {
  session_demutiplexor_.reset(
      new SessionDemutiplexor(session_manager_.get(), kThreadCount));
  CHECK_NOTNULL(session_demutiplexor_.get());
  message_demutiplexor_.reset(new MessageDemutiplexor(kThreadCount));
  CHECK_NOTNULL(message_demutiplexor_.get());
  DLOG(INFO)<< "Create demutiplexor successful.";
}

void NetWrapper::CreateSessionManager() {
  session_manager_.reset(new SessionManager());
  CHECK_NOTNULL(session_manager_.get());
  DLOG(INFO)<< "Create Session manager successful.";
}

void NetWrapper::CreateServiceHandler() {
  service_handler_.reset(new plugin::ServiceHandler());
  CHECK_NOTNULL(service_handler_.get());
  DLOG(INFO)<< "Create service handler successful.";
}

void NetWrapper::CreateRelationShip() {
  main_reactor_->AddDataRecvCallback(
      std::bind(&SessionDemutiplexor::OnPushSession,
                session_demutiplexor_.get(), std::placeholders::_1));
  session_demutiplexor_->AddCallback(
      std::bind(&MessageDemutiplexor::OnPushMessage,
                message_demutiplexor_.get(), std::placeholders::_1));
  message_demutiplexor_->AddCallback(
      std::bind(&plugin::ServiceHandler::OnHandler, service_handler_.get(),
                std::placeholders::_1));
}

void NetWrapper::ReleaseComponents() {
  session_manager_.reset();
  session_demutiplexor_.reset();
  message_demutiplexor_.reset();
  main_reactor_.reset();
}

}

/* namespace network */

#undef CHECK_STATUS

