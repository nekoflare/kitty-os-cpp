//
// Created by Piotr on 08.05.2024.
//

#include "pci.hpp"

constexpr bool pci_extended_debug_mode = false;

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

constexpr uint16_t INTEL_VENDOR = 0x8086;
constexpr uint16_t VMWARE_VENDOR = 0x15AD;
constexpr uint16_t ENSONIQ_VENDOR = 0x1274;
constexpr uint16_t LSI_LOGIC_BROADCOM_SYMBIOS_VENDOR = 0x1000;
constexpr uint16_t RENESAS_TECH_CORP = 0x1912;
constexpr uint16_t QEMU_VENDOR = 0x1234;
constexpr uint16_t LSI_CORP = 0x11c1;

const char* pci_vendor_to_string(uint16_t vendor_id)
{
    switch (vendor_id)
    {
        case INTEL_VENDOR:
            return "Intel";
        case VMWARE_VENDOR:
            return "VMWare";
        case ENSONIQ_VENDOR:
            return "Ensoniq";
        case LSI_LOGIC_BROADCOM_SYMBIOS_VENDOR:
            return "LSI Logic / Broadcom / Symbios";
        case RENESAS_TECH_CORP:
            return "Renesas Technology Corp.";
        case QEMU_VENDOR:
            return "QEMU / Bochs";
        case LSI_CORP:
            return "LSI Corporation";
        default:
            kstd::printf("\n\nUnknown vendor id: %hx\n\n", vendor_id);
            return "Unknown";
    }
}

const char* pci_device_to_string(uint16_t vendor_id, uint16_t device_id)
{
    switch (vendor_id)
    {
        case INTEL_VENDOR:
            switch (device_id)
            {
                case 0x7190:
                    return "440BX/ZX/DX - 82443BX/ZX/DX Host bridge";
                case 0x7191:
                    return "440BX/ZX/DX - 82443BX/ZX/DX AGP bridge";
                case 0x7111:
                    return "82371AB/EB/MB PIIX4 IDE";
                case 0x100f:
                    return "82545EM Gigabit Ethernet Controller (Copper)";
                case 0x1237:
                    return "440FX - 82441FX PMC [Natoma]";
                case 0x7010:
                    return "82371SB PIIX3 IDE [Natoma/Triton II]";
                case 0x7020:
                    return "82371SB PIIX3 USB [Natoma/Triton II]";
                case 0x7113:
                    return "82371AB/EB/MB PIIX4 ACPI";
                case 0x100e:
                    return "82540EM Gigabit Ethernet Controller";
            }
            break;
        case VMWARE_VENDOR:
            switch (device_id)
            {
                case 0x0740:
                    return "Virtual Machine Communication Interface";
                case 0x0405:
                    return "SVGA II Adapter";
                case 0x0774:
                    return "USB1.1 UHCI Controller";
                case 0x0770:
                    return "USB2 EHCI Controller";
                case 0x07e0:
                    return "SATA AHCI controller";
                case 0x07f0:
                    return "NVMe SSD Controller";
                case 0x0790:
                    return "PCI bridge";
                case 0x077a:
                    return "Undocumented device.";
            }
            break;
        case ENSONIQ_VENDOR:
            switch (device_id)
            {
                case 0x1371:
                    return "ES1371/ES1373 / Creative Labs CT2518";
            }
            break;
        case LSI_LOGIC_BROADCOM_SYMBIOS_VENDOR:
            switch (device_id)
            {
                case 0x0030:
                    return "53c1030 PCI-X Fusion-MPT Dual Ultra320 SCSI";
            }
            break;
        case RENESAS_TECH_CORP:
            switch (device_id)
            {
                case 0x0015:
                    return "uPD720202 USB 3.0 Host Controller";
            }
            break;
        case QEMU_VENDOR:
            switch (device_id)
            {
                case 0x1111:
                    return "VGA display";
            }
            break;
        case LSI_CORP:
            switch (device_id)
            {
                case 0x5803:
                    return "USS-344S USB Controller";
            }
            break;
    }

    kstd::printf("Unknown device id: %hx on vendor id: %hx\n", device_id, vendor_id);
    return "Unknown";
}

