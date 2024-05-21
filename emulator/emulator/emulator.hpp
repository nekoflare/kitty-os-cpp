#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <vector>
#include <cstdint>
#include "processor.hpp"
#include "../instructions/instruction_set.hpp"

class Emulator {
public:
    void AddProcessor(uint64_t starting_vector, bool enable);
    void EmulationLoop();
    void AddInstructionSet(ISA* isa);
private:
    std::vector<Processor*> processors;
};

#endif