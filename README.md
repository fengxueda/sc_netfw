# sc_netfw
	This is a server framework that base on libevent.
	The model of this framework is main-sub ractor:
	1 main-reactor, n(n is the number of cpu core)sub-reactor, 1 session-manager and the threadpool(the number of cpu core). 
	Main-reactor is used for listen event what ever read or write. When the main-reactor catch
	a event, it will push the event to sub-reactor. All the datagrams were received by subreactors.
	After sub-reactor get the message, and it would push the message to threadpool for processing.
	Send message via session.
	All the application handler should be registered in the ServiceHander.cpp .
	
	# Before use it, you should install libevent , glog & gflags,  mysql connector(optional).
	$ make
	
	Create a static library at the build/* .
	
	这个是我个人写的一个网络框架，基于libevent.
	模型是“主从式Reactor”模式:
	总共有一个主Reactor，n个从Reactor(n是CPU核心数)，一个Session管理器和线程池(线程池的个数目前取cpu核心个数)
	主Rector用于监听事件，从Reactor用于读操作，把数据读取出来之后通过队列推给线程池处理，线程池处理完毕之后通过session发送.
	所有的应用处理接口，要注册到ServciceHandler下面.