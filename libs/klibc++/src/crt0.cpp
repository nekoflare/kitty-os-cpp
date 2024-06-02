//
// Created by Piotr on 31.05.2024.
//

#include <kcstdlib.hpp>

extern int main();

extern "C" void _start()
{
    int return_code = main();
    SysExit(return_code);
}