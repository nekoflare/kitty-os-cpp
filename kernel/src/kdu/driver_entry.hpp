#ifndef KITTY_OS_CPP_DRIVER_MAIN_HPP
#define KITTY_OS_CPP_DRIVER_MAIN_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define driver_type __attribute__((section(".drivers")))

typedef enum driver_statuses : uint32_t {
    DS_SUCCESS,
    DS_FAILURE
} driver_status_t;

typedef enum driver_load : uint32_t {
    DL_LOAD_LAST,
    DL_LOAD_ON_PCI_MATCH,
    DL_ALWAYS_LOAD
} driver_load_t;

typedef enum driver_types : uint32_t {
    DT_SERIAL
} driver_type_t;

struct driver_entry_t {
    const char* driver_name;
    const char* driver_author;
    const char* driver_description;
    const char* driver_version;

    driver_type_t driver_designation;
    driver_load_t driver_load;
    driver_status_t (*driver_entry)();
    driver_status_t (*driver_cleanup)();

    bool is_loaded;

    __attribute__((aligned(0x10))) void* driver_descriptor;
};

constexpr uint64_t serial_driver_create_baud_rate(uint64_t divisor) {
    return 115200 / divisor;
}

struct uart_object_t {
    uint64_t port_index;
    uint64_t divisor;
    uint64_t speed;
    uint64_t flags;
    bool irqs_enabled;
    uint64_t bytes_sent;
    uint64_t bytes_received;
};

struct serial_driver_t {
    driver_status_t (*uart_start_connection)(uart_object_t*, uint64_t, bool, uint64_t, size_t); // divisor, irq_enable, flags, port_index
    driver_status_t (*uart_destroy_object)(uart_object_t*); // uart object
    driver_status_t (*uart_write)(uart_object_t*, const char*, size_t); // object, payload, length
    driver_status_t (*uart_read)(uart_object_t*, char*, size_t); // object, buffer, length
};

#endif //KITTY_OS_CPP_DRIVER_MAIN_HPP
