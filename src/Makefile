#sed 's/要被替换的字符串/新的字符串/g'''

CC = g++
ROOT_PATH ?= ..
SRC = $(shell find ${ROOT_PATH} -name '*.cpp')
OBJ = $(patsubst %.cpp,%.o,${SRC})  
CCFLAGS = -std=c++11 -Wall -g -O2
INCLUDE = -I${ROOT_PATH}/include
TARGET = ${ROOT_PATH}/build/libnet-wrapper.a

${TARGET} : ${OBJ}
	ar -rv $@ $^

%.o : %.cpp
	${CC} -c ${CCFLAGS} $< -o $@ ${INCLUDE}

.PHONY clean : 
	rm -rf ${OBJ} ${TARGET}
