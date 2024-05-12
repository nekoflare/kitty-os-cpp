//
// Created by Piotr on 10.05.2024.
//

#include "serial.hpp"

static driver_status_t serial_driver_entry()
{
    return DS_SUCCESS;
}

static driver_status_t serial_driver_cleanup()
{
    return DS_SUCCESS;
}

static driver_status_t uart_start_connection(
        uart_object_t* object,
        uint64_t uart_divisor,
        bool irq_enable,
        uint64_t flags,
        size_t port_index
)
{
    object->divisor = uart_divisor;
    object->bytes_received = 0;
    object->bytes_sent = 0;
    object->flags = flags;
    object->port_index = port_index;
    object->irqs_enabled = irq_enable;

    return DS_SUCCESS;
}

static driver_status_t uart_destroy_object(
        uart_object_t* object
)
{
    return DS_SUCCESS;
}

static driver_status_t uart_write(
        uart_object_t* object,
        const char* payload,
        size_t length
)
{
    object->bytes_sent += length;
    return DS_SUCCESS;
}

static driver_status_t uart_read(
        uart_object_t* object,
        char* buffer,
        size_t length
)
{
    kstd::memset(
            buffer,
            0x20,
            length
    );

    object->bytes_received += length;

    return DS_SUCCESS;
}

static const char* driver_name = "Default serial driver";
static const char* driver_author = "Piotr 10.05.2024";
static const char* driver_description = "Allows the kernel to do basic serial actions.";
static const char* driver_version = "Pre-release 1.0.0 beta";

static serial_driver_t SerialDriverDescriptor = {
        .uart_start_connection = &uart_start_connection,
        .uart_destroy_object = &uart_destroy_object,
        .uart_write = &uart_write,
        .uart_read = &uart_read
};

driver_type driver_entry_t Driver = {
        .driver_name = driver_name,
        .driver_author = driver_author,
        .driver_description = driver_description,
        .driver_version = driver_version,
        .driver_designation = DT_SERIAL,
        .driver_load = DL_ALWAYS_LOAD,
        .driver_entry = &serial_driver_entry,
        .driver_cleanup = &serial_driver_cleanup,
        .is_loaded = false,
        .driver_descriptor = static_cast<void*>(&SerialDriverDescriptor),
};