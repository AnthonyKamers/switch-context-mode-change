OUTPUT_ARCH("riscv")

OUTPUT_FORMAT("elf64-littleriscv")

MEMORY
{
	ram (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

ENTRY(_start)

PHDRS {
	text PT_LOAD;
	data PT_LOAD;
	bss PT_LOAD;
}

/* Organiza a memória. Como o código é em assembly, dá de colocar direto qual é a sessão no assembly.
Se fosse em c, teria que fazer uns troços pra dizer qual tipo de variável vai em qual */
SECTIONS
{
	. = ORIGIN(ram);
	
	.text : {
		PROVIDE(_text_start = .);
		*(.text.init) *(.text)
		PROVIDE(_text_end = .);
	/*
	  The portion after the right brace is in an odd format. However, this is telling the
	  linker what memory portion to put it in. We labeled our RAM, ram, with the constraints
	  that it is writeable, allocatable, and executable. The linker will make sure with this
	  that we can do all of those things.

	  >ram - This just tells the linker script to put this entire section (.text) into the
	         ram region of memory. To my knowledge, the '>' does not mean "greater than". Instead,
			 it is a symbol to let the linker know we want to put this in ram.

	  AT>ram - This sets the LMA (load memory address) region to the same thing. LMA is the final
	           translation of a VMA (virtual memory address). With this linker script, we're loading
			   everything into its physical location. We'll let the kernel copy and sort out the 
			   virtual memory. That's why >ram and AT>ram are continually the same thing.

	  :text  - This tells the linker script to put this into the :text program header. We've only
	           defined three: text, data, and bss (PHDRS). In this case, we're telling the linker script
			   to go into the text section.
	*/
	} >ram AT>ram :text	
	
	PROVIDE(_global_pointer = .);

	.rodata	: {
		PROVIDE(_rodata_start = .);
		*(.rodata)
		PROVIDE(_rodata_end = .);
	} >ram AT>ram :text
	
	.data : {
		PROVIDE(_data_start = .);
		*(.stada)
		PROVIDE(_data_end = .);
	} >ram AT>ram :data

	.bss : {
		PROVIDE(_bss_start = .);
		*(.bss)
		PROVIDE(_bss_end = .);
	} >ram AT>ram :bss
	
	PROVIDE(_memory_start = ORIGIN(ram));
	PROVIDE(_stack_start = _bss_end);
	PROVIDE(_stack_end = _stack_start + 0x80000);
	PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));
}
