#include <acpi/acpi.hpp>
#include <dDraw/pixels.hpp>
#include <debug.hpp>
#include <dev/framebuffer.hpp>
#include <exe/elf/elf.hpp>
#include <hal/apic/apic.hpp>
#include <hal/hal.hpp>
#include <hal/io.hpp>
#include <hal/ioapic/ioapic.hpp>
#include <hal/pcie/pcie.hpp>
#include <mem/heap.hpp>
#include <mem/malloc.h>
#include <mem/physical_memory.hpp>
#include <mem/virtual_memory.hpp>
#include <smbios/smbios.hpp>
#include <stdint.h>
#include <string.h>
#include <sys/modules.hpp>
#include <sys/runtime.hpp>
#include <sys/symbols.hpp>
#include <sys/test/tests.hpp>
#include <sys/types.h>
#include <us/syscalls.hpp>
#include <mem/common_memory.hpp>
#include <sqlite3.h>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <utilities/utilities.hpp>

limine_framebuffer *main_framebuffer = nullptr;

void timer()
{
    debug_printf(".");
}

void set_pit_frequency(uint16_t frequency)
{
    uint16_t divisor = 1193180 / frequency;

    // Send control word to PIT (Channel 0, Mode 2, Binary)
    outb(0x43, 0x34);

    // Send the low byte of the divisor to Channel 0
    outb(0x40, (uint8_t)(divisor & 0xFF));

    // Send the high byte of the divisor to Channel 0
    outb(0x40, (uint8_t)(divisor >> 8));
}

extern "C" void kernel_main()
{
    initialize_runtime();
    initialize_framebuffer();
    initialize_flanterm();
    initialize_hal();
    initialize_phys_memory();
    initialize_virtual_memory();
    set_kernel_page_map();
    initialize_heap();
    initialize_acpi();
    initialize_smbios();
    initialize_pcie();
    initialize_apic();
    initialize_ioapic();
    initialize_irqs();
    initialize_syscall(reinterpret_cast<uint64_t>(&syscall_handler), 0x8, 0x10, 0x8, 0x10, ~(1 << 9));

    auto bgrt_width = get_bgrt_bitmap_width();
    auto bgrt_height = get_bgrt_bitmap_height();

    if (bgrt_width != -1 && bgrt_height != -1)
    {
        auto fb = get_framebuffer(0);
        draw_bgrt_bitmap(fb, fb->width - bgrt_width, fb->height - bgrt_height);
    }

    // print_smbios_entries();

    // attach_irq_handler(0, timer);
    // set_pit_frequency(2);
    // print_ioapic_descriptors();

    std::vector<uint8_t> values;

    for (size_t i = 0; 0xffff > i; i++) {
        auto value = inb(i);
        values.push_back(value);
    }
    hexdump(values.data(), values.size());

    asm volatile ("cli; hlt");

    /*
     * Start with this as the first step of the initialization. This loads all
     * tables, brings the event subsystem online, and enters ACPI mode. We pass
     * in 0 as the flags as we don't want to override any default behavior for now.
     */
    uacpi_status ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret)) {
        debug_printf("uacpi_initialize error: %s", uacpi_status_to_string(ret));
        asm volatile ("cli; hlt");
    }

    /*
     * Load the AML namespace. This feeds DSDT and all SSDTs to the interpreter
     * for execution.
     */
    ret = uacpi_namespace_load();
    if (uacpi_unlikely_error(ret)) {
        debug_printf("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
        asm volatile ("cli; hlt");
    }

    /*
     * Initialize the namespace. This calls all necessary _STA/_INI AML methods,
     * as well as _REG for registered operation region handlers.
     */
    ret = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(ret)) {
        debug_printf("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
        asm volatile ("cli; hlt");
    }

    /*
     * Tell uACPI that we have marked all GPEs we wanted for wake (even though we haven't
     * actually marked any, as we have no power management support right now). This is
     * needed to let uACPI enable all unmarked GPEs that have a corresponding AML handler.
     * These handlers are used by the firmware to dynamically execute AML code at runtime
     * to e.g. react to thermal events or device hotplug.
     */
    ret = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(ret)) {
        debug_printf("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
        asm volatile ("cli; hlt");
    }

    while (true)
    {
        asm volatile("nop");
    }
}
