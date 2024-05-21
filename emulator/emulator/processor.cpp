//
// Created by Piotr on 20.05.2024.
//

#include "processor.hpp"

void Processor::Reset() {
    for (auto& reg : this->registers)
        reg = 0;

    for (auto &reg: this->dqword_registers)
        reg = 0;


}

void Processor::Enable() {
    this->halted = false;
}

void Processor::Disable() {
    this->halted = true;
}

bool Processor::isEnabled() {
    return !this->halted;
}

void Processor::Tick() {
    // Logic for doing the actual reading the instruction and doing stuff with it
}

void Processor::addISA(ISA *isa) {
    this->instruction_sets.push_back(isa);
}