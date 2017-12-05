/*
 * SessionImpl.cpp
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include "SessionImpl.h"
#include "Utils.h"

namespace network {

SessionImpl::SessionImpl() {

}

SessionImpl::~SessionImpl() {
}

int SessionImpl::SendMessage(unsigned char* data, int size) {
  CHECK_NOTNULL(data);
  return write(sockfd(), data, size);
}

int SessionImpl::GetErrorStatus() {
  return GetErrorCodeBySocket(sockfd());
}

} /* namespace network */
