//
// Created by Piotr on 20.05.2024.
//

#ifndef EMULATOR_PROCESSOR_HPP
#define EMULATOR_PROCESSOR_HPP

#include <cstdint>
#include "../instructions/instruction_set.hpp"

class ISA; // forward declaration because compiler is a bitch!
class Processor;

class Processor {
public:
    Processor(uint64_t starting_vector) : instruction_pointer(starting_vector) {}

    void Reset();
    void Disable();
    void Enable();
    bool isEnabled();

    void Tick();

    void addISA(ISA* isa);
private:
    bool halted {false};

    uint64_t instruction_pointer {0};
    uint64_t stack_pointer {0};

    uint64_t registers[32];
    __int128 dqword_registers[32];

    std::vector<ISA*> instruction_sets;
};

#endif //EMULATOR_PROCESSOR_HPP
