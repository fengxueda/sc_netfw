/*
 * SubReactor.cpp
 *
 *  Created on: 2017年12月6日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "SubReactor.h"
#include "Selector.h"
#include "Session.h"

namespace network {

SubReactor::SubReactor(SessionManager* session_manager)
    : Reactor(session_manager) {
  CHECK_NOTNULL(session_manager);
  selector_.reset(new Selector(session_manager));
  selector_->SetDataRecvCallback(
      std::bind(&SubReactor::OnDataRecv, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
}

SubReactor::~SubReactor() {

}

void SubReactor::SetNotifyCallback(
    const std::function<void(const std::shared_ptr<Session> &)>& callback) {
  notify_callback_ = callback;
}

void SubReactor::SetNotifiedCallback(
    const std::function<void(const std::shared_ptr<Session> &)>& callback) {
  notified_callback_ = callback;
}

void SubReactor::OnNotify(const std::shared_ptr<Session>& session) {
  notify_callback_(session);
}

/* SubReactor增加一个读事件 */
void SubReactor::OnNotified(const std::shared_ptr<Session>& session) {
  std::shared_ptr<Selector::ListenEvent> event = std::make_shared<
      Selector::ListenEvent>();
  event->set_sockfd(session->sockfd());
  event->set_type(Selector::TYPE_READ);
  selector_->AddEvent(event);
  notified_callback_(session);
}

void SubReactor::OnDataRecv(std::shared_ptr<Session>& session, int event,
                            void* ctx) {
  // TODO : Read data from remote

}

} /* namespace network */
