//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_IDT_HPP
#define KITTY_OS_CPP_IDT_HPP

#include <kstd/kstdint.hpp>
#include <kstd/kstdio.hpp>

struct IDTR
{
    kstd::uint16_t limit;
    kstd::uint64_t idt_address;
} __attribute__((packed));

struct GateDescriptor64
{
    unsigned short offset_one;
    unsigned short segment_selector;
    unsigned char ist : 3;
    unsigned char reserved_one : 5;
    unsigned char gate_type : 4;
    unsigned char zero : 1;
    unsigned char dpl : 2;
    unsigned char present : 1;
    unsigned short offset_two;
    unsigned long offset_three;
    unsigned long reserved_two;
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
        kstd::uint16_t segment_selector,
        kstd::uint8_t gate_type,
        kstd::uint8_t dpl_layer,
        kstd::uint8_t is_present,
        kstd::uint8_t ist )
{
    return {
        .offset_one = static_cast<unsigned short>(reinterpret_cast<kstd::uint64_t>(offset) & 0xFFFF),
        .segment_selector = segment_selector,
        .ist = ist,
        .reserved_one = 0,
        .gate_type = gate_type,
        .zero = 0,
        .dpl = dpl_layer,
        .present = is_present,
        .offset_two = static_cast<unsigned short>((reinterpret_cast<kstd::uint64_t>(offset) & 0xFFFF0000) >> 16),
        .offset_three = static_cast<unsigned long>((reinterpret_cast<kstd::uint64_t>(offset) & 0xFFFFFFFF00000000) >> 32),
        .reserved_two = 0
    };
}

struct Registers_x86_64
{
    kstd::uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rsp, rdx, rcx, rbx, rax, interrupt_number, error_code, rip, cs, rflags, orig_rsp, ss;
} __attribute__((packed));

extern volatile GateDescriptor64 idt[256];

void flush_idt();
extern "C" void enable_interrupts();
extern "C" void disable_interrupts();
extern "C" void flush_idt_asm(IDTR* idtr);

#endif //KITTY_OS_CPP_IDT_HPP
