/*
 * SessionImpl.cpp
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include "SessionImpl.h"
#include "Utils.h"

namespace network {

SessionImpl::SessionImpl()
    : buffer_event_(nullptr) {
}

SessionImpl::~SessionImpl() {
  if (buffer_event_ != nullptr) {
    bufferevent_free(buffer_event_);
    buffer_event_ = nullptr;
  }
}

void SessionImpl::SetBufferEvent(const struct bufferevent* buffer_event) {
  CHECK_NOTNULL(buffer_event);
  buffer_event_ = const_cast<struct bufferevent*>(buffer_event);
}

void SessionImpl::SendMessage(unsigned char* data, int size) {
  CHECK_NOTNULL(data);
  CHECK(write(sockfd(), data, size) > 0);
}

} /* namespace network */

