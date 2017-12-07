#sed 's/要被替换的字符串/新的字符串/g'''

ROOT ?= ../..
SRC = $(shell find . -name '*.cpp')
OBJ = $(patsubst %.cpp,%.o,${SRC})  

include ${ROOT}/Makefile.rule
