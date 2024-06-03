//
// Created by Piotr on 02.06.2024.
//

#include <kstd/kstring.hpp>
#include "cpuinfo.hpp"

namespace CPUInfo
{
    bool HasSSE() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx >> 25) & 1;
    }

    // Function to check if CPU has SSE2
    bool HasSSE2() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx >> 26) & 1;
    }

    // Function to check if CPU has SSE3
    bool HasSSE3() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 0) & 1;
    }

    // Function to check if CPU has Supplemental SSE3
    bool HasSupplementalSSE3() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 9) & 1;
    }

    // Function to check if CPU has SSE4.1
    bool HasSSE4_1() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 19) & 1;
    }

    // Function to check if CPU has SSE4.2
    bool HasSSE4_2() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 20) & 1;
    }

    bool HasFPU()
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx) & 1;
    }

    bool HasSSE4a()
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid(0x80000001, eax, ebx, ecx, edx);
        return (ecx >> 6) & 1;
    }

    bool HasAVX()
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 28) & 1;
    }

    bool HasAVX2()
    {
        uint32_t eax, ebx, ecx, edx;
        ecx = 0;
        cpuid(7, eax, ebx, ecx, edx);
        return (ebx >> 5) & 1;
    }

    bool HasAVX512_F()
    {
        uint32_t eax, ebx, ecx, edx;
        ecx = 0;
        cpuid(7, eax, ebx, ecx, edx);
        return (ecx >> 28) & 1;
    }
}

bool CPUInfo::IsAMD()
{
    uint32_t vendor_id[4];
    cpuid(0, vendor_id[0], vendor_id[1], vendor_id[2], vendor_id[3]);

    // Convert vendor ID to a string
    char vendor_str[13];
    *(uint32_t*)(&vendor_str[0]) = vendor_id[1];
    *(uint32_t*)(&vendor_str[4]) = vendor_id[3];
    *(uint32_t*)(&vendor_str[8]) = vendor_id[2];
    vendor_str[12] = '\0';

    // Check if vendor ID corresponds to AMD
    return (vendor_str[0] == 'A' && vendor_str[1] == 'u' && vendor_str[2] == 't' && vendor_str[3] == 'h');
}

// Function to check if CPU is Intel
bool CPUInfo::IsIntel()
{
    uint32_t vendor_id[4];
    cpuid(0, vendor_id[0], vendor_id[1], vendor_id[2], vendor_id[3]);

    // Convert vendor ID to a string
    char vendor_str[13];
    *(uint32_t*)(&vendor_str[0]) = vendor_id[1];
    *(uint32_t*)(&vendor_str[4]) = vendor_id[3];
    *(uint32_t*)(&vendor_str[8]) = vendor_id[2];
    vendor_str[12] = '\0';

    // Check if vendor ID corresponds to Intel
    return (vendor_str[0] == 'G' && vendor_str[1] == 'e' && vendor_str[2] == 'n' && vendor_str[3] == 'u');
}

CPUArchitecture CPUInfo::GetCPUArchitecture()
{
    if (CPUInfo::IsAMD())
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        if (edx & (1 << 20))
        {
            return X86_64;
        }
        else
        {
            return X86;
        }
    }
    else if (CPUInfo::IsIntel())
    {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        if (edx & (1 << 21))
        {
            return X86_64;
        }
        else
        {
            return X86;
        }
    }
}

size_t CPUInfo::GetVirtualBusWidth()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, eax, ebx, ecx, edx);
    eax &= 0xff00;
    eax >>= 8;
    return eax;
}

size_t CPUInfo::GetPhysicalBusWidth()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, eax, ebx, ecx, edx);
    eax &= 0xff;
    return eax;
}

// Function to get the CPU vendor ID
const char* CPUInfo::GetCPUVendorID() {
    static char vendor_id[13];
    uint32_t vendor_id_regs[4];
    cpuid(0, vendor_id_regs[0], vendor_id_regs[1], vendor_id_regs[2], vendor_id_regs[3]);

    // Convert vendor ID to a string
    *(uint32_t*)(&vendor_id[0]) = vendor_id_regs[1];
    *(uint32_t*)(&vendor_id[4]) = vendor_id_regs[3];
    *(uint32_t*)(&vendor_id[8]) = vendor_id_regs[2];
    vendor_id[12] = '\0';

    return vendor_id;
}

// Function to get the CPU model name
const char* CPUInfo::GetCPUModelName() {
    static char model_name[49];
    uint32_t model_name_regs[4];
    cpuid(0x80000002, model_name_regs[0], model_name_regs[1], model_name_regs[2], model_name_regs[3]);
    kstd::memcpy(model_name, model_name_regs, sizeof(model_name_regs));

    cpuid(0x80000003, model_name_regs[0], model_name_regs[1], model_name_regs[2], model_name_regs[3]);
    kstd::memcpy(model_name + 16, model_name_regs, sizeof(model_name_regs));

    cpuid(0x80000004, model_name_regs[0], model_name_regs[1], model_name_regs[2], model_name_regs[3]);
    kstd::memcpy(model_name + 32, model_name_regs, sizeof(model_name_regs));

    model_name[48] = '\0';


    return model_name;
}

// Function to get the CPU family
size_t CPUInfo::GetCPUFamily() {
    uint32_t family, model;
    cpuid(1, family, model, *(uint32_t *)(&model + 1), *(uint32_t *)(&model + 2));

    size_t family_bits = (family >> 8) & 0xf;
    size_t extended_family = (family >> 20) & 0xff;
    return extended_family + family_bits;
}

// Function to get the CPU model
size_t CPUInfo::GetCPUModel() {
    uint32_t family, model;
    cpuid(1, family, model, *(uint32_t *)(&model + 1), *(uint32_t *)(&model + 2));

    size_t model_bits = (model >> 4) & 0xf;
    size_t extended_model = (model >> 16) & 0xf;
    return extended_model + model_bits;
}