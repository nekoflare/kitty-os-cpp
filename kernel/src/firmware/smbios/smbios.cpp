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

static SMBIOS_32bit* smbios_32bit = nullptr;
static bool smbios_64bit = false;

void smbios_init() {
    if (smbios_request.response == nullptr) {
        kstd::printf("SMBIOS is missing.\n");
        return;
    }

    void* smbios_pointer;

    smbios = smbios_request.response;

    if (smbios->entry_32 != nullptr) {
        kstd::printf("We have 32-bit SMBIOS.\n");

        smbios_pointer = smbios->entry_32;

        smbios_32bit = static_cast<SMBIOS_32bit *>(smbios_pointer);
    } else if (smbios->entry_64 != nullptr) {
        kstd::printf("We have 64-bit SMBIOS.\n");

        smbios_64bit = true;
        smbios_pointer = smbios->entry_64;

        // Todo: add 64-bit smbios
    } else {
        kstd::printf("No SMBIOS pointer is available.\n");
        return;
    }

    kstd::printf("SMBIOS pointer: %lx\n", smbios_pointer);


    kstd::printf("Number of structures: %ld\n", static_cast<uint64_t>(smbios_32bit->number_of_structures));

    smbios_get_entry(0);
}

void* smbios_get_entry(size_t idx) {
    if (idx >= smbios_get_structure_count() || !smbios_32bit) {
        return nullptr;
    }

    uintptr_t current_address = reinterpret_cast<uintptr_t>(smbios_32bit);
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
    // Todo: add support for 64 bit too
    return smbios_32bit->number_of_structures;
}

void smbios_dump_info()
{
    kstd::printf("Dumping SMBIOS information...\n");

    size_t entry_count = smbios_get_structure_count();
    uint64_t last_entry_type = 0;

    for (size_t i = 0; i < entry_count; i++)
    {
        auto t = reinterpret_cast<SMBIOS_Tag*>(smbios_get_entry(i));
        auto name = smbios_entry_type_to_string(t->type);

        kstd::printf("(%ld). %s", i, name);
    }
}
