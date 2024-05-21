//
// Created by Piotr on 20.05.2024.
//

#ifndef EMULATOR_INSTRUCTION_SET_HPP
#define EMULATOR_INSTRUCTION_SET_HPP

#include <unordered_map>
#include <functional>
#include <cstdint>
#include "../emulator/processor.hpp"

class Processor; // circular dependency! i hate you!
class ISA;

class ISA {
public:
    bool depends {false}; // This says if this ISA depends on some flag in the system.
    size_t aci_index {0}; // available cpu instruction set flags register
    size_t aci_bit {0}; // available cpu instruction set bit in the register
};

#endif //EMULATOR_INSTRUCTION_SET_HPP
