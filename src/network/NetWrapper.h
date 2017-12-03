/*
 * NetWrapper.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef SRC_NETWORK_NETWRAPPER_H_
#define SRC_NETWORK_NETWRAPPER_H_

#include <memory>
#include <unordered_map>

namespace network {

class Session;

class NetWrapper {
public:
	NetWrapper();
	virtual ~NetWrapper();

	void TcpServerInit();

private:
	static const unsigned int kServerPort = 8802;
	static const int kMaxListenCount = 2048;

	std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};

} /* namespace network */

#endif /* SRC_NETWORK_NETWRAPPER_H_ */
