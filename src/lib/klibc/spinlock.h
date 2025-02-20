//
// Created by Neko on 25.01.2025.
//

#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <kernel.h>

handle create_spinlock();
void lock_spinlock(handle lock);
void unlock_spinlock(handle lock);

#endif // SPINLOCK_H
