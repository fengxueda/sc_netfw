/*
 * ServiceHandler.h
 *
 *  Created on: 2017年12月7日
 *      Author: xueda
 */

#ifndef SRC_PLUGIN_SERVICEHANDLER_H_
#define SRC_PLUGIN_SERVICEHANDLER_H_

#include <vector>
#include <mutex>
#include <memory>
#include <functional>

namespace network {
class Session;
class ServiceMessage;
}

namespace plugin {

class ServiceHandler {
 public:
  ServiceHandler();
  virtual ~ServiceHandler();

  void SetHandlerCallback(
      const std::function<
          void(const std::shared_ptr<network::Session>&,
               const std::shared_ptr<network::ServiceMessage>&)>& callback);
  void AddPluginCallback(
      const std::function<
          void(const std::shared_ptr<network::Session>&,
               const std::shared_ptr<network::ServiceMessage>&)>& callback);
  void OnHandler(const std::shared_ptr<network::Session>& session,
                 const std::shared_ptr<network::ServiceMessage>& message);

 private:
  std::mutex mutex_;
  std::vector<
      std::function<
          void(const std::shared_ptr<network::Session>&,
               const std::shared_ptr<network::ServiceMessage>&)>>callbacks_;
};

}
/* namespace plugin */

#endif /* SRC_PLUGIN_SERVICEHANDLER_H_ */
