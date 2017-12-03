/*
 * SessionManager.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include "SessionManager.h"

namespace network {

SessionManager::SessionManager() :
		running_(true), monitor_(nullptr) {
	monitor_ = new std::thread(&SessionManager::MonitorThread, this);
	CHECK_NOTNULL(monitor_);
}

SessionManager::~SessionManager() {
	running_ = false;
	monitor_->join();
	delete monitor_;
	monitor_ = nullptr;
}

void SessionManager::MonitorThread() {
	while(running_) {

	}
}

} /* namespace network */
