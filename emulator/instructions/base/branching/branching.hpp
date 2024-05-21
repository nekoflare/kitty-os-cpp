//
// Created by Piotr on 20.05.2024.
//

#ifndef EMULATOR_BRANCHING_HPP
#define EMULATOR_BRANCHING_HPP

#include "../../instruction_set.hpp"

class BranchingInstructions : public ISA {
public:
    BranchingInstructions();

    static void JumpShortRelative(Processor* cpu);
    static void JumpMediumRelative(Processor* cpu);
    static void JumpFarRelative(Processor* cpu);

    static std::unordered_map<uint16_t, std::function<void(Processor*)>> instructions;
};

#endif //EMULATOR_BRANCHING_HPP
