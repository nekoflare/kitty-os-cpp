//
// Created by Neko on 20.01.2025.
//

#ifndef APIC_H
#define APIC_H

#include <stdint.h>

constexpr uint64_t IA32_APIC_BASE = 0x1B;
constexpr uint32_t APIC_REGISTER_ID = 0x020;            // Local APIC ID Register
constexpr uint32_t APIC_REGISTER_VERSION = 0x030;       // Local APIC Version Register
constexpr uint32_t APIC_REGISTER_TASK_PRIORITY = 0x080; // Task Priority Register
constexpr uint32_t APIC_REGISTER_EOI = 0x0B0;           // End of Interrupt Register
constexpr uint32_t APIC_REGISTER_SPURIOUS = 0x0F0;      // Spurious Interrupt Vector Register
constexpr uint32_t APIC_REGISTER_ICR_LOW = 0x300;       // Interrupt Command Register (low)
constexpr uint32_t APIC_REGISTER_ICR_HIGH = 0x310;      // Interrupt Command Register (high)
constexpr uint32_t APIC_REGISTER_LVT_TIMER = 0x320;     // Local Vector Table (LVT) Timer Register
constexpr uint32_t APIC_REGISTER_LVT_THERMAL = 0x330;   // LVT Thermal Sensor Register
constexpr uint32_t APIC_REGISTER_LVT_PERF = 0x340;      // LVT Performance Counter Register
constexpr uint32_t APIC_REGISTER_LVT_LINT0 = 0x350;     // LVT LINT0 Register
constexpr uint32_t APIC_REGISTER_LVT_LINT1 = 0x360;     // LVT LINT1 Register
constexpr uint32_t APIC_REGISTER_LVT_ERROR = 0x370;     // LVT Error Register
constexpr uint32_t APIC_REGISTER_TIMER_INITIAL = 0x380; // Timer Initial Count Register
constexpr uint32_t APIC_REGISTER_TIMER_CURRENT = 0x390; // Timer Current Count Register
constexpr uint32_t APIC_REGISTER_TIMER_DIVIDE = 0x3E0;  // Timer Divide Configuration Register

uint32_t apic_read(uintptr_t apic_base, uint32_t offset);
void apic_write(uintptr_t apic_base, uint32_t offset, uint32_t value);
void initialize_apic();
void apic_send_eoi(uintptr_t apic_base);
uint32_t get_lapic_id();

#endif // APIC_H
