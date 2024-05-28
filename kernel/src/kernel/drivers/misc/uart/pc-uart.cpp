//
// Created by Piotr on 26.05.2024.
//

#include "pc-uart.hpp"

static driver_status_t pc_uart_entry(pci_handle_t* pci_handle)
{
    kstd::printf("[PC-UART] Starting...\n");

    kstd::printf("[PC-UART] Finished.\n");
    return DS_SUCCESS;
}

static driver_status_t pc_uart_cleanup()
{
    return DS_SUCCESS;
}

static driver_status_t pc_uart_ioctl(
        driver_handle_t* driver_handle,
        uint64_t driver_question,
        const char* question_buffer,
        char* ioctl_answer
)
{
    return DS_SUCCESS;
}

driver_type driver_entry_t pc_uart_desc = {
        .driver_name = "PC UART COM port controller.",
        .driver_author = "Piotr",
        .driver_description = "Basic UART controller.",
        .driver_version = "Pre-release 1.00",
        .driver_designation = DT_SERIAL,
        .driver_load = DL_ALWAYS_LOAD,
        .driver_entry = &pc_uart_entry,
        .driver_cleanup = &pc_uart_cleanup,
        .driver_ioctl = &pc_uart_ioctl,
        .requirements = nullptr,
        .requirements_count = 0,
        .is_loaded = false
};