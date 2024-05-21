//
// Created by Piotr on 20.05.2024.
//

#include "emulator.hpp"

void Emulator::AddProcessor(uint64_t starting_vector, bool enable) {
    this->processors.push_back(new Processor(starting_vector));
    size_t last_element = this->processors.size() - 1;
    this->processors.at(last_element)->Reset(); // reset cpu
    if (enable)
        this->processors.at(last_element)->Enable();
}

void Emulator::EmulationLoop() {
    for (auto& cpu : this->processors) {
        if (!cpu->isEnabled())
            continue;

        cpu->Tick();
    }
}

void Emulator::AddInstructionSet(ISA* isa) {
    for (auto& cpu : this->processors) {
        cpu->addISA(isa);
    }
}