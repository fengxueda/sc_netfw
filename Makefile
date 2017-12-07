ROOT = $(shell pwd)
SRC_DIR = ${ROOT}/src
MODULE_DIR = ${SRC_DIR}/storage ${SRC_DIR}/network ${SRC_DIR}/misc ${SRC_DIR}/plugin
BUILD = ${ROOT}/build
TARGET = ${ROOT}/build/libnet-wrapper.a

export ROOT

${TARGET} : all
	ar -rv $@ $(shell find ${BUILD} -name '*.o')
	@echo "Build \033[1m\033[32m ${TARGET} \033[0m successful..."
		
all :
	@for module in $(MODULE_DIR); 							\
	do {													\
		$(MAKE) -C $$module || exit "$$?";					\
		};													\
	done							


.PHONY clean :
	@rm -rf ${TARGET} $(shell find ${BUILD} -name '*.o')
	@for module in $(MODULE_DIR); 							\
	do {													\
		$(MAKE) -C $$module $@;								\
		}; 													\
	done
