//
// Created by Piotr on 31.05.2024.
//

#include <cstdint>
#include <Kitty.hpp>

int test = 20;
// TODO: The memory isn't aligned! The page is here but data not there! Do something idiot!


int main()
{
    if (test == 10)
        KernWrite("10");
    else
        KernWrite("!10");
    return 0;
}
