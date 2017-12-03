#sed 's/要被替换的字符串/新的字符串/g'''

CC = g++
SRC = $(shell find . -name '*.cpp')
OBJ = $(patsubst %.cpp,%.o,${SRC})  
CCFLAGS = -std=c++11 -Wall -g -O2
ROOT_PATH = ..
TARGET = $(ROOT_PATH)/build/libnet.a

${TARGET} : ${OBJ}
	ar -rv $@ $^
	@rm -rf ${OBJ}

${OBJ} : ${SRC}
	${CC} -c ${CCFLAGS} $< -o $@

.PHONY clean : 
	rm -rf ${OBJ} ${TARGET}
