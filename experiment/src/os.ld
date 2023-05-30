/* 输出文件的体系架构 */
OUTPUT_ARCH( "riscv" )

/* 指定程序运行的第一条指令的入口点的符号名称 */
ENTRY( _start )

/*
 * https://sourceware.org/binutils/docs/ld/MEMORY.html
 * 编译目标的每一块内存区域情况
 * syntax: 
 * MEMORY
 * {
 *     name [(attr)] : ORIGIN = origin, LENGTH = len
 *     ......
 * }
 */
MEMORY
{
	/* 定义一个叫做ram的内存区域 */
	/* writeable executable allocatable !read-only initialized */
	ram   (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 128M
}

/* 描述如何将 input file sections 映射到 output file sections，如何将 output sections 放置在内存中 */
SECTIONS
{
	.text : {
		/* 类似于赋值命令，最终将数值会放在链接结果的符号表中 _text_start 字段 */
		PROVIDE(_text_start = .);
		*(.text .text.*)
		PROVIDE(_text_end = .);
	} >ram

	.rodata : {
		PROVIDE(_rodata_start = .);
		*(.rodata .rodata.*)
		PROVIDE(_rodata_end = .);
	} >ram

	.data : {
		
		. = ALIGN(4096);
		PROVIDE(_data_start = .);
		*(.sdata .sdata.*)
		*(.data .data.*)
		PROVIDE(_data_end = .);
	} >ram

	.bss :{
		PROVIDE(_bss_start = .);
		*(.sbss .sbss.*)
		*(.bss .bss.*)
		*(COMMON)
		PROVIDE(_bss_end = .);
	} >ram

	PROVIDE(_memory_start = ORIGIN(ram));
	PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));

	/* 计算堆内存的起始位置与大小 */
	PROVIDE(_heap_start = _bss_end);
	PROVIDE(_heap_size = _memory_end - _heap_start);
}
