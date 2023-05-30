include toolchains.mk
include project.mk

.DEFAULT_GOAL := all
all: build_prepare ${ELF_TARGET}

${TARGET}.elf: ${OBJS}
	${CC} ${CFLAGS} ${INCS} -T ${LINKER_SCEIPT} -o ${ELF_TARGET} $^
	${OBJCOPY} -O binary ${ELF_TARGET} ${BIN_TARGET}

${OBJ_PATH}/%.o: ${SOURCE_PATH}/asm/%.S
	${CC} ${CFLAGS} ${INCS} -c -o $@ $<

${OBJ_PATH}/%.o: ${SOURCE_PATH}/c/%.c
	${CC} ${CFLAGS} ${INCS} -D__FILE__="\"$(notdir $<)\"" -c -o $@ $<

${DISASMS_PATH}/%.dis: ${OBJ_PATH}/%.o
	${OBJDUMP} -S $< >> $@

.PHONY : run_no_build
run_no_build:
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "------------------------------------"
	@${QEMU} ${QFLAGS} -kernel ${ELF_TARGET}

.PHONY : run
run: all run_no_build

.PHONY : debug
debug: all
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel ${ELF_TARGET} -s -S &
	@${GDB} ${ELF_TARGET} -q -x ../gdbinit

.PHONY : debug_no_gdb
debug_no_gdb: all
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel ${ELF_TARGET} -s -S

.PHONY : code
code: all
	@${OBJDUMP} -S ${ELF_TARGET} | less

.PHONY : dis
dis: ${DISASMS}

.PHONY : clean
clean:
	rm -rf build/

