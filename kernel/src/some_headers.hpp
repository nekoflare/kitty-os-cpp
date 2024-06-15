//
// Created by Piotr on 13.06.2024.
//

#ifndef KITTY_OS_CPP_SOME_HEADERS_HPP
#define KITTY_OS_CPP_SOME_HEADERS_HPP

// NVMe Controller Registers structure based on the provided specification
struct NVMeControllerRegisters {
    uint64_t CAP;       // 0x00: Controller Capabilities
    uint32_t VS;        // 0x08: Version
    uint32_t INTMS;     // 0x0C: Interrupt Mask Set
    uint32_t INTMC;     // 0x10: Interrupt Mask Clear
    uint32_t CC;        // 0x14: Controller Configuration
    uint8_t Reserved1[4];  // 0x18: Reserved
    uint32_t CSTS;      // 0x1C: Controller Status
    uint32_t NSSR;      // 0x20: NVM Subsystem Reset
    uint32_t AQA;       // 0x24: Admin Queue Attributes
    uint64_t ASQ;       // 0x28: Admin Submission Queue Base Address
    uint64_t ACQ;       // 0x30: Admin Completion Queue Base Address
    uint32_t CMBLOC;    // 0x38: Controller Memory Buffer Location
    uint32_t CMBSZ;     // 0x3C: Controller Memory Buffer Size
    uint32_t BPINFO;    // 0x40: Boot Partition Information
    uint32_t BPRSEL;    // 0x44: Boot Partition Read Select
    uint64_t BPMBL;     // 0x48: Boot Partition Memory Buffer Location
    uint64_t CMBMSC;    // 0x50: Controller Memory Buffer Memory Space Control
    uint32_t CMBSTS;    // 0x58: Controller Memory Buffer Status
    uint32_t CMBEBS;    // 0x5C: Controller Memory Buffer Elasticity Buffer Size
    uint32_t CMBSWTP;   // 0x60: Controller Memory Buffer Sustained Write Throughput
    uint32_t NSSD;      // 0x64: NVM Subsystem Shutdown
    uint32_t CRTO;      // 0x68: Controller Ready Timeouts
    uint8_t Reserved2[0x980 - 0x6C];  // 0x6C - 0xE00: Reserved
    uint32_t PMRCAP;    // 0xE00: Persistent Memory Capabilities
    uint32_t PMRCTL;    // 0xE04: Persistent Memory Region Control
    uint32_t PMRSTS;    // 0xE08: Persistent Memory Region Status
    uint32_t PMREBS;    // 0xE0C: Persistent Memory Region Elasticity Buffer Size
    uint32_t PMRSWTP;   // 0xE10: Persistent Memory Region Sustained Write Throughput
    uint32_t PMRMSCL;   // 0xE14: Persistent Memory Region Controller Memory Space Control Lower
    uint32_t PMRMSCU;   // 0xE18: Persistent Memory Region Controller Memory Space Control Upper
    uint8_t Reserved3[0x1000 - 0xE1C]; // 0xE1C - 0x1000: Reserved
    uint8_t TransportSpecific[0x300]; // 0x1000: Transport Specific
    uint8_t VendorSpecific[0x1000];   // 0x1300: Vendor Specific
};

#endif //KITTY_OS_CPP_SOME_HEADERS_HPP
