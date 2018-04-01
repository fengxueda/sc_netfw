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
class ServiceContext;
}

namespace plugin {

class ServiceHandler {
 public:
  ServiceHandler();
  virtual ~ServiceHandler();

  void AddPluginCallback(
      const std::function<void(const std::shared_ptr<network::ServiceContext>&)>& callback);
  void OnHandler(const std::shared_ptr<network::ServiceContext>& ctx);

 private:
  std::mutex mutex_;
  std::vector<
      std::function<void(const std::shared_ptr<network::ServiceContext>&)>> callbacks_;
};

}
/* namespace plugin */

#endif /* SRC_PLUGIN_SERVICEHANDLER_H_ */
