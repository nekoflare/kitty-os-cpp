//
// Created by Piotr on 20.05.2024.
//

#include "branching.hpp"

void BranchingInstructions::JumpShortRelative(Processor *cpu) {

}

void BranchingInstructions::JumpMediumRelative(Processor *cpu) {

}

void BranchingInstructions::JumpFarRelative(Processor *cpu) {

}

std::unordered_map<uint16_t, std::function<void(Processor*)>> BranchingInstructions::instructions = {
        {0x01, BranchingInstructions::JumpShortRelative},
        {0x02, BranchingInstructions::JumpMediumRelative},
        {0x03, BranchingInstructions::JumpFarRelative}
};