/*
 * ServiceWorker.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SERVICEWORKER_H_
#define SRC_NETWORK_SERVICEWORKER_H_

#include <thread>

namespace network {

class ServiceWorker {
 public:
  ServiceWorker(void *ctx);
  virtual ~ServiceWorker();

 private:
  void ServiceProcessor();

  bool running_;
  std::thread* worker_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SERVICEWORKER_H_ */
