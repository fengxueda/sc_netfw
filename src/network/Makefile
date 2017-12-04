#sed 's/要被替换的字符串/新的字符串/g'''

CC = g++
ROOT_PATH ?= ../..
SRC = $(shell find . -name '*.cpp')
OBJ = $(patsubst %.cpp,%.o,${SRC})  
CCFLAGS ?= -std=c++11 -Wall -g -O2
INCLUDE ?= -I${ROOT_PATH}/include

all : ${OBJ}
${OBJ} :%.o : %.cpp
	${CC} -c ${CCFLAGS} $< -o $@ ${INCLUDE}
	@cp $@ ${ROOT_PATH}/build

.PHONY clean : 
	@rm -rf ${OBJ} ${TARGET}
