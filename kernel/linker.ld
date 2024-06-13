/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2)) ; /* Dynamic PHDR for relocations */
}

SECTIONS
{
    /* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next memory page for .rodata */
    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    /* C++ is a language that allows for global constructors. In order to obtain the */
    /* address of the ".init_array" section we need to define a symbol for it. */
    .init_array : {
        __init_array = .;
        *(.init_array .init_array.*)
        __init_array_end = .;
    } :rodata

    /* Move to the next memory page for .data */
    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data .data.*)
    } :data

    /* Dynamic section for relocations, both in its own PHDR and inside data PHDR */
    .dynamic : {
        *(.dynamic)
    } :data :dynamic

    .drivers : {
        __driver_array = .;
        *(.drivers .drivers.*)
        __driver_array_end = .;
    } :data

    .kt_commands : {
            __kt_commands_array = .;
            *(.kt_commands .kt_commands.*)
            __kt_commands_array_end = .;
    } :data

    /* NOTE: .bss needs to be the last thing mapped to :data, otherwise lots of */
    /* unnecessary zeros will be written to the binary. */
    /* If you need, for example, .init_array and .fini_array, those should be placed */
    /* above this. */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Include debug sections in the output */
        .debug_aranges    0 : { *(.debug_aranges) }
        .debug_pubnames   0 : { *(.debug_pubnames) }
        .debug_info       0 : { *(.debug_info) }
        .debug_abbrev     0 : { *(.debug_abbrev) }
        .debug_line       0 : { *(.debug_line) }
        .debug_frame      0 : { *(.debug_frame) }
        .debug_str        0 : { *(.debug_str) }
        .debug_loc        0 : { *(.debug_loc) }
        .debug_macinfo    0 : { *(.debug_macinfo) }

        /* Define the section for exception handling (EH) frames */
        .eh_frame_hdr :
        {
            *(.eh_frame_hdr)
        }

        .eh_frame :
        {
            *(.eh_frame)
        }

        /* Define sections for C++ runtime support */
        .gcc_except_table :
        {
            *(.gcc_except_table)
        }

        /* Sections for the C++ global constructors and destructors */
        .init_array :
        {
            PROVIDE_HIDDEN (__init_array_start = .);
            KEEP (*(.init_array))
            KEEP (*(.init_array.*))
            PROVIDE_HIDDEN (__init_array_end = .);
        }

        .fini_array :
        {
            PROVIDE_HIDDEN (__fini_array_start = .);
            KEEP (*(.fini_array))
            KEEP (*(.fini_array.*))
            PROVIDE_HIDDEN (__fini_array_end = .);
        }

        /* Ensure the end of the output file is aligned */
        /DISCARD/ : { *(.note.GNU-stack) }
}
