//
// Created by Neko on 20.01.2025.
//

#ifndef IRQ_H
#define IRQ_H

#include <cstdint>
#include <functional>
#include <kernel.h>

#define INVALID_VECTOR 0

void initialize_irq_vectors();
status hook_irq(handle *irq_handle, bool is_gsi, const uint64_t cpu_id, uint64_t irq,
                const std::function<void(handle irq)> &handler);
void dispatch_irq(uint64_t vector);

uint64_t get_irq_from_handle(handle irq);
uint64_t get_idt_vector_from_handle(handle irq);
uint64_t get_cpu_id_from_handle(handle irq);

#endif // IRQ_H
