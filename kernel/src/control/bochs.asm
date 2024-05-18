section .text

global bochs_breakpoint
bochs_breakpoint:
    xchg bx, bx
    ret