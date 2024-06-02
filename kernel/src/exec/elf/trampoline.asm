bits 64

section .text

global elf_trampoline

elf_trampoline:
    mov rax, cr3
    and rax, 0xfff
    or rax, rdi
    mov cr3, rax

    xchg bx, bx
    jmp rsi

    ret