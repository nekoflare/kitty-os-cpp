//
// Created by Piotr on 20.05.2024.
//

#ifndef EMULATOR_MEMORY_HPP
#define EMULATOR_MEMORY_HPP

#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>

enum class MemoryType {
    MEM_USABLE = 0,
    MEM_UNUSABLE = 1,
    MEM_RESERVED = 2,
    MEM_ACPI = 3,
    MEM_FW = 4
};

extern std::unordered_map<MemoryType, std::string> memory_type_strings;

class MemoryMetadata {
public:
    uint8_t* gPointer;
    const size_t gLength;
    const uint64_t gLogicalAddress;
    const MemoryType gType;

    MemoryMetadata(uint8_t* pointer, size_t length, uint64_t log_address, MemoryType gType);
};

class Memory {
private:
    std::vector<MemoryMetadata*> gMemoryMappings;

public:
    uint64_t last_address = 0;
    Memory();
    ~Memory();

    MemoryMetadata* MapMemory(uint8_t* pointer, size_t length, uint64_t where, MemoryType type, bool increment_la = true);

    uint8_t ReadByteBe(uint64_t address) const;
    void WriteByteBe(uint64_t address, uint8_t value);
    void WriteWordBe(uint64_t address, uint16_t value);
    void WriteDwordBe(uint64_t address, uint32_t value);
    void WriteQwordBe(uint64_t address, uint64_t value);
    void WriteDqwordBe(uint64_t address, __uint128_t value);
    uint16_t ReadWordBe(uint64_t address) const;
    uint32_t ReadDwordBe(uint64_t address) const;
    uint64_t ReadQwordBe(uint64_t address) const;
    __uint128_t ReadDqwordBe(uint64_t address) const;

    class Iterator {
    private:
        std::vector<MemoryMetadata*>::iterator it;

    public:
        Iterator(std::vector<MemoryMetadata*>::iterator initIt);
        Iterator operator++();
        bool operator!=(const Iterator& other) const;
        MemoryMetadata* operator*() const;
    };

    Iterator begin();
    Iterator end();
};

#endif // MEMORY_H


#endif //EMULATOR_MEMORY_HPP
