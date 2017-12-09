# sc_netfw
	This is a server framework that base on libevent.
	The model of this framework is main-sub ractor:
	1 main-reactor, n(n is the number of cpu core)sub-reactor, 1 session-manager and the threadpool. 
	Main-reactor is used for listen event what ever read or write. When the main-reactor catch
	a event, it will push the event to sub-reactor. All the datagrams were received by subreactors.
	After sub-reactor get the message, and it would push the message to threadpool for processing.
	All the application handler should be registered in the ServiceHander.cpp .
	
	# Before use it, you should install libevent and mysql connector(optional).
	$ make
	
	Create a static library at the build/* .