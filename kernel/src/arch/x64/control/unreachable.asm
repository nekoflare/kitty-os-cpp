bits 64

section .text

global unreachable

unreachable:
    cli
    hlt
.spin: jmp .spin
