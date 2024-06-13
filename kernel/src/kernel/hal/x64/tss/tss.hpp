//
// Created by Piotr on 07.06.2024.
//

#ifndef KITTY_OS_CPP_TSS_HPP
#define KITTY_OS_CPP_TSS_HPP

struct tss_x64
{
    uint32_t rsvd0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t rsvd1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t rsvd2;
    uint16_t rsvd3;
    uint16_t iopb;
} __attribute__((packed));

void tss_flush();
extern "C" void tss_flush_impl();

#endif //KITTY_OS_CPP_TSS_HPP
