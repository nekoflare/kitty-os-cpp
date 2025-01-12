#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/kernel_api.h>
#include <string.h>
#include <mem/virtual_memory.hpp>
#include <mem/common_memory.hpp>
#include <acpi/acpi.hpp>
#include <hal/io.hpp>
#include <debug.hpp>

extern "C"
{

void *uacpi_kernel_alloc(uacpi_size size)
{
    auto address = new uint8_t[size];

    return static_cast<void*>(address);
}

#ifndef UACPI_SIZED_FREES
void uacpi_kernel_free(void *mem)
{
    delete static_cast<uint8_t*>(mem);
}
#else
void uacpi_kernel_free(void *mem, uacpi_size size_hint)
{
    delete static_cast<uint8_t*>(mem);
}
#endif

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{
    return 0; // todo ?
}

void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size)
{
    auto allocated = new uint8_t[count];

    memset(allocated, 0, size);
    return static_cast<void*>(allocated);
}

void uacpi_kernel_stall(uacpi_u8 usec)
{
    return; // todo ?
}

typedef struct {
    int signaled; /* 0 = not signaled, 1 = signaled */
} uacpi_event;

uacpi_handle uacpi_kernel_create_event(void)
{
    uacpi_event* event = new uacpi_event;

    event->signaled = 0;
    return (uacpi_handle)event;
}

void uacpi_kernel_free_event(uacpi_handle handle)
{
    if (handle == NULL)
    {
        return;
    }

    delete (uacpi_event*)handle;
}

/* Spinlock object structure */
class uacpi_spinlock {
public:
    uacpi_spinlock() : locked(false) {}

    void lock() {
        while (__sync_lock_test_and_set(&locked, true)) {
            /* Busy wait */
        }
    }

    void unlock() {
        __sync_lock_release(&locked);
    }

private:
    volatile bool locked;
};

/* Create a kernel spinlock */
uacpi_handle uacpi_kernel_create_spinlock(void) {
        uacpi_spinlock* spinlock = new uacpi_spinlock();
        return (uacpi_handle)spinlock;
}

/* Free a kernel spinlock */
void uacpi_kernel_free_spinlock(uacpi_handle handle) {
    if (handle == NULL) {
        return; /* Nothing to free */
    }

    /* Delete the spinlock object */
    delete static_cast<uacpi_spinlock*>(handle);
}

/* Mutex object structure */
class uacpi_mutex {
public:
    uacpi_mutex() : locked(false) {}

    void lock() {
        while (__sync_lock_test_and_set(&locked, true)) {
            /* Busy wait */
        }
    }

    void unlock() {
        __sync_lock_release(&locked);
    }

private:
    volatile bool locked;
};

/* Create a kernel mutex */
uacpi_handle uacpi_kernel_create_mutex(void) {
        uacpi_mutex* mutex = new uacpi_mutex();
        return (uacpi_handle)mutex;
}

/* Free a kernel mutex */
void uacpi_kernel_free_mutex(uacpi_handle handle) {
    if (handle == NULL) {
        return; /* Nothing to free */
    }

    /* Delete the mutex object */
    delete static_cast<uacpi_mutex*>(handle);
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
    auto virtual_address = allocate_virtual_memory_kernel(len);

    // debug_printf("uacpi_kernel_map(%p, %zu)\n", addr, len);

    addr &= 0xfffffffffffff000;

    auto aligned_size = (len + 4096) & ~4095;
    for (size_t i = 0; aligned_size / 4096 > i; i++)
    {

        virtual_map(
            addr + (i * 4096),
            virtual_address + (i * 4096),
            true,
            false,
            true,
            false,
            false,
            false,
            0,
            true
        );
    }

    return (void*)virtual_address;
}

