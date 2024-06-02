//
// Created by Piotr on 01.06.2024.
//

#ifndef KITTY_OS_CPP_APIC_HPP
#define KITTY_OS_CPP_APIC_HPP

#include <firmware/acpi/acpi.hpp>
#include <kstd/kstdio.hpp>
#include <kernel/hal/x64/msr.hpp>
#include <kernel/hal/x64/cpuid.hpp>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800


void apic_init();

#endif //KITTY_OS_CPP_APIC_HPP
