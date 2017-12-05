/*
 * EventDemutiplexor.h
 *
 *  Created on: 2017年12月5日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_EVENTDEMUTIPLEXOR_H_
#define SRC_NETWORK_EVENTDEMUTIPLEXOR_H_

#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

namespace network {

class ServiceWorker;
class ServiceEvent;

class EventDemutiplexor {
 public:
  EventDemutiplexor();
  virtual ~EventDemutiplexor();

  void AddCallback(
      std::function<void(std::shared_ptr<ServiceEvent>&)>& callback);
  void PushEvent(const std::shared_ptr<ServiceEvent>& event);

 private:
  void CreateServiceWorkers();
  void RegisterEventDispatcher();

  void EventDispatcher();

  static const int kThreadCount = 4;

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::queue<std::shared_ptr<ServiceEvent>> events_;
  std::vector<std::shared_ptr<ServiceWorker>> workers_;
  std::vector<std::function<void(std::shared_ptr<ServiceEvent>&)>> callbacks_;
};

} /* namespace network */

#endif /* SRC_NETWORK_EVENTDEMUTIPLEXOR_H_ */
