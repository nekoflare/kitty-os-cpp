//
// Created by Piotr on 30.04.2024.
//

#ifndef GDT_H
#define GDT_H


#include <stdint.h>
#include <sys/types.h>

struct GDTR
{
    uint16_t size;               // Note: It's size of the GDT table minus 1.
    uint64_t gdt_address;        // Note: Virtual address of GDT.
} __attribute__((packed));

struct GDTAccessByte
{
    uint8_t accessed_bit : 1;
    uint8_t read_write_bit : 1;
    uint8_t direction_bit : 1;
    uint8_t executable_bit : 1;
    uint8_t descriptor_type_bit : 1;
    uint8_t descriptor_privilege_level_bits : 2;
    uint8_t present_bit : 1;
} __attribute__((packed));

enum GDTAccessByteBits
{
    GDT_ACCESSED_BIT = 1 << 0,
    GDT_READ_WRITE_BIT = 1 << 1,
    GDT_DIRECTION_BIT = 1 << 2,
    GDT_EXECUTABLE_BIT = 1 << 3,
    GDT_DESCRIPTOR_TYPE_BIT = 1 << 4,
    GDT_DPL0 = 0,
    GDT_DPL1 = 1 << 5,
    GDT_DPL2 = 1 << 6,
    GDT_DPL3 = (1 << 5) | (1 << 6),
    GDT_PRESENT = 1 << 7
};

enum GDTFlagsBits
{
    GDT_GRANULARITY_FLAG = 1 << 3,
    GDT_SIZE_FLAG = 1 << 2,
    GDT_LONG_MODE_FLAG = 1 << 1
};

struct GDTSegmentDescriptor
{
    uint16_t limit_one;
    uint16_t base_zero;
    uint8_t base_two;
    uint8_t access_byte;
    uint8_t limit_two : 4;
    uint8_t flags : 4;
    uint8_t base_three;
} __attribute__((packed));

struct gdt_system_segment_descriptor {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access_byte;
    uint8_t limit1 : 4;
    uint8_t flags : 4;
    uint8_t base2;
    uint32_t base3;
} __attribute__((packed));

void flush_gdt();
extern "C" void flush_gdt_asm(GDTR* gdtr);

extern struct GDTSegmentDescriptor gdt[];

#define GDT_ENTRY(_Base, _Limit, _Access, _Flags) {                    \
    .limit_one = static_cast<uint16_t>(_Limit & 0xFFFF),               \
    .base_zero = static_cast<uint16_t>(_Base & 0xFFFF),                \
    .base_two = static_cast<uint8_t>((_Base & 0xFF0000) >> 16),        \
    .access_byte = (_Access),                                          \
    .limit_two = static_cast<uint8_t>((_Limit & 0xF0000) >> 16),       \
    .flags = _Flags,                                                   \
    .base_three = static_cast<uint8_t>((_Base & 0xFF000000) >> 24)     \
}                                                                      \

#endif //GDT_H
