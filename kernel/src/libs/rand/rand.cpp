//
// Created by Piotr on 15.06.2024.
//

#include "rand.hpp"

uint64_t seed = 1;

// Function to generate the next random number
unsigned long lcg() {
    seed = (a * seed + c) % m;
    return seed;
}