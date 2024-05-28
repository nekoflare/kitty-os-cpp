bits 64

section .text

global invlpg
invlpg:
    invlpg [rdi]
    ret