void uacpi_kernel_unmap(void *addr, uacpi_size len)
{
    return; // eh, unmapped much? we dont check, we don't care.
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rdsp_address)
{

    *out_rdsp_address = (void*) get_rdsp_address() - get_higher_half_offset();
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read(
    uacpi_handle handle, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 *value)
{
    if (!handle || !value || (byte_width != 1 && byte_width != 2 && byte_width != 4))
    {
        return UACPI_STATUS_INVALID_ARGUMENT;
    }

    uacpi_u16 port = static_cast<uacpi_u16>((uintptr_t)handle + offset);

    // Perform the I/O read based on the byte width
    switch (byte_width)
    {
    case 1:
        *value = static_cast<uacpi_u64>(inb(port));
        break;
    case 2:
        *value = static_cast<uacpi_u64>(inw(port));
        break;
    case 4:
        *value = static_cast<uacpi_u64>(inl(port));
        break;
    default:
        return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write(
    uacpi_handle handle, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 value)
{
    if (!handle || (byte_width != 1 && byte_width != 2 && byte_width != 4))
    {
        return UACPI_STATUS_INVALID_ARGUMENT;
    }

    uacpi_u16 port = static_cast<uacpi_u16>((uintptr_t)handle + offset);

    // Perform the I/O write based on the byte width
    switch (byte_width)
    {
    case 1:
        outb(port, static_cast<uacpi_u8>(value));
        break;
    case 2:
        outw(port, static_cast<uacpi_u16>(value));
        break;
    case 4:
        outl(port, static_cast<uacpi_u32>(value));
        break;
    default:
        return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

constexpr const char* log_level_to_string(uacpi_log_level level) {
    switch (level) {
        case UACPI_LOG_DEBUG:
            return "DEBUG";
            break;
        case UACPI_LOG_TRACE:
            return "TRACE";
            break;
        case UACPI_LOG_INFO:
            return "INFO";
            break;
        case UACPI_LOG_WARN:
            return "WARN";
            break;
        case UACPI_LOG_ERROR:
            return "ERROR";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

#ifndef UACPI_FORMATTED_LOGGING
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* message) {
    auto level_str = log_level_to_string(level);

    debug_printf("[%s] %s", level_str, message);
}
#else
UACPI_PRINTF_DECL(2, 3)
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* format, ...) {
    debug_printf("uacpi_kernel_log(uacpi_log_level, const uacpi_char*, ...) not implemented.\n");
}

void uacpi_kernel_vlog(uacpi_log_level level, const uacpi_char* format, uacpi_va_list args) {
    debug_printf("uacpi_kernel_vlog(uacpi_log_level, const uacpi_char*, uacpi_va_list) not implemented");
}
#endif

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
    debug_printf("uacpi_thread_id uacpi_kernel_get_thread_id(void) not implemented.\n");
    return (uacpi_thread_id)0;
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 timeout) {
    debug_printf("uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
    debug_printf("void uacpi_kernel_release_mutex(uacpi_handle) not implemented.\n");
}

uacpi_status uacpi_kernel_pci_read(
    uacpi_handle device, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 *value
) {
    debug_printf("uacpi_kernel_pci_read(uacpi_handle, uacpi_size, uacpi_u8, uacpi_u64*) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write(
    uacpi_handle device, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 value
) {
    debug_printf("uacpi_kernel_pci_write(uacpi_handle, uacpi_size, uacpi_u8, uacpi_u64) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_map(
    uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle
) {
    debug_printf("uacpi_kernel_io_map(uacpi_io_addr, uacpi_size, uacpi_handle*) not implemented.\n");
    debug_printf("base=0x%lx, len=0x%lx\n", base, len);
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
    debug_printf("uacpi_kernel_io_unmap(uacpi_handle) not implemented.\n");
}

uacpi_status uacpi_kernel_pci_device_open(
    uacpi_pci_address address, uacpi_handle *out_handle
) {
    debug_printf("uacpi_kernel_pci_device_open(uacpi_pci_address, uacpi_handle*) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {
    debug_printf("uacpi_kernel_pci_device_close(uacpi_handle) not implemented.\n");
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
    uacpi_cpu_flags flags;

    // Store the current state of the CPU flags (whether interrupts were enabled or disabled)
    asm volatile ("pushf; pop %0" : "=rm" (flags));

    // Disable interrupts
    asm volatile ("cli");

    // Return the previous state of the CPU flags
    return flags;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags) {
    // Restore the CPU flags, enabling interrupts if they were enabled before
    if (flags) {
        asm volatile ("sti");
    }
}

uacpi_status uacpi_kernel_schedule_work(
    uacpi_work_type, uacpi_work_handler, uacpi_handle ctx
) {
    debug_printf("uacpi_status uacpi_kernel_schedule_work(\n"
                 "    uacpi_work_type, uacpi_work_handler, uacpi_handle ctx\n"
                 ") not implemented.\n");

    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx,
    uacpi_handle *out_irq_handle
) {
    debug_printf("uacpi_status uacpi_kernel_install_interrupt_handler(\n"
                 "    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx,\n"
                 "    uacpi_handle *out_irq_handle\n"
                 ") not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
    debug_printf("uacpi_status uacpi_kernel_wait_for_work_completion(void) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(
    uacpi_interrupt_handler, uacpi_handle irq_handle
) {
    debug_printf("uacpi_status uacpi_kernel_uninstall_interrupt_handler(\n"
                 "    uacpi_interrupt_handler, uacpi_handle irq_handle\n"
                 ") not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_signal_event(uacpi_handle) {
    debug_printf("void uacpi_kernel_signal_event(uacpi_handle) not implemented.\n");
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
    debug_printf("void uacpi_kernel_sleep(uacpi_u64 msec) not implemented.\n");
}

void uacpi_kernel_reset_event(uacpi_handle) {
    debug_printf("void uacpi_kernel_reset_event(uacpi_handle) not implemented.\n");
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16) {
    debug_printf("uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16) not implemented.\n");
    return false;
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*) {
    debug_printf("uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*) not implemented.\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

}