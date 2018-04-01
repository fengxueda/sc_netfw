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

extern int errno;

namespace network {

SessionImpl::SessionImpl() {
}

SessionImpl::~SessionImpl() {

}

void SessionImpl::SendMessage(unsigned char* data, int size) {
  CHECK_NOTNULL(data);

  int offset = 0;
  do {
    int nbytes = 0;
    if (Session::sockfd() < 0) {
      break;
    }
    nbytes = write(Session::sockfd(), data + offset, size - offset);
    if (nbytes == -1 && errno == EAGAIN) {
      continue;
    } else if (nbytes == -1 && errno != EAGAIN) {
      break;
    }
    offset += nbytes;
  } while(offset < size);
}

} /* namespace network */

