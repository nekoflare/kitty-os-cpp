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

void print_pci_header_type_0(PCIHeaderType0& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    kstd::printf("Location: \\\\PCI\\Devices\\%hhx\\%hhx\\%hhx\\pcidev\n", bus, slot, func);
    kstd::printf("Device ID: %hx Vendor ID: %hx\n", hdr.common.device_id, hdr.common.vendor_id);
    kstd::printf("Revision ID: %hhx Prog IF: %hhx\n", hdr.common.revision_id, hdr.common.prog_if);
    kstd::printf("Subclass: %hhx Class Code: %hhx\n", hdr.common.subclass, hdr.common.class_code);

    if (hdr.bar0) kstd::printf("    BAR0: %x\n", hdr.bar0);
    if (hdr.bar1) kstd::printf("    BAR0: %x\n", hdr.bar1);
    if (hdr.bar2) kstd::printf("    BAR0: %x\n", hdr.bar2);
    if (hdr.bar3) kstd::printf("    BAR0: %x\n", hdr.bar3);
    if (hdr.bar4) kstd::printf("    BAR0: %x\n", hdr.bar4);
}

void print_pci_header_type_1(PCIHeaderType1& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    kstd::printf("Location: \\\\PCI\\Devices\\%hhx\\%hhx\\%hhx\\pcidev\n", bus, slot, func);
    kstd::printf("Device ID: %hx Vendor ID: %hx\n", hdr.common.device_id, hdr.common.vendor_id);
    kstd::printf("Revision ID: %hhx Prog IF: %hhx\n", hdr.common.revision_id, hdr.common.prog_if);
    kstd::printf("Subclass: %hhx Class Code: %hhx\n", hdr.common.subclass, hdr.common.class_code);

    if (hdr.bar0) kstd::printf("    BAR0: %x\n", hdr.bar0);
    if (hdr.bar1) kstd::printf("    BAR0: %x\n", hdr.bar1);
    // Todo: Add more entries because I fucking need it.
}

void print_pci_header_type_2(PCIHeaderType2& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    kstd::printf("Location: \\\\SYSTEM\\PCI\\Devices\\%hhx\\%hhx\\%hhx\\pcidev\n", bus, slot, func);
    kstd::printf("Device ID: %hx Vendor ID: %hx\n", hdr.common.device_id, hdr.common.vendor_id);
    kstd::printf("Revision ID: %hhx Prog IF: %hhx\n", hdr.common.revision_id, hdr.common.prog_if);
    kstd::printf("Subclass: %hhx Class Code: %hhx\n", hdr.common.subclass, hdr.common.class_code);
    // todo:: Add more entries because I fucking need it.
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

            switch (phc.header_type)
            {
                case 0x00:
                    pci_copy_header<PCIHeaderType0>(&phc_T0, _bus, _slot, 0);
                    print_pci_header_type_0(phc_T0, _bus, _slot, 0);
                    break;
                case 0x01:
                    pci_copy_header<PCIHeaderType1>(&phc_T1, _bus, _slot, 0);
                    print_pci_header_type_1(phc_T1, _bus, _slot, 0);
                    break;
                case 0x02:
                    pci_copy_header<PCIHeaderType2>(&phc_T2, _bus, _slot, 0);
                    print_pci_header_type_2(phc_T2, _bus, _slot, 0);
                    break;
                case 0x80:
                    // Multifunction device.

                    for (size_t func = 1; 8 > func; func++)
                    {
                        pci_copy_common_header(&phc, _bus, _slot, func);

                        if (phc.device_id == 0xFFFF && phc.vendor_id == 0xFFFF)
                        {
                            continue;
                        }

                        switch (phc.header_type)
                        {
                            case 0x00:
                                pci_copy_header<PCIHeaderType0>(&phc_T0, _bus, _slot, func);
                                print_pci_header_type_0(phc_T0, _bus, _slot, func);
                                break;
                            case 0x01:
                                pci_copy_header<PCIHeaderType1>(&phc_T1, _bus, _slot, func);
                                print_pci_header_type_1(phc_T1, _bus, _slot, func);
                                break;
                            case 0x02:
                                pci_copy_header<PCIHeaderType2>(&phc_T2, _bus, _slot, func);
                                print_pci_header_type_2(phc_T2, _bus, _slot, func);
                                break;
                            default:
                                kstd::printf("Unknown header type inside of multi-function PCI device. Header type: %hhx\n", phc.header_type);
                                break;
                        }
                    }

                    break;
                default:
                    kstd::printf("Unsupported header type: %hhx\n", phc.header_type);
                    break;
            }
        }
    }
}