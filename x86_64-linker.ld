/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(_start)

/* Define the program headers for proper MMU permissions */
PHDRS
{
    text     PT_LOAD;
    rodata   PT_LOAD;
    data     PT_LOAD;
}

SECTIONS
{
    /* Place the kernel at the topmost 2GiB of the address space */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* Align to next page for .rodata */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .init_array : {
        __init_array = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        __init_array_end = .;
    } :rodata

    /* Align to next page for .data */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)

        symbols_start = .;
        KEEP(*(.dynsym .dynsym.*))
        symbols_end = .;

        string_table_start = .;
        KEEP(*(.dynstr))
        string_table_end = .;
    } :data

    .bss : {
        bss_start = .;  /* Start of bss section */
        *(.bss .bss.*)
        *(COMMON)
        bss_end = .;    /* End of bss section */
    } :data

    /* Preserve exception handling, thread-local storage, and metadata */
    .fini           : { KEEP (*(SORT_NONE(.fini))) }
    .tdata          : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
    .tbss           : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
    .preinit_array  : {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
