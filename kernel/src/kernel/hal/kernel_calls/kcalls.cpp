//
// Created by Piotr on 31.05.2024.
//

#include "kcalls.hpp"

void kcall_handler(Registers_x86_64* regs)
{
    // kstd::printf("We've received a call!\n");
    switch (regs->rax)
    {
        case 0x00:
            kstd::printf("Kernel Process has died!\n");
            unreachable();
        case 0x01:
            kstd::printf("%s", regs->rbx);
            break;
        default:
            // Todo: Kill the process I guess?
            kstd::printf("Invalid kernel interrupt code: %llx", regs->rax);
            break;
    }
}