//
// Created by Piotr on 12.05.2024.
//

#ifndef KITTY_OS_CPP_DRIVER_CTRL_HPP
#define KITTY_OS_CPP_DRIVER_CTRL_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kstd/kstdio.hpp>
#include <public/kdu/driver_entry.hpp>
#include <hal/bus/pci.hpp>

extern struct driver_entry_t __driver_array[];
extern struct driver_entry_t __driver_array_end[];

struct loaded_driver_descriptor
{
    uint64_t identifier;
    driver_handle_t driver_handle;
    driver_entry_t* driver;
    loaded_driver_descriptor* next;
};

void driver_ctrl_init();
void driver_ctrl_call_ald(); // Call always loadable drivers
bool driver_ctrl_find_and_call(pci_dev* dev);
void driver_ctrl_enumerate_drivers();
void driver_ctrl_print_ready_drivers();

driver_entry_t* driver_ctrl_get_driver(uint64_t dci);
int64_t driver_ctrl_find_driver_by_designation(driver_type_t type);
loaded_driver_descriptor* driver_ctrl_get_descriptors();

driver_status_t ioctl_auto(
        driver_type_t type,
        driver_handle_t* ioctl_handle,
        uint64_t ioctl_question,
        const char* ioctl_msg,
        char* ioctl_resp
);

#endif //KITTY_OS_CPP_DRIVER_CTRL_HPP
