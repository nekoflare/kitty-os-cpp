bits 64

section .data
rsp_: dq 0

section .text

global syscall_handler
extern syscall_proper_handler
syscall_handler:
    cli

    ;; Save current stack pointer
    mov qword [rsp_], rsp    

    ;; Save SSE/AVX state
    lea rax, [rsp - 512]  ; Temporary space for SSE state
    fxsave [rax]          ; Save SSE state to the temporary location
    sub rsp, 512          ; Adjust stack pointer to include the state

    swapgs
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

    ;; Pass current stack pointer to syscall_proper_handler
    lea rdi, [rsp]
    call syscall_proper_handler

    ;; Restore general-purpose registers
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

    ;; Restore SSE/AVX state
    add rsp, 512          ; Adjust stack pointer back
    lea rax, [rsp - 512]  ; Point to the saved SSE state
    fxrstor [rax]         ; Restore SSE state

    swapgs

    ;; Restore original stack pointer
    mov rsp, qword [rsp_]

    sti
    o64 sysret
