//
// Created by Piotr on 07.06.2024.
//

#include "../gdt/gdt.hpp"
#include "tss.hpp"

void tss_flush()
{
    tss_x64* tss = new tss_x64;
    tss->rsp0 = reinterpret_cast<uint64_t>(new char[8192]);
    tss->iopb = sizeof(tss_x64);
    tss->rsvd0 = 0;
    tss->rsvd1 = 0;
    tss->rsvd2 = 0;
    tss->rsvd3 = 0;


    auto ssd = reinterpret_cast<gdt_system_segment_descriptor*>(&gdt[9]);
    uint64_t a = reinterpret_cast<uint64_t>(&tss);

    ssd->limit0 = 0;
    ssd->limit1 = 0;
    ssd->base0 = a & 0xffff;
    ssd->base1 = (a >> 16) & 0xff;
    ssd->base2 = (a >> 24) & 0xff;
    ssd->base3 = (a >> 32) & 0xffffffff;
    ssd->limit0 = sizeof(tss) & 0xffff;
    ssd->limit1 = (sizeof(tss) >> 16) & 0xf;
    ssd->flags = GDT_SIZE_FLAG;
    ssd->access_byte = GDT_PRESENT | GDT_EXECUTABLE_BIT | GDT_ACCESSED_BIT;

    flush_gdt();
    tss_flush_impl();
}