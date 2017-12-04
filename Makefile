ROOT_PATH = $(shell pwd)
SRC_DIR = ${ROOT_PATH}/src
MODULE_DIR = ${SRC_DIR}/storage ${SRC_DIR}/network ${SRC_DIR}/misc
BUILD = ${ROOT_PATH}/build
TARGET = ${ROOT_PATH}/build/libnet-wrapper.a

export ROOT_PATH

	

${TARGET} : all
	@ar -rv $@ $(shell find ${BUILD} -name '*.o')
	@echo 'Build ${TARGET} successful...'
	
all :
	@for module in $(MODULE_DIR); 		\
	do 									\
		$(MAKE) -C $$module;			\
	done							


.PHONY clean :
	@rm -rf ${TARGET} $(shell find ${BUILD} -name '*.o')
	@for module in $(MODULE_DIR); 		\
	do 									\
		$(MAKE) -C $$module $@;			\
	done