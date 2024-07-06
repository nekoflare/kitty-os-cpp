#include <hal/x64/cpuid.hpp>
#include <arch/x64/cpu/cpuinfo.hpp>
#include <kstd/kstring.hpp>

namespace CPUInfo
{
    bool HasSSE() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx >> 25) & 1;
    }

    bool HasSSE2() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx >> 26) & 1;
    }

    bool HasSSE3() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 0) & 1;
    }

    bool HasSupplementalSSE3() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 9) & 1;
    }

    bool HasSSE4_1() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 19) & 1;
    }

    bool HasSSE4_2() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 20) & 1;
    }

    bool HasFPU() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (edx) & 1;
    }

    bool HasSSE4a() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(0x80000001, eax, ebx, ecx, edx);
        return (ecx >> 6) & 1;
    }

    bool HasAVX() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, eax, ebx, ecx, edx);
        return (ecx >> 28) & 1;
    }

    bool HasAVX2() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(7, eax, ebx, ecx, edx);
        return (ebx >> 5) & 1;
    }

    bool HasAVX512_F() {
        uint32_t eax, ebx, ecx, edx;
        cpuid(7, eax, ebx, ecx, edx);
        return (ebx >> 16) & 1;
    }
}

bool CPUInfo::IsAMD() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, eax, ebx, ecx, edx);

    char vendor_str[13];
    *reinterpret_cast<uint32_t*>(&vendor_str[0]) = ebx;
    *reinterpret_cast<uint32_t*>(&vendor_str[4]) = edx;
    *reinterpret_cast<uint32_t*>(&vendor_str[8]) = ecx;
    vendor_str[12] = '\0';

    return kstd::strcmp(vendor_str, "AuthenticAMD") == 0;
}

bool CPUInfo::IsIntel() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, eax, ebx, ecx, edx);

    char vendor_str[13];
    *reinterpret_cast<uint32_t*>(&vendor_str[0]) = ebx;
    *reinterpret_cast<uint32_t*>(&vendor_str[4]) = edx;
    *reinterpret_cast<uint32_t*>(&vendor_str[8]) = ecx;
    vendor_str[12] = '\0';

    return kstd::strcmp(vendor_str, "GenuineIntel") == 0;
}

CPUArchitecture CPUInfo::GetCPUArchitecture() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, eax, ebx, ecx, edx);
    if (edx & (1 << 20)) {
        return X86_64;
    }
    return X86;
}

size_t CPUInfo::GetVirtualBusWidth() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, eax, ebx, ecx, edx);
    return (eax >> 8) & 0xff;
}

size_t CPUInfo::GetPhysicalBusWidth() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, eax, ebx, ecx, edx);
    return eax & 0xff;
}

const char* CPUInfo::GetCPUVendorID() {
    static char vendor_id[13];
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, eax, ebx, ecx, edx);

    *reinterpret_cast<uint32_t*>(&vendor_id[0]) = ebx;
    *reinterpret_cast<uint32_t*>(&vendor_id[4]) = edx;
    *reinterpret_cast<uint32_t*>(&vendor_id[8]) = ecx;
    vendor_id[12] = '\0';

    return vendor_id;
}

const char* CPUInfo::GetCPUModelName() {
    static char model_name[49];
    uint32_t regs[4];

    cpuid(0x80000002, regs[0], regs[1], regs[2], regs[3]);
    kstd::memcpy(model_name, regs, sizeof(regs));

    cpuid(0x80000003, regs[0], regs[1], regs[2], regs[3]);
    kstd::memcpy(model_name + 16, regs, sizeof(regs));

    cpuid(0x80000004, regs[0], regs[1], regs[2], regs[3]);
    kstd::memcpy(model_name + 32, regs, sizeof(regs));

    model_name[48] = '\0';

    return model_name;
}

size_t CPUInfo::GetCPUFamily() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, eax, ebx, ecx, edx);
    size_t family_bits = (eax >> 8) & 0xf;
    size_t extended_family = (eax >> 20) & 0xff;
    return extended_family + family_bits;
}

size_t CPUInfo::GetCPUModel() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, eax, ebx, ecx, edx);
    size_t model_bits = (eax >> 4) & 0xf;
    size_t extended_model = (eax >> 16) & 0xf;
    return extended_model + model_bits;
}
