//
// Created by Piotr on 02.05.2024.
//

#include "kernel/fb/fb.hpp"
#include "kernel/hal/x64/gdt/gdt.hpp"
#include "control/control.hpp"
#include "kstd/kstdio.hpp"
#include <kernel/hal/x64/idt/idt.hpp>
#include <kernel/memory/mm.hpp>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kernel/hal/x64/bus/pci/pci.hpp>
#include <kdu/driver_entry.hpp>

extern void (*__init_array[])();
extern void (*__init_array_end[])();
extern struct driver_entry_t __driver_array[];
extern struct driver_entry_t __driver_array_end[];

extern "C" void kernel_main()
{
    for (size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }

    Framebuffer::Initialize();
    kstd::InitializeTerminal();
    flush_gdt();
    flush_idt();
    enable_interrupts();

    pmsa_initialize();
    mm_enumerate_memmap_entries(true);

    // mm_test();

    pci_init();

    kstd::printf("Drivers on the kernel: \n");

    // Calculate the size of the array
    size_t array_size = __driver_array_end - __driver_array;

    // Iterate through the array
    for (size_t i = 0; i < array_size; ++i) {
        // Access each element of the array
        struct driver_entry_t entry = __driver_array[i];
        // Process the entry as needed
        kstd::printf("=[=============================================]=\n");
        kstd::printf("Name: %s\nAuthor: %s\nDescription: %s\nVersion: %s\n", entry.driver_name, entry.driver_author, entry.driver_description, entry.driver_version);
    }
    kstd::printf("=[=============================================]=\n");
    // mm_map_pages(_Pml4e, 0xffffffff80000000, 0x6e000, 512, PROT_FORCE_MAP | PROT_RW | PROT_EXEC | PROT_SUPERVISOR, MAP_PRESENT, MISC_INVLPG, 0);

    // mm_map_page(_Pml4e, mm_create_va(false, free_pml4e, 0, 0, 0, 0), 0x6e000, PROT_RW | PROT_EXEC | PROT_SUPERVISOR, MAP_PRESENT, MISC_INVLPG, 0);

    // Find driver for serial.
    for (size_t i = 0; i < array_size; i++)
    {
        struct driver_entry_t entry = __driver_array[i];
        if (entry.driver_designation == DT_SERIAL)
        {
            const void* driver_desc = entry.driver_description;
            if (entry.driver_entry() != DS_SUCCESS)
            {
                kstd::printf("Couldn't initialize the serial driver.\n");

                return;
            }

            uart_object_t obj;
            const serial_driver_t* serial_driver = static_cast<const serial_driver_t*>(driver_desc);

            const uint8_t* serial_driver_bytes = static_cast<const uint8_t*>(driver_desc);

            for (size_t i = 0; sizeof(serial_driver_t) > i; i++)
            {
                kstd::printf("%hhx", serial_driver_bytes[i]);
            }
            kstd::printf("\n");

            break;

            driver_status_t conn_status = serial_driver->uart_start_connection(
                    &obj,
                    1,
                    false,
                    0,
                    0
            );

            if (conn_status != DS_SUCCESS)
            {
                kstd::printf("Failed to create new serial connection.\n");
                return;
            }

            const char* payload = "Hello, World!\n";
            driver_status_t write_status = serial_driver->uart_write(
                    &obj,
                    payload,
                    sizeof(payload)
            );

            if (write_status != DS_SUCCESS)
            {
                kstd::printf("Failed to write to the serial object.\n");
                return;
            }

            char buffer[64];
            size_t buffer_size = 64;
            driver_status_t read_status = serial_driver->uart_read(
                    &obj,
                    buffer,
                    buffer_size
            );

            if (read_status != DS_SUCCESS)
            {
                kstd::printf("Failed to read from the serial object.\n");
                return;
            }

            for (size_t i = 0; buffer_size > i; i++)
            {
                if (buffer[i] != 0x20)
                {
                    kstd::printf("Fail!\n");
                }
            }

            kstd::printf("Finished!\n");
        }
    }

    while (true)
    {
        asm volatile ("nop");
    }
}