void print_pci_header_type_0(PCIHeaderType0& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    if (pci_extended_debug_mode)
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
    } else {
        kstd::printf("Vendor: %s Device ID: %s\n", pci_vendor_to_string(hdr.common.vendor_id), pci_device_to_string(hdr.common.vendor_id, hdr.common.device_id));
    }
}

void print_pci_header_type_1(PCIHeaderType1& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    if (pci_extended_debug_mode)
    {
        kstd::printf("Location: \\\\PCI\\Devices\\%hhx\\%hhx\\%hhx\\pcidev\n", bus, slot, func);
        kstd::printf("Device ID: %hx Vendor ID: %hx\n", hdr.common.device_id, hdr.common.vendor_id);
        kstd::printf("Revision ID: %hhx Prog IF: %hhx\n", hdr.common.revision_id, hdr.common.prog_if);
        kstd::printf("Subclass: %hhx Class Code: %hhx\n", hdr.common.subclass, hdr.common.class_code);

        if (hdr.bar0) kstd::printf("    BAR0: %x\n", hdr.bar0);
        if (hdr.bar1) kstd::printf("    BAR0: %x\n", hdr.bar1);
    } else {
        kstd::printf("Vendor: %s Device ID: %s\n", pci_vendor_to_string(hdr.common.vendor_id), pci_device_to_string(hdr.common.vendor_id, hdr.common.device_id));
    }
    // Todo: Add more entries because I fucking need it.
}

void print_pci_header_type_2(PCIHeaderType2& hdr, uint8_t bus, uint8_t slot, uint8_t func)
{
    if (pci_extended_debug_mode)
    {
        kstd::printf("Location: \\\\SYSTEM\\PCI\\Devices\\%hhx\\%hhx\\%hhx\\pcidev\n", bus, slot, func);
        kstd::printf("Device ID: %hx Vendor ID: %hx\n", hdr.common.device_id, hdr.common.vendor_id);
        kstd::printf("Revision ID: %hhx Prog IF: %hhx\n", hdr.common.revision_id, hdr.common.prog_if);
        kstd::printf("Subclass: %hhx Class Code: %hhx\n", hdr.common.subclass, hdr.common.class_code);
    } else {
        kstd::printf("Vendor: %s Device ID: %s\n", pci_vendor_to_string(hdr.common.vendor_id), pci_device_to_string(hdr.common.vendor_id, hdr.common.device_id));
    }
    // todo:: Add more entries because I fucking need it.
}

