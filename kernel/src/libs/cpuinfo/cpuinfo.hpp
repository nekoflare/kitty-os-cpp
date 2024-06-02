//
// Created by Piotr on 02.06.2024.
//

#ifndef KITTY_OS_CPP_CPUINFO_HPP
#define KITTY_OS_CPP_CPUINFO_HPP

#include <kernel/hal/x64/cpuid.hpp>
#include <cstddef>

typedef enum _CPUArchitecture
{
    X86, X86_64
} CPUArchitecture;

typedef struct _CPUInformation
{
    const char* vendor_id;
    const char* model_name;
    size_t speed_mhz;
    size_t bogo_mips;
} CPUInformation;

namespace CPUInfo
{
    bool HasSSE();
    bool HasSSE2();
    bool HasSSE3();
    bool HasSupplementalSSE3();
    bool HasSSE4_1();
    bool HasSSE4_2();
    bool IsAMD();
    bool IsIntel();
    CPUArchitecture GetCPUArchitecture();
    size_t GetVirtualBusWidth();
    size_t GetPhysicalBusWidth();
    const char* GetCPUVendorID();
    const char* GetCPUModelName();
    size_t GetCPUFamily();
    size_t GetCPUModel();
};

#endif //KITTY_OS_CPP_CPUINFO_HPP
