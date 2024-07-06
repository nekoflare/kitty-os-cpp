//
// Created by Piotr on 26.05.2024.
//

#include <limine.h>
#include <kstd/kstdio.hpp>
#include <mm/vmm.hpp>
#include <kstd/kstring.hpp>
#include "acpi.hpp"

volatile limine_rsdp_request acpi_rsdp_request = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
        .response = nullptr,
};

static bool acpi_is_xsdt = false;
static acpi_rsdp* acpi_rsdp_pointer = nullptr;
static acpi_rsdt* acpi_rsdt_pointer = nullptr;
static acpi_xsdt* acpi_xsdt_pointer = nullptr;
static bool acpi_is_initialized = false;

void acpi_init() {
    if (acpi_rsdp_request.response == nullptr) {
        acpi_is_initialized = false;
        return;
    }

    acpi_rsdp_pointer = reinterpret_cast<acpi_rsdp*>(acpi_rsdp_request.response->address);

    if (acpi_rsdp_pointer->revision > 0) {
        // uses XSDT
        acpi_is_xsdt = true;
        acpi_xsdt_pointer = reinterpret_cast<acpi_xsdt*>(acpi_rsdp_pointer->xsdt_address);
    } else {
        // uses RSDT
        acpi_is_xsdt = false;
        acpi_rsdt_pointer = reinterpret_cast<acpi_rsdt*>(acpi_rsdp_pointer->rsdt_address);
    }

    acpi_is_initialized = true;
}

void* acpi_get_table(const char* signature) {
    if (!acpi_is_initialized) return nullptr;

    if (acpi_is_xsdt) {
        size_t entry_count = (acpi_xsdt_pointer->common.length - sizeof(acpi_xsdt)) / 8; // Pointers are 8-byte wide in this case.
        uintptr_t* current_table = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(acpi_xsdt_pointer) + sizeof(acpi_xsdt)); // Go to the end of the table where the table pointers live.

        // Iterate through table pointers
        for (size_t i = 0; entry_count > i; i++) {
            // Convert address to virtual pointer
            acpi_sdt_common* current_table_virtual = reinterpret_cast<acpi_sdt_common*>(vmm_make_virtual_singular(current_table[i]));

            if (kstd::memcmp(reinterpret_cast<const void*>(signature), reinterpret_cast<void*>(&current_table_virtual->signature), 4) == 0) {
                // The table signatures match.
                return reinterpret_cast<void*>(current_table_virtual);
            }
        }
    } else {
        // it's using old type
        size_t entry_count = (acpi_rsdt_pointer->common.length - sizeof(acpi_rsdt)) / 4; // Pointers are 4-byte wide in this case.
        uint32_t* current_table = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(acpi_rsdt_pointer) + sizeof(acpi_rsdt)); // Go to the end of the table where the table pointers live.

        // Iterate through table pointers
        for (size_t i = 0; entry_count > i; i++) {
            // Extend the 4-byte address to 8 bytes before conversion
            uintptr_t table_address = static_cast<uintptr_t>(current_table[i]);
            acpi_sdt_common* current_table_virtual = reinterpret_cast<acpi_sdt_common*>(vmm_make_virtual_singular(table_address));

            if (kstd::memcmp(reinterpret_cast<const void*>(signature), reinterpret_cast<void*>(&current_table_virtual->signature), 4) == 0) {
                // The table signatures match.
                return reinterpret_cast<void*>(current_table_virtual);
            }
        }
    }

    return nullptr;
}

bool acpi_table_exists(const char* signature) {
    if (!acpi_is_initialized) return false;

    if (acpi_is_xsdt) {
        size_t entry_count = (acpi_xsdt_pointer->common.length - sizeof(acpi_xsdt)) / 8; // Pointers are 8-byte wide in this case.
        uintptr_t* current_table = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(acpi_xsdt_pointer) + sizeof(acpi_xsdt)); // Go to the end of the table where the table pointers live.

        // Iterate through table pointers
        for (size_t i = 0; entry_count > i; i++) {
            // Convert address to virtual pointer
            acpi_sdt_common* current_table_virtual = reinterpret_cast<acpi_sdt_common*>(vmm_make_virtual_singular(current_table[i]));

            if (kstd::memcmp(reinterpret_cast<const void*>(signature), reinterpret_cast<void*>(&current_table_virtual->signature), 4) == 0) {
                // The table signatures match.
                return true;
            }
        }
    } else {
        // it's using old type
        size_t entry_count = (acpi_rsdt_pointer->common.length - sizeof(acpi_rsdt)) / 4; // Pointers are 4-byte wide in this case.
        uint32_t* current_table = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(acpi_rsdt_pointer) + sizeof(acpi_rsdt)); // Go to the end of the table where the table pointers live.

        // Iterate through table pointers
        for (size_t i = 0; entry_count > i; i++) {
            // Extend the 4-byte address to 8 bytes before conversion
            uintptr_t table_address = static_cast<uintptr_t>(current_table[i]);
            acpi_sdt_common* current_table_virtual = reinterpret_cast<acpi_sdt_common*>(vmm_make_virtual_singular(table_address));

            if (kstd::memcmp(reinterpret_cast<const void*>(signature), reinterpret_cast<void*>(&current_table_virtual->signature), 4) == 0) {
                // The table signatures match.
                return true;
            }
        }
    }

    return false;
}
