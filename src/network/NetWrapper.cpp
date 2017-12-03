/*
 * NetWrapper.cpp
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#include <glog/logging.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "NetWrapper.h"
#include "Session.h"

namespace network {

NetWrapper::NetWrapper() {
}

NetWrapper::~NetWrapper() {
}

void NetWrapper::TcpServerInit() {
	int listen_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	CHECK(listen_sd > 0);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(kServerPort);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	CHECK(0 == bind(listen_sd, (struct sockaddr *)&server, sizeof(struct sockaddr)));
	DLOG(INFO) << "Bind port(" << kServerPort << ") successful.";
	listen(listen_sd ,kMaxListenCount);
}

} /* namespace network */
