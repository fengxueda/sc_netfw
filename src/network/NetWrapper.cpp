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
#include "EventDemutiplexor.h"
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

NetWrapper::NetWrapper()
    : listen_sd_(-1) {
}

NetWrapper::~NetWrapper() {
  ReleaseComponents();
  DLOG(INFO)<< __FUNCTION__;
}

/* 创建顺序不能变 */
void NetWrapper::Launch() {
  CreateSessionManager();
  CreateReactors();
  CreateDemutiplexor();
  CreateServiceHandler();

  main_reactor_->Start();
  for (auto sub_ractor : sub_reactors_) {
    sub_ractor->Start();
  }

  main_reactor_->Join();
  for (auto sub_reactor : sub_reactors_) {
    sub_reactor->Join();
  }
}

void NetWrapper::CreateReactors() {
  CHECK_NOTNULL(session_manager_.get());
  main_reactor_.reset(new MainReactor(session_manager_.get()));
  CHECK_NOTNULL(main_reactor_.get());
  for (unsigned int index = 0; index < kSubReactorCount; index++) {
    std::shared_ptr<SubReactor> sub_reactor = std::make_shared<SubReactor>(
        session_manager_.get());
    CHECK_NOTNULL(sub_reactor.get());
    main_reactor_->SetNotifyCallback(
        std::bind(&SubReactor::OnNotified, sub_reactor.get(),
                  std::placeholders::_1, std::placeholders::_2));
    sub_reactors_.push_back(sub_reactor);
  }
}

void NetWrapper::CreateDemutiplexor() {
  event_demutiplexor_.reset(new EventDemutiplexor(kThreadCount));
  CHECK_NOTNULL(event_demutiplexor_.get());
  for (auto sub_reactor : sub_reactors_) {
    CHECK_NOTNULL(sub_reactor.get());
    sub_reactor->SetNotifyCallback(
        std::bind(&EventDemutiplexor::PushEventToDispatcher,
                  event_demutiplexor_.get(), std::placeholders::_1,
                  std::placeholders::_2));
  }
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
  event_demutiplexor_->AddCallback(
      std::bind(&plugin::ServiceHandler::OnHandler, service_handler_.get(),
                std::placeholders::_1, std::placeholders::_2));
}

void NetWrapper::ReleaseComponents() {
  session_manager_.reset();
  event_demutiplexor_.reset();
  main_reactor_.reset();
  sub_reactors_.clear();
}

}

/* namespace network */

#undef CHECK_STATUS

