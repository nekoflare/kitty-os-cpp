//
// Created by Piotr on 26.05.2024.
//

#include "acpi.hpp"

volatile limine_rsdp_request acpi_rsdp_request = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
        .response = nullptr,
};

volatile limine_rsdp_response* acpi_rsdp_limine = nullptr;

/**
 * Tables
 */
void* acpi_table_start = nullptr;
acpi_rsdp* rsdp_table = nullptr;
acpi_rsdt* rsdt_table = nullptr;
acpi_xsdt* xsdt_table = nullptr;
acpi_mcfg* mcfg_table = nullptr;
acpi_fadt* fadt_table = nullptr;

uint64_t* acpi_discovered_tables[512] = { nullptr };

size_t acpi_entry_count = 0;

void acpi_print_name(uint32_t s)
{
    char str[5] {0};
    kstd::memcpy(str, reinterpret_cast<void*>(&s), 4);
    str[4] = 0;
    kstd::puts(str);
}

acpi_mcfg* acpi_get_mcfg()
{
    return mcfg_table;
}

acpi_fadt* acpi_get_fadt()
{
    return fadt_table;
}

void acpi_parse_mcfg(acpi_mcfg* _mcfg_table)
{
    kstd::printf("[ACPI] [MCFG] Parsing MCFG table...\n");

    mcfg_table = _mcfg_table;

    kstd::printf("[ACPI] [MCFG] Done!\n");
}

acpi_madt* madt_table = nullptr;

void acpi_parse_madt(acpi_madt* _madt_table)
{
    kstd::printf("[ACPI] [MADT] Parsing MADT table...\n");

    madt_table = _madt_table;

    kstd::printf("[ACPI] [MADT] Done!\n");
}

void acpi_parse_fadt(acpi_fadt* _fadt_table)
{
    kstd::printf("[ACPI] [FADT] Parsing FADT table...\n");

    fadt_table = _fadt_table;

    kstd::printf("[ACPI] [FADT] Done!\n");
}

acpi_madt* acpi_get_madt()
{
    return madt_table;
}

void acpi_init()
{
    kstd::printf("[ACPI] Initializing...\n");

    if (acpi_rsdp_request.response == nullptr)
    {
        kstd::printf("[ACPI] No RSDP has been found on this machine.\n");

        return;
    }

    acpi_rsdp_limine = acpi_rsdp_request.response;

    rsdp_table = reinterpret_cast<acpi_rsdp*>(acpi_rsdp_limine->address);
    rsdt_table = reinterpret_cast<acpi_rsdt*>(vmm_make_virtual<uint64_t>(rsdp_table->rsdt_address));
    if (rsdp_table->xsdt_address != 0)
        xsdt_table = reinterpret_cast<acpi_xsdt*>(vmm_make_virtual<uint64_t>(rsdp_table->xsdt_address));

    kstd::printf("RSDP address: %llx\n", rsdp_table);
    kstd::printf("RSDT address: %llx\n", rsdt_table);
    kstd::printf("XSDT address: %llx\n", xsdt_table);

    acpi_entry_count = (rsdt_table->common.length - sizeof(acpi_rsdt)) / 4;

    kstd::printf("[ACPI] [RSDT] Entry count: %lld\n", acpi_entry_count);

    volatile uint32_t* entries = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<volatile uint8_t*>(rsdt_table) + sizeof(acpi_rsdt));

    // Print each entry
    for (size_t i = 0; i < acpi_entry_count; ++i) {
        kstd::printf("[ACPI] [RSDT] Entry %lld: %x\n", i, entries[i]);
        acpi_discovered_tables[i] = vmm_make_virtual<uint64_t*>(entries[i]);
    }

    int i = 0;
    while (acpi_discovered_tables[i] != 0)
    {
        uint64_t* table = acpi_discovered_tables[i];
        acpi_sdt_common* sdt = reinterpret_cast<acpi_sdt_common*>(table);
        acpi_print_name(sdt->signature);
        kstd::puts("\n");

        if (sdt->signature == 'GFCM') // MCFG but in reverse order
        {
            acpi_parse_mcfg(reinterpret_cast<acpi_mcfg*>(table));
        }

        if (sdt->signature == 'CIPA') // APIC but in reverse order
        {
            acpi_parse_madt(reinterpret_cast<acpi_madt*>(table));
        }

        if (sdt->signature == 'PCAF') // FACP but in reverse order
        {
            acpi_parse_fadt(reinterpret_cast<acpi_fadt*>(table));
        }

        i++;
    }

    kstd::printf("[ACPI] Done!\n");
}