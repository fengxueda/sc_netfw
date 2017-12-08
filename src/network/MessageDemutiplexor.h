/*
 * MessageDemutiplexor.h
 *
 *  Created on: 2017年12月8日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_MESSAGEDEMUTIPLEXOR_H_
#define SRC_NETWORK_MESSAGEDEMUTIPLEXOR_H_

#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

namespace network {

class ServiceWorker;
class ServiceMessage;

class MessageDemutiplexor {
 public:
  MessageDemutiplexor();
  virtual ~MessageDemutiplexor();

  void StartUp();
  void OnPushMessage(const std::shared_ptr<ServiceMessage>& message);
  void AddCallback(
      const std::function<void(const std::shared_ptr<ServiceMessage>&)>& callback);

 private:
  void CreateServiceWorkers();
  void RegisterMessageDispatcher();
  void OnMessageDispatch();

  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::vector<std::shared_ptr<ServiceWorker>> workers_;
  std::queue<std::shared_ptr<ServiceMessage>> messages_;
  std::vector<std::function<void(const std::shared_ptr<ServiceMessage>&)>> callbacks_;
};

} /* namespace network */

#endif /* SRC_NETWORK_MESSAGEDEMUTIPLEXOR_H_ */
