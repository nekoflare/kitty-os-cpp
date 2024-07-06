//
// Created by Piotr on 24.05.2024.
//

#include "smbios.hpp"

volatile static limine_smbios_request smbios_request = {
        .id = LIMINE_SMBIOS_REQUEST,
        .revision = 0,
        .response = nullptr
};

volatile static limine_smbios_response* smbios = nullptr;

SMBIOS_32bit* smbios32 = nullptr;
SMBIOS_64bit* smbios64 = nullptr;
bool is_smbios64 = false;

void smbios_init() {
    if (smbios_request.response == nullptr) {
        kstd::printf("SMBIOS is missing.\n");
        return;
    }

    smbios = smbios_request.response;

    if (smbios->entry_64 != nullptr) {
        kstd::printf("We have 64-bit SMBIOS.\n");

        is_smbios64 = true;
        smbios64 = static_cast<SMBIOS_64bit *>(smbios->entry_64);
    } else if (smbios->entry_32 != nullptr) {
        kstd::printf("We have 32-bit SMBIOS.\n");

        smbios32 = static_cast<SMBIOS_32bit *>(smbios->entry_32);
    } else {
        kstd::printf("No SMBIOS pointer is available.\n");
        return;
    }

    void* smbios_pointer = is_smbios64 ? static_cast<void*>(smbios64) : static_cast<void*>(smbios32);
    kstd::printf("SMBIOS pointer: %p\n", smbios_pointer);

    if (!is_smbios64)
        kstd::printf("Number of structures: %zu\n", static_cast<size_t>(smbios32->number_of_structures));
    else
        kstd::printf("Number of structures: unknown for 64-bit\n"); // TODO: Implement proper structure counting for 64-bit
}

void* smbios_get_entry(size_t idx) {
    if (is_smbios64 || idx >= smbios_get_structure_count()) {
        return nullptr;
    }

    uintptr_t current_address = reinterpret_cast<uintptr_t>(smbios32) + sizeof(SMBIOS_32bit);
    for (size_t i = 0; i < idx; ++i) {
        SMBIOS_Tag* tag = reinterpret_cast<SMBIOS_Tag*>(current_address);
        current_address += tag->length;

        // Skip strings that are part of the structure
        while (*(reinterpret_cast<uint16_t*>(current_address)) != 0) {
            current_address++;
        }
        current_address += 2; // Skip the double null terminator of the strings section
    }

    return reinterpret_cast<void*>(current_address);
}

size_t smbios_get_structure_count() {
    if (is_smbios64) {
        // TODO: Implement structure count for 64-bit SMBIOS
        return 0; // Placeholder
    }
    return smbios32->number_of_structures;
}

void smbios_dump_info() {
    kstd::printf("Dumping SMBIOS information...\n");

    size_t entry_count = smbios_get_structure_count();
    for (size_t i = 0; i < entry_count; i++) {
        auto t = reinterpret_cast<SMBIOS_Tag*>(smbios_get_entry(i));
        auto name = smbios_entry_type_to_string(t->type);

        kstd::printf("(%zu). %s\n", i + 1, name);
    }
}
