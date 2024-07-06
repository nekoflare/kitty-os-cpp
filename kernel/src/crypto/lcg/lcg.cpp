//
// Created by Piotr on 15.06.2024.
//

#include <crypto/lcg/lcg.hpp>

uint64_t seed = 1;

unsigned long lcg() {
    seed = (a * seed + c) % m;
    return seed;
}