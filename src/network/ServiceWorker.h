/*
 * ServiceWorker.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SERVICEWORKER_H_
#define SRC_NETWORK_SERVICEWORKER_H_

#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

namespace network {

class ServiceWorker {
 public:
  ServiceWorker();
  virtual ~ServiceWorker();

  void Stop();
  void AddCallback(const std::function<void ()>& callback);

 private:
  void ServiceProcessor();

  bool running_;
  std::thread* worker_;
  std::vector< std::function<void ()> > callbacks_;

  std::mutex mutex_;
  std::condition_variable cond_var_;

};

} /* namespace network */

#endif /* SRC_NETWORK_SERVICEWORKER_H_ */
