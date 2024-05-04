//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_IDT_HPP
#define KITTY_OS_CPP_IDT_HPP

#include <kstd/kstdint.hpp>

struct IDTR
{
    kstd::uint16_t limit;
    kstd::uint64_t idt_address;
} __attribute__((packed));

struct GateDescriptor64
{
    kstd::uint16_t offset_one;
    kstd::uint16_t segment_selector;
    kstd::uint8_t ist : 3;
    kstd::uint8_t reserved_one : 5;
    kstd::uint8_t gate_type : 4;
    kstd::uint8_t zero : 1;
    kstd::uint8_t dpl : 2;
    kstd::uint8_t present : 1;
    kstd::uint16_t offset_two;
    kstd::uint32_t offset_three;
    kstd::uint32_t reserved_two;
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

#define IDT_ENTRY(_Offset, _Segment_selector, _Gate_type, _Dpl_layer, _Is_present, _Ist) { \
    .offset_one = static_cast<kstd::uint16_t>(reinterpret_cast<kstd::uint64_t>(_Offset) & 0xffff),\
    .segment_selector = _Segment_selector,\
    .ist = _Ist,\
    .reserved_one = 0,\
    .gate_type = _Gate_type,\
    .zero = 0,\
    .dpl = _Dpl_layer,\
    .present = _Is_present,\
    .offset_two = static_cast<kstd::uint16_t>((reinterpret_cast<kstd::uint64_t>(_Offset) & 0xffff0000) >> 16),\
    .offset_three = (static_cast<kstd::uint32_t>((reinterpret_cast<kstd::uint64_t>(_Offset) & 0xffffffff00000000))>> 32) ,\
    .reserved_two = 0\
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
