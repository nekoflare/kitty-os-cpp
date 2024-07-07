bits 64
section .text
global fuckme
fuckme:
    mov rax, 0x80000000
    int 0x80
    ret