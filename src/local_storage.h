//
// Created by Neko on 25.01.2025.
//

#ifndef LOCAL_STORAGE_H
#define LOCAL_STORAGE_H

#define CURRENT_PROCESS_STORAGE 0
#include <cstdint>

struct thread_local_storage
{
    uint64_t stack_ptr;     // Stack pointer when switching between tasks or interrupts.
    uint64_t old_stack_ptr; // Stack pointer before swap
};

void initialize_local_storage();
void create_storage_entry(bool global, uint64_t stor_id, void *data);
void *get_storage_value(bool global, uint64_t stor_id);

#endif // LOCAL_STORAGE_H
