#ifndef _HAL_HPP
#define _HAL_HPP

#include <functional>

void initialize_hal(void);
void initialize_irqs();

bool attach_irq_handler(uint8_t irq, std::function<void()> function);
// void attach_gsi_handler(uint8_t irq, std::function<void()> function);

void receive_irq(uint64_t interrupt_id, uint64_t cpu_id);

void print_ioapic_descriptors();

#endif