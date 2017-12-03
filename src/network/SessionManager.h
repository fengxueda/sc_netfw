/*
 * SessionManager.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_SESSIONMANAGER_H_
#define SRC_NETWORK_SESSIONMANAGER_H_

#include <thread>

namespace network {

class SessionManager {
public:
	SessionManager();
	virtual ~SessionManager();

private:
	void MonitorThread();

	bool running_;
	std::thread* monitor_;
};

} /* namespace network */

#endif /* SRC_NETWORK_SESSIONMANAGER_H_ */
