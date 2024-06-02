//
// Created by Piotr on 01.06.2024.
//

#include "apic.hpp"

bool check_apic() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, eax, ebx, ecx, edx);
    return edx & ( 1 << 9 );
}

/* Set the physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic) {
    uint64_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
    uint64_t edx = 0; // Upper 32 bits should be 0 for APIC base MSR

    cpuSetMSR(IA32_APIC_BASE_MSR, eax | (edx << 32));
}

void apic_register_write(uint64_t base, uint32_t reg, uint32_t value)
{
    base += vmm_hhdm->offset;
    base += reg;

    uint32_t* v = reinterpret_cast<uint32_t*>(base);
    *v = value;
}

uint32_t apic_register_read(uint64_t base, uint32_t reg)
{
    base += vmm_hhdm->offset;
    base += reg;

    uint32_t* v = reinterpret_cast<uint32_t*>(base);
    return *v;
}

void apic_init() {
    kstd::printf("[IRQ:APIC] Starting to route hardware interrupts.\n");

    if (!check_apic()) {
        kstd::printf("This CPU does not have APIC.\n");
        unreachable();
    }

    // Get MADT table for my ease.
    acpi_madt* madt = acpi_get_madt();

    if (madt == nullptr) {
        kstd::printf("No MADT!\n");
        unreachable();
    }

    auto lapic_address = madt->lapic_address;
    kstd::printf("LAPIC address: %lx\n", lapic_address);

    // Pointer to the first entry
    acpi_madt_entry* entry = reinterpret_cast<acpi_madt_entry*>(
            reinterpret_cast<uint8_t*>(madt) + sizeof(acpi_madt)
    );
    size_t index = 0;
    kstd::printf("Index address: %llx\n", entry);

    // Iterate through all MADT entries
    while (reinterpret_cast<uint8_t*>(entry) < reinterpret_cast<uint8_t*>(madt) + madt->common.length) {
        kstd::printf("Entry %lld: %s\n", index, madt_entry_type_to_string(entry->entry_type));

        // Move to the next entry
        entry = reinterpret_cast<acpi_madt_entry*>(
                reinterpret_cast<uint8_t*>(entry) + entry->entry_length
        );
        index++;
    }

    kstd::printf("[IRQ:APIC] Setting the APIC base address.\n");
    cpu_set_apic_base(madt->lapic_address);
}