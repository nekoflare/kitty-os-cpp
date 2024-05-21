#include "memory.hpp"

std::unordered_map<MemoryType, std::string> memory_type_strings = {
        {MemoryType::MEM_USABLE, "Usable"},
        {MemoryType::MEM_UNUSABLE, "Unusable"},
        {MemoryType::MEM_RESERVED, "Reserved"},
        {MemoryType::MEM_ACPI, "ACPI"},
        {MemoryType::MEM_FW, "Firmware"}
};

MemoryMetadata::MemoryMetadata(uint8_t* pointer, size_t length, uint64_t log_address, MemoryType type)
        : gPointer(pointer), gLength(length), gLogicalAddress(log_address), gType(type) {}

Memory::Memory() : gMemoryMappings() {}

Memory::~Memory() {
    for (auto& i : gMemoryMappings) {
        delete[] i->gPointer;
        delete i;
    }
}

MemoryMetadata* Memory::MapMemory(uint8_t* pointer, size_t length, uint64_t where, MemoryType type, bool increment_la) {
    auto* mm = new MemoryMetadata(pointer, length, where, type);
    gMemoryMappings.push_back(mm);
    if (increment_la) {
        last_address += length;
    }
    return mm;
}

uint8_t Memory::ReadByteBe(uint64_t address) const {
    for (auto mme : gMemoryMappings) {
        uint64_t start = mme->gLogicalAddress;
        uint64_t end = start + mme->gLength;
        if (address >= start && address < end) {
            size_t offset = address - start;
            return mme->gPointer[offset];
        }
    }
    throw std::out_of_range("Address out of mapped range");
}

void Memory::WriteByteBe(uint64_t address, uint8_t value) {
    for (auto mme : gMemoryMappings) {
        uint64_t start = mme->gLogicalAddress;
        uint64_t end = start + mme->gLength;
        if (address >= start && address < end) {
            size_t offset = address - start;
            mme->gPointer[offset] = value;
            return;
        }
    }
    throw std::out_of_range("Address out of mapped range");
}

void Memory::WriteWordBe(uint64_t address, uint16_t value) {
    WriteByteBe(address, static_cast<uint8_t>((value >> 8) & 0xFF));
    WriteByteBe(address + 1, static_cast<uint8_t>(value & 0xFF));
}

void Memory::WriteDwordBe(uint64_t address, uint32_t value) {
    WriteWordBe(address, static_cast<uint16_t>((value >> 16) & 0xFFFF));
    WriteWordBe(address + 2, static_cast<uint16_t>(value & 0xFFFF));
}

void Memory::WriteQwordBe(uint64_t address, uint64_t value) {
    WriteDwordBe(address, static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF));
    WriteDwordBe(address + 4, static_cast<uint32_t>(value & 0xFFFFFFFF));
}

void Memory::WriteDqwordBe(uint64_t address, __uint128_t value) {
    WriteQwordBe(address, static_cast<uint64_t>((value >> 64) & 0xFFFFFFFFFFFFFFFF));
    WriteQwordBe(address + 8, static_cast<uint64_t>(value & 0xFFFFFFFFFFFFFFFF));
}

uint16_t Memory::ReadWordBe(uint64_t address) const {
    return (static_cast<uint16_t>(ReadByteBe(address)) << 8) |
           (static_cast<uint16_t>(ReadByteBe(address + 1)));
}

uint32_t Memory::ReadDwordBe(uint64_t address) const {
    return (static_cast<uint32_t>(ReadWordBe(address)) << 16) |
           (static_cast<uint32_t>(ReadWordBe(address + 2)));
}

uint64_t Memory::ReadQwordBe(uint64_t address) const {
    return (static_cast<uint64_t>(ReadDwordBe(address)) << 32) |
           (static_cast<uint64_t>(ReadDwordBe(address + 4)));
}

__uint128_t Memory::ReadDqwordBe(uint64_t address) const {
    return (static_cast<__uint128_t>(ReadQwordBe(address)) << 64) |
           (static_cast<__uint128_t>(ReadQwordBe(address + 8)));
}

Memory::Iterator::Iterator(std::vector<MemoryMetadata*>::iterator initIt) : it(initIt) {}

Memory::Iterator Memory::Iterator::operator++() { ++it; return *this; }

bool Memory::Iterator::operator!=(const Memory::Iterator& other) const { return it != other.it; }

MemoryMetadata* Memory::Iterator::operator*() const { return *it; }

Memory::Iterator Memory::begin() { return Iterator(gMemoryMappings.begin()); }

Memory::Iterator Memory::end() { return Iterator(gMemoryMappings.end()); }
