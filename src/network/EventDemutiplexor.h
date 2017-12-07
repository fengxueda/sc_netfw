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

class Session;
class ServiceWorker;
class ServiceMessage;

class EventDemutiplexor {
 public:
  EventDemutiplexor(int thread_count);
  virtual ~EventDemutiplexor();

  void AddCallback(
      const std::function<
          void(const std::shared_ptr<Session>&,
               const std::shared_ptr<ServiceMessage>&)>& callback);
  void PushEventToDispatcher(const std::shared_ptr<Session>& session,
                             const std::shared_ptr<ServiceMessage>& message);

 private:
  void CreateServiceWorkers();
  void RegisterEventDispatcher();

  void EventDispatcher();

  int worker_count_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::queue<std::shared_ptr<ServiceMessage>> messages_;
  std::vector<std::shared_ptr<ServiceWorker>> workers_;
  std::vector<
      std::function<
          void(const std::shared_ptr<Session>&,
               const std::shared_ptr<ServiceMessage>&)>> callbacks_;
};

} /* namespace network */

#endif /* SRC_NETWORK_EVENTDEMUTIPLEXOR_H_ */
