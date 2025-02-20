//
// Created by Neko on 20.01.2025.
//

#include <acpi/acpi.h>
#include <clock/clock.h>
#include <clock/rtc.h>
#include <cstring>
#include <dbg/log.h>
#include <fb/framebuffer.h>
#include <fs/fs/virt_kernel/vk_sys.h>
#include <hal/apic/apic.h>
#include <hal/cpu/io.h>
#include <hal/gdt/gdt.h>
#include <hal/idt/idt.h>
#include <hal/ioapic/ioapic.h>
#include <hal/irq/irq.h>
#include <hal/pcie/pcie.h>
#include <kernel.h>
#include <lib/klibc/runtime-blob.h>
#include <lib/stb/stb_image.h>
#include <local_storage.h>
#include <mem/heap.h>
#include <mem/physical.h>
#include <mem/virtual.h>
#include <smp/smp.h>
#include <vfs/interfaces/vdi/vdi.h>
#include <vfs/virt_fs.h>

#include "lib/npf/nanoprintf.h"
#include "smp/sched.h"
#include "uacpi/event.h"
#include "uacpi/resources.h"
#include "uacpi/types.h"
#include "uacpi/uacpi.h"
#include "uacpi/utilities.h"

static uacpi_interrupt_ret handle_power_button(uacpi_handle ctx)
{
    /*
     * Shut down right here using the helper we have defined above.
     *
     * Note that it's generally terrible practice to run any AML from
     * an interrupt handler, as it's allowed to allocate, map, sleep,
     * stall, acquire mutexes, etc. So, if possible in your kernel,
     * instead schedule the shutdown callback to be run in a normal
     * preemptible context later.
     */
    debug_print("Power button has been pressed!\n");
    return UACPI_INTERRUPT_HANDLED;
}

static uacpi_iteration_decision match_ps2k(void *user, uacpi_namespace_node *node, uacpi_u32 node_depth)
{
    // Found a PS2 keyboard! Do initialization below.
    uacpi_resources *kb_res;

    uacpi_status ret = uacpi_get_current_resources(node, &kb_res);
    if (uacpi_unlikely_error(ret))
    {
        debug_print("unable to retrieve PS2K resources: %s", uacpi_status_to_string(ret));
        return UACPI_ITERATION_DECISION_NEXT_PEER;
    }

    debug_print("Found PS2 keyboard resources.\n");

    uacpi_free_resources(kb_res);

    return UACPI_ITERATION_DECISION_CONTINUE;
}

#define PS2K_PNP_ID "PNP0303"

void find_ps2_keyboard()
{
    uacpi_find_devices(PS2K_PNP_ID, match_ps2k, nullptr);
}

extern "C" [[noreturn]] void kernel_main(void)
{
    initialize_runtime();
    initialize_gdt();
    initialize_idt();
    initialize_physical_memory();
    initialize_virtual_memory();
    initialize_heap();
    initialize_local_storage();
    initialize_acpi();
    initialize_apic();
    initialize_ioapic();
    initialize_irq_vectors();
    //initialize_clock();
    initialize_rtc();
    initialize_pcie();
    initialize_framebuffer();

    const auto framebuffer = get_framebuffer(0);
    if (!framebuffer)
    {
        debug_print("No framebuffer\n");
    }

    draw_pixel(framebuffer, 100, 100, 255, 255, 255);
    draw_rectangle(framebuffer, 100, 100, 100, 100, 255, 100, 25);

    auto sd = get_system_date();
    debug_print("Date: %04llu-%02llu-%02llu Time: %02llu:%02llu:%02llu\n", sd.year, sd.month, sd.day, sd.hour,
                sd.minute, sd.second);

    debug_print("Hello World!\n");

    auto [status, address] = virtual_to_physical(
        get_kernel_page_map(),
        (uint64_t)&kernel_main);
    if (address) {
        debug_print("%lx\n", address);
    }

    auto hProcess = create_process("/kernel.elf", "Kernel main process", nullptr, USER, false);

    allocate_memory_in_process(hProcess, 0x10000, 16);

    uint8_t buf[8] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xef};

    debug_print("Writing to the process.\n");
    bool bWrite = process_write(hProcess, 0x10000, &buf, sizeof(buf));

    if (!bWrite)
    {
        debug_print("Couldn't write to the process.\n");
        goto _end;
    }

    uint8_t rdBuf[8];

    debug_print("Read from the process.\n");
    process_read(hProcess, 0x10000, &buf, sizeof(buf));

    for (auto v : rdBuf)
    {
        debug_print("%hhx ", v);
    }
    debug_print("\n");
_end:
    debug_print("hProcess = %p;", hProcess);

    /*
     * Start with this as the first step of the initialization. This loads all
     * tables, brings the event subsystem online, and enters ACPI mode. We pass
     * in 0 as the flags as we don't want to override any default behavior for
     * now.
     */
    uacpi_status ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret))
    {
        debug_print("uacpi_initialize error: %s", uacpi_status_to_string(ret));
        asm volatile("cli; hlt");
    }

    /*
     * Load the AML namespace. This feeds DSDT and all SSDTs to the interpreter
     * for execution.
     */
    ret = uacpi_namespace_load();
    if (uacpi_unlikely_error(ret))
    {
        debug_print("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
        asm volatile("cli; hlt");
    }

    /*
     * Initialize the namespace. This calls all necessary _STA/_INI AML methods,
     * as well as _REG for registered operation region handlers.
     */
    ret = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(ret))
    {
        debug_print("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
        asm volatile("cli; hlt");
    }

    /*
     * Tell uACPI that we have marked all GPEs we wanted for wake (even though we
     * haven't actually marked any, as we have no power management support right
     * now). This is needed to let uACPI enable all unmarked GPEs that have a
     * corresponding AML handler. These handlers are used by the firmware to
     * dynamically execute AML code at runtime to e.g. react to thermal events or
     * device hotplug.
     */
    ret = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(ret))
    {
        debug_print("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
        asm volatile("cli; hlt");
    }

    uacpi_status _ret =
        uacpi_install_fixed_event_handler(UACPI_FIXED_EVENT_POWER_BUTTON, handle_power_button, UACPI_NULL);
    if (uacpi_unlikely_error(_ret))
    {
        debug_print("failed to install power button event callback: %s", uacpi_status_to_string(ret));
    }

    while (true)
    {
        asm volatile("nop;");
    }
}
