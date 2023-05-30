
# 获取当前目录
PWD = $(shell pwd)
# 源代码文件夹
SOURCE_PATH = ${PWD}/src
# 头文件夹编译器选项
INCS = -I${SOURCE_PATH}/include
# 汇编文件
SRCS_ASM = $(wildcard ${SOURCE_PATH}/asm/*.S)
# c文件
SRCS_C = $(wildcard ${SOURCE_PATH}/c/*.c)
# cpp文件
SRCS_CXX = $(wildcard ${SOURCE_PATH}/cpp/*.cpp)

# 构建文件夹
BUILD_PATH = ${PWD}/build
# 目标文件文件夹
OBJ_PATH = $(BUILD_PATH)/objs
# 反汇编文件文件夹
DISASMS_PATH = $(BUILD_PATH)/disasms

# 目标文件
OBJS_WITHOUT_DIR = $(patsubst %.S, %.o, $(notdir $(SRCS_ASM))) $(patsubst %.c, %.o, $(notdir $(SRCS_C))) $(patsubst %.cpp, %.o, $(notdir $(SRCS_CXX)))
OBJS = $(addprefix ${OBJ_PATH}/, $(OBJS_WITHOUT_DIR))
# 反汇编文件
DISASMS = $(addprefix ${DISASMS_PATH}/, $(patsubst %.o, %.dis, ${OBJS_WITHOUT_DIR}))

# 构建目标
TARGET = $(BUILD_PATH)/os
# 构建目标的elf文件
ELF_TARGET = $(TARGET).elf
# 构建目标的bin文件
BIN_TARGET = $(TARGET).bin
# 链接器脚本
LINKER_SCEIPT = ${SOURCE_PATH}/os.ld

build_prepare:
	@if [ ! -d $(BUILD_PATH) ]; then \
	mkdir -p $(BUILD_PATH); \
	mkdir -p $(DISASMS_PATH); \
	mkdir -p $(OBJ_PATH); \
	fi