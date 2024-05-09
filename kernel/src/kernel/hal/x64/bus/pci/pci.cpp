//
// Created by Piotr on 08.05.2024.
//

#include "pci.hpp"

uint8_t read_pci_config_byte(int bus, int slot, int function, int offset) {
    uint32_t address = (1 << 31) | (offset & 0xFC) | (function << 8) | (slot << 11) | (bus << 16);

    outl(0xCF8, address);

    uint32_t res = inl(0xCFC);

    // Shift the desired byte to the least significant byte position
    res >>= (offset & 3) * 8;

    return static_cast<uint8_t>(res);
}


uint16_t read_pci_config_word(int bus, int slot, int function, int offset)
{
    uint32_t address = (1 << 31) | (offset & 0xfc) | (function << 8) | (slot << 11) | (bus << 16);

    outl(0xcf8, address);

    uint32_t res = inl(0xcfc) & 0xffff;

    return static_cast<uint16_t>(res);
}

void pci_copy_common_header(void* phc, int bus, int slot, int function)
{
    uint8_t* phc_u8 = reinterpret_cast<uint8_t*>(phc);

    for (size_t i = 0; sizeof(PCIHeaderCommon) > i; i++)
    {
        phc_u8[i] = read_pci_config_byte(bus, slot, function, i);
    }
}

template<typename T>
void pci_copy_header(void* phc, int bus, int slot, int function)
{
    uint8_t* phc_u8 = reinterpret_cast<uint8_t*>(phc);

    for (size_t i = 0; sizeof(T) > i; i++)
    {
        phc_u8[i] = read_pci_config_byte(bus, slot, function, i);
    }
}

void pci_init()
{
    PCIHeaderCommon phc {};
    PCIHeaderType0 phc_T0 {};
    PCIHeaderType1 phc_T1 {};
    PCIHeaderType2 phc_T2 {};

    for (size_t _bus = 0; 256 > _bus; _bus++)
    {
        for (size_t _slot = 0; 32 > _slot; _slot++)
        {
            pci_copy_common_header(&phc, _bus, _slot, 0);

            if (phc.vendor_id == 0xffff && phc.device_id == 0xffff)
            {
                continue;
            }

            /*
            kstd::printf("Bus: %llx Slot: %llx\n", _bus, _slot);
            kstd::printf("(Vendor : Device ID) %hx:%hx\n", phc.vendor_id, phc.device_id);
            kstd::printf("Command: %hx Status: %hx\n", phc.command, phc.vendor_id);
            kstd::printf("RevID: %hhx ProgIF: %hhx\n", phc.revision_id, phc.prog_if);
            kstd::printf("Subclass: %hhx Class Code: %hhx\n", phc.subclass, phc.class_code);
            kstd::printf("Cache line size: %hhx Latency Timer: %hhx\n", phc.cache_line_size, phc.latency_timer);
            kstd::printf("Header type: %hhx Bist: %hhx\n", phc.header_type, phc.bist);
            */

            kstd::printf("BUS: %lld SLOT: %lld (%hx:%hx)\n", _bus, _slot, phc.vendor_id, phc.device_id);

            switch (phc.header_type)
            {
                case 0x00:
                    pci_copy_header<PCIHeaderType0>(&phc_T0, _bus, _slot, 0);
                    if (phc_T0.bar0 != 0)
                        kstd::printf("BAR0: %llx", phc_T0.bar0);
                    if (phc_T0.bar1 != 0)
                        kstd::printf(" BAR1: %llx", phc_T0.bar1);
                    if (phc_T0.bar2 != 0)
                        kstd::printf(" BAR2: %llx", phc_T0.bar2);
                    if (phc_T0.bar3 != 0)
                        kstd::printf("BAR3: %llx", phc_T0.bar3);
                    if (phc_T0.bar4 != 0)
                        kstd::printf(" BAR4: %llx", phc_T0.bar4);
                    if (phc_T0.bar5 != 0)
                        kstd::printf(" BAR5: %llx\n", phc_T0.bar5);

                    break;
                case 0x01:
                    kstd::printf("Unsupported header type: %hhx\n", phc.header_type);
                    break;
                case 0x02:
                    kstd::printf("Unsupported header type: %hhx\n", phc.header_type);
                    break;
                default:
                    kstd::printf("Unsupported header type: %hhx\n", phc.header_type);
                    break;
            }
        }
    }
}