//
// Created by Neko on 20.01.2025.
//

#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>

// IO APIC Register Offsets
constexpr uint32_t IOAPIC_REGISTER_IOAPICID = 0x00;         // IO APIC ID
constexpr uint32_t IOAPIC_REGISTER_IOAPICVER = 0x01;        // IO APIC Version
constexpr uint32_t IOAPIC_REGISTER_IOAPICARB = 0x02;        // IO APIC Arbitration ID
constexpr uint32_t IOAPIC_REGISTER_REDIRECTION_BASE = 0x10; // Redirection Table Base

struct ioapic_entry
{
    uint8_t vector;               // 8 bits: Interrupt vector (0-7)
    uint8_t delivery_mode : 3;    // 3 bits: Delivery Mode (8-10)
    uint8_t destination_mode : 1; // 1 bit: Destination Mode (11)
    uint8_t delivery_status : 1;  // 1 bit: Delivery Status (12)
    uint8_t pin_polarity : 1;     // 1 bit: Pin Polarity (13)
    uint8_t remote_irr : 1;       // 1 bit: Remote IRR (14)
    uint8_t trigger_mode : 1;     // 1 bit: Trigger Mode (15)
    uint8_t mask : 1;             // 1 bit: Mask (16)
    uint32_t reserved0 : 32;      // 39 bits reserved (17-55) - Using uint8_t for the
                                  // first part of reserved
    uint8_t reserved1 : 7;
    uint8_t destination; // 8 bits: Destination (56-63)
} __attribute__((packed));

uint8_t get_ioapic_max_redirections(uintptr_t ioapic_base);
void ioapic_set_redirection(uintptr_t ioapic_base, uint8_t irq, uint64_t destination, uint8_t vector,
                            uint8_t delivery_mode, bool level_triggered, bool physical_destination = true,
                            bool delivery_status = false, bool pin_polarity = false, bool mask_irq = false);

uint64_t ioapic_get_redirection(uintptr_t ioapic_base, uint8_t irq);
void initialize_ioapic();

#endif // IOAPIC_H
