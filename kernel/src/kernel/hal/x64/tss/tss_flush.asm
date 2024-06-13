bits 64
section .text
global tss_flush_impl
tss_flush_impl:
    mov ax, 0x48
    ltr ax
    ret