ROOT = ..
CXXFLAG = -std=c++11 -Wall -g -O2 
BUILD = ${ROOT}/build
LIBS = -lnet-wrapper -lmysqlcppconn -levent -pthread -lglog -lgflags -lcrypto -lssl
TARGET = test_server

${TARGET} : ${BUILD}/libnet-wrapper.a main.cpp
	g++ main.cpp -o $@ ${CXXFLAG} -I.. -L${BUILD} ${LIBS}

.PHONY clean :
	rm -rf ${TARGET}
