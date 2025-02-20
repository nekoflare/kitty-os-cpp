bits 64

section .text

extern interrupt_handler

global load_idt_table
load_idt_table:
    lidt [rdi]
    ret

%macro ISR_NOERRORCODE 1
    global idt_isr%1:
    idt_isr%1:
        push qword 0 ; dummy error code
        push qword %1 ; interrupt number
        jmp isr_common
%endmacro

%macro ISR_ERRORCODE 1
    global idt_isr%1:
    idt_isr%1:
        push qword %1 ; interrupt number
        jmp isr_common
%endmacro

%include 'hal/idt/idt.inc'

global isr_common
isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsp
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rax, rax

    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax

    mov rax, cr8
    push rax
    mov rax, cr4
    push rax
    mov rax, cr3
    push rax
    mov rax, cr2
    push rax
    mov rax, cr0
    push rax
    mov rax, dr7
    push rax
    mov rax, dr6
    push rax
    mov rax, dr5
    push rax
    mov rax, dr4
    push rax
    mov rax, dr3
    push rax
    mov rax, dr2
    push rax
    mov rax, dr1
    push rax
    mov rax, dr0
    push rax

    cld

    ; Push the address of the structure onto the stack
    lea rdi, [rsp]   ; Address of the saved registers
    call interrupt_handler

    pop rax
    mov dr0, rax
    pop rax
    mov dr1, rax
    pop rax
    mov dr2, rax
    pop rax
    mov dr3, rax
    pop rax
    mov dr4, rax
    pop rax
    mov dr5, rax
    pop rax
    mov dr6, rax
    pop rax
    mov dr7, rax
    pop rax
    mov cr0, rax
    pop rax
    mov cr2, rax
    pop rax
    mov cr3, rax
    pop rax
    mov cr4, rax
    pop rax
    mov cr8, rax

    xor rax, rax
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rsp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq


global enable_interrupts
enable_interrupts:
    sti
    ret

global disable_interrupts
disable_interrupts:
    cli
    ret