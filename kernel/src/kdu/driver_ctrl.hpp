//
// Created by Piotr on 12.05.2024.
//

#ifndef KITTY_OS_CPP_DRIVER_CTRL_HPP
#define KITTY_OS_CPP_DRIVER_CTRL_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kstd/kstdio.hpp>
#include <kdu/driver_entry.hpp>

extern struct driver_entry_t __driver_array[];
extern struct driver_entry_t __driver_array_end[];

bool driver_ctrl_find_and_call(pci_handle_t pci_handle);
void driver_ctrl_enumerate_drivers();

#endif //KITTY_OS_CPP_DRIVER_CTRL_HPP
