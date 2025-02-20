//
// Created by Neko on 21.01.2025.
//

#include "smp.h"

#include <hal/apic/apic.h>

uint64_t get_current_processor_id()
{
    const auto lapic_id = get_lapic_id();
    return lapic_id;
}
