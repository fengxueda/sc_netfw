/*
 * ServiceWorker.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SERVICEWORKER_H_
#define SRC_NETWORK_SERVICEWORKER_H_

#include <thread>
#include <functional>

namespace network {

class ServiceWorker {
 public:
  ServiceWorker();
  virtual ~ServiceWorker();

  void AddCallback(const std::function<void ()>& callback);

 private:
  void ServiceProcessor();

  bool running_;
  std::thread* worker_;
  std::vector< std::function<void ()> > callbacks_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SERVICEWORKER_H_ */
