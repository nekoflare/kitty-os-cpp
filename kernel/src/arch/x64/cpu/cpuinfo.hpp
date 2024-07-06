//
// Created by Piotr on 02.06.2024.
//

#ifndef KITTY_OS_CPP_CPUINFO_HPP
#define KITTY_OS_CPP_CPUINFO_HPP

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
    // Misc
    bool HasFPU();

    // SSE
    bool HasSSE();
    bool HasSSE2();
    bool HasSSE3();
    bool HasSupplementalSSE3();
    bool HasSSE4a();
    bool HasSSE4_1();
    bool HasSSE4_2();

    // AVX
    bool HasAVX();
    bool HasAVX2();
    bool HasAVX512_F();

    // CPU Info
    bool IsAMD();
    bool IsIntel();
    CPUArchitecture GetCPUArchitecture();
    const char* GetCPUVendorID();
    const char* GetCPUModelName();
    size_t GetCPUFamily();
    size_t GetCPUModel();

    // Memory info
    size_t GetVirtualBusWidth();
    size_t GetPhysicalBusWidth();
};

#endif //KITTY_OS_CPP_CPUINFO_HPP
