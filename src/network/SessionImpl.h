/*
 * SessionImpl.h
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSIONIMPL_H_
#define SRC_NETWORK_SESSIONIMPL_H_

#include "Session.h"

namespace network {

class SessionImpl : public Session {
 public:
  SessionImpl();
  virtual ~SessionImpl();

  int SendMessage(unsigned char *data, int size);
  int GetErrorStatus();

 private:

};

} /* namespace network */

#endif /* SRC_NETWORK_SESSIONIMPL_H_ */
