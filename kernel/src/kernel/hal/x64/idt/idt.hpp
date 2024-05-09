//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_IDT_HPP
#define KITTY_OS_CPP_IDT_HPP

#include <kstd/kstdio.hpp>

#include <stdint.h>
#include <sys/types.h>

struct IDTR
{
    uint16_t limit;
    uint64_t idt_address;
} __attribute__((packed));

struct GateDescriptor64
{
    uint16_t offset_one;
    uint16_t segment_selector;
    uint8_t ist : 3;
    uint8_t reserved_one : 5;
    uint8_t gate_type : 4;
    uint8_t zero : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_two;
    uint32_t offset_three;
    uint32_t reserved_two;
} __attribute__((packed));

#define IDT_GATE_TYPE_INTERRUPT 0xE
#define IDT_GATE_TYPE_TRAP_GATE 0xF
#define IDT_DPL_RING_0 0
#define IDT_DPL_RING_1 1
#define IDT_DPL_RING_2 2
#define IDT_DPL_RING_3 3
#define IDT_PRESENT 1
#define IDT_NO_IST 0
#define IDT_DEFAULT_SEGMENT 0x8

constexpr GateDescriptor64 IDT_ENTRY(
        void(*offset)(),
        uint16_t segment_selector,
        uint8_t gate_type,
        uint8_t dpl_layer,
        uint8_t is_present,
        uint8_t ist )
{
    return {
        .offset_one = static_cast<uint16_t>(reinterpret_cast<uint64_t>(offset) & 0xFFFF),
        .segment_selector = segment_selector,
        .ist = ist,
        .reserved_one = 0,
        .gate_type = gate_type,
        .zero = 0,
        .dpl = dpl_layer,
        .present = is_present,
        .offset_two = static_cast<uint16_t>((reinterpret_cast<uint64_t>(offset) & 0xFFFF0000) >> 16),
        .offset_three = static_cast<uint32_t>((reinterpret_cast<uint64_t>(offset) & 0xFFFFFFFF00000000) >> 32),
        .reserved_two = 0
    };
}

struct Registers_x86_64
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rsp, rdx, rcx, rbx, rax, interrupt_number, error_code, rip, cs, rflags, orig_rsp, ss;
} __attribute__((packed));

extern volatile GateDescriptor64 idt[256];

void flush_idt();
extern "C" void enable_interrupts();
extern "C" void disable_interrupts();
extern "C" void flush_idt_asm(IDTR* idtr);
bool hook_interrupt(size_t index, void (*new_interrupt_handler)(Registers_x86_64*));

#endif //KITTY_OS_CPP_IDT_HPP