void pci_init()
{
    PCIHeaderCommon phc {};
    PCIHeaderType0 phc_T0 {};
    PCIHeaderType1 phc_T1 {};
    PCIHeaderType2 phc_T2 {};
    pci_handle_t pci_handle;

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

                    pci_handle.vendor_id = phc_T0.common.vendor_id;
                    pci_handle.device_id = phc_T0.common.device_id;
                    pci_handle._class = phc_T0.common.class_code;
                    pci_handle.subclass = phc_T0.common.subclass;
                    pci_handle.prog_if = phc_T0.common.prog_if;
                    pci_handle.header_type = phc_T0.common.header_type;
                    pci_handle.bus = _bus;
                    pci_handle.slot = _slot;
                    pci_handle.function = 0;
                    pci_handle.full_header = static_cast<void*>(&phc_T0);

                    driver_ctrl_find_and_call(pci_handle);

                    break;
                case 0x01:
                    pci_copy_header<PCIHeaderType1>(&phc_T1, _bus, _slot, 0);
                    print_pci_header_type_1(phc_T1, _bus, _slot, 0);

                    pci_handle.vendor_id = phc_T1.common.vendor_id;
                    pci_handle.device_id = phc_T1.common.device_id;
                    pci_handle._class = phc_T1.common.class_code;
                    pci_handle.subclass = phc_T1.common.subclass;
                    pci_handle.prog_if = phc_T1.common.prog_if;
                    pci_handle.header_type = phc_T1.common.header_type;
                    pci_handle.bus = _bus;
                    pci_handle.slot = _slot;
                    pci_handle.function = 0;
                    pci_handle.full_header = static_cast<void*>(&phc_T1);

                    driver_ctrl_find_and_call(pci_handle);

                    break;
                case 0x02:
                    pci_copy_header<PCIHeaderType2>(&phc_T2, _bus, _slot, 0);
                    print_pci_header_type_2(phc_T2, _bus, _slot, 0);

                    pci_handle.vendor_id = phc_T2.common.vendor_id;
                    pci_handle.device_id = phc_T2.common.device_id;
                    pci_handle._class = phc_T2.common.class_code;
                    pci_handle.subclass = phc_T2.common.subclass;
                    pci_handle.prog_if = phc_T2.common.prog_if;
                    pci_handle.header_type = phc_T2.common.header_type;
                    pci_handle.bus = _bus;
                    pci_handle.slot = _slot;
                    pci_handle.function = 0;
                    pci_handle.full_header = static_cast<void*>(&phc_T2);

                    driver_ctrl_find_and_call(pci_handle);

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

                                pci_handle.vendor_id = phc_T0.common.vendor_id;
                                pci_handle.device_id = phc_T0.common.device_id;
                                pci_handle._class = phc_T0.common.class_code;
                                pci_handle.subclass = phc_T0.common.subclass;
                                pci_handle.prog_if = phc_T0.common.prog_if;
                                pci_handle.header_type = phc_T0.common.header_type;
                                pci_handle.bus = _bus;
                                pci_handle.slot = _slot;
                                pci_handle.function = func;
                                pci_handle.full_header = static_cast<void*>(&phc_T0);

                                driver_ctrl_find_and_call(pci_handle);

                                break;
                            case 0x01:
                                pci_copy_header<PCIHeaderType1>(&phc_T1, _bus, _slot, func);
                                print_pci_header_type_1(phc_T1, _bus, _slot, func);

                                pci_handle.vendor_id = phc_T1.common.vendor_id;
                                pci_handle.device_id = phc_T1.common.device_id;
                                pci_handle._class = phc_T1.common.class_code;
                                pci_handle.subclass = phc_T1.common.subclass;
                                pci_handle.prog_if = phc_T1.common.prog_if;
                                pci_handle.header_type = phc_T1.common.header_type;
                                pci_handle.bus = _bus;
                                pci_handle.slot = _slot;
                                pci_handle.function = func;
                                pci_handle.full_header = static_cast<void*>(&phc_T1);

                                driver_ctrl_find_and_call(pci_handle);

                                break;
                            case 0x02:
                                pci_copy_header<PCIHeaderType2>(&phc_T2, _bus, _slot, func);
                                print_pci_header_type_2(phc_T2, _bus, _slot, func);

                                pci_handle.vendor_id = phc_T2.common.vendor_id;
                                pci_handle.device_id = phc_T2.common.device_id;
                                pci_handle._class = phc_T2.common.class_code;
                                pci_handle.subclass = phc_T2.common.subclass;
                                pci_handle.prog_if = phc_T2.common.prog_if;
                                pci_handle.header_type = phc_T2.common.header_type;
                                pci_handle.bus = _bus;
                                pci_handle.slot = _slot;
                                pci_handle.function = func;
                                pci_handle.full_header = static_cast<void*>(&phc_T2);

                                driver_ctrl_find_and_call(pci_handle);

                                break;
                            default:
                                if (pci_extended_debug_mode)
                                    kstd::printf("Unknown header type inside of multi-function PCI device. Header type: %hhx\n", phc.header_type);
                                break;
                        }
                    }

                    break;
                default:
                    if (pci_extended_debug_mode)
                        kstd::printf("Unsupported header type: %hhx\n", phc.header_type);
                    break;
            }
        }
    }
}