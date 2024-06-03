//
// Created by Piotr on 03.06.2024.
//

#ifndef KITTY_OS_CPP_UNIIRQ_HPP
#define KITTY_OS_CPP_UNIIRQ_HPP

#include <kernel/hal/x64/idt/idt.hpp>

void uirq_mask_irq(int irq);
void uirq_unmask_irq(int irq);
void uirq_register_irq(int irq, idt_function_pointer fn);

void uniirq_init();

#endif //KITTY_OS_CPP_UNIIRQ_HPP
