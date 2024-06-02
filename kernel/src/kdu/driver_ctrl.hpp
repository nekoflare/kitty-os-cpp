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
#include <kernel/hal/bus/pci.hpp>

extern struct driver_entry_t __driver_array[];
extern struct driver_entry_t __driver_array_end[];

void driver_ctrl_call_ald(); // Call always loadable drivers
bool driver_ctrl_find_and_call(pci_dev* dev);
void driver_ctrl_enumerate_drivers();

driver_status_t ioctl_auto(
        driver_type_t type,
        driver_handle_t* ioctl_handle,
        uint64_t ioctl_question,
        const char* ioctl_msg,
        char* ioctl_resp
);

#endif //KITTY_OS_CPP_DRIVER_CTRL_HPP
