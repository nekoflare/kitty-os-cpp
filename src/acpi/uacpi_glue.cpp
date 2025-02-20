//
// Created by Neko on 19.02.2025.
//

#include <cstring>
#include <uacpi/event.h>
#include <uacpi/kernel_api.h>
#include <uacpi/uacpi.h>

#include "acpi.h"
#include "dbg/log.h"
#include "hal/cpu/io.h"
#include "hal/irq/irq.h"
#include "hal/pcie/pcie.h"
#include "mem/heap.h"
#include "mem/virtual.h"
#include "smp/smp.h"

constexpr bool enable_debug = false;

extern "C"
{
    void *uacpi_kernel_alloc(uacpi_size size)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_alloc(size=%zu)\n", size);
        }
        // enable_cxx_allocation_debug_statements();
        auto data = new uint8_t[size];
        // disable_cxx_allocation_debug_statements();
        void *ret = data;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_alloc with return %p\n", ret);
        }
        return ret;
    }

#ifndef UACPI_SIZED_FREES
    void uacpi_kernel_free(void *mem)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_free(mem=%p)\n", mem);
        }
        delete static_cast<uint8_t *>(mem);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_free\n");
        }
    }
#else
    void uacpi_kernel_free(void *mem, uacpi_size size_hint)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_free(mem=%p, size_hint=%zu)\n", mem, size_hint);
        }
        delete static_cast<uint8_t *>(mem);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_free\n");
        }
    }
#endif

    uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_get_nanoseconds_since_boot()\n");
        }
        uacpi_u64 ret = 0; // todo ?
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_get_nanoseconds_since_boot with return %llu\n", ret);
        }
        return ret;
    }

    void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_calloc(count=%zu, size=%zu)\n", count, size);
        }
        auto allocated = new uint8_t[count];
        memset(allocated, 0, size);
        void *ret = static_cast<void *>(allocated);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_calloc with return %p\n", ret);
        }
        return ret;
    }

    void uacpi_kernel_stall(uacpi_u8 usec)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_stall(usec=%u)\n", usec);
        }
        // todo ?
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_stall\n");
        }
    }

    typedef struct
    {
        int signaled; /* 0 = not signaled, 1 = signaled */
    } uacpi_event;

    uacpi_handle uacpi_kernel_create_event(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_create_event()\n");
        }
        auto *event = new uacpi_event;
        event->signaled = 0;
        uacpi_handle ret = (uacpi_handle)event;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_create_event with return %p\n", ret);
        }
        return ret;
    }

    void uacpi_kernel_free_event(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_free_event(handle=%p)\n", handle);
        }
        if (handle == nullptr)
        {
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_free_event early (handle is nullptr)\n");
            }
            return;
        }
        delete static_cast<uacpi_event *>(handle);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_free_event\n");
        }
    }

    /* Spinlock object structure */
    class uacpi_spinlock
    {
      public:
        uacpi_spinlock() : locked(false)
        {
        }

        void lock()
        {
            while (__sync_lock_test_and_set(&locked, true))
            {
                /* Busy wait */
            }
        }

        void unlock()
        {
            __sync_lock_release(&locked);
        }

      private:
        volatile bool locked;
    };

    /* Create a kernel spinlock */
    uacpi_handle uacpi_kernel_create_spinlock(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_create_spinlock()\n");
        }
        auto *spinlock = new uacpi_spinlock();
        uacpi_handle ret = (uacpi_handle)spinlock;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_create_spinlock with return %p\n", ret);
        }
        return ret;
    }

    /* Free a kernel spinlock */
    void uacpi_kernel_free_spinlock(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_free_spinlock(handle=%p)\n", handle);
        }
        if (handle == nullptr)
        {
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_free_spinlock early (handle is nullptr)\n");
            }
            return;
        }
        delete static_cast<uacpi_spinlock *>(handle);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_free_spinlock\n");
        }
    }

    /* Mutex object structure */
    class uacpi_mutex
    {
      public:
        uacpi_mutex() : locked(false)
        {
        }

        void lock()
        {
            while (__sync_lock_test_and_set(&locked, true))
            {
                /* Busy wait */
            }
        }

        void unlock()
        {
            __sync_lock_release(&locked);
        }

      private:
        volatile bool locked;
    };

    /* Create a kernel mutex */
    uacpi_handle uacpi_kernel_create_mutex(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_create_mutex()\n");
        }
        auto *mutex = new uacpi_mutex();
        uacpi_handle ret = (uacpi_handle)mutex;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_create_mutex with return %p\n", ret);
        }
        return ret;
    }

    /* Free a kernel mutex */
    void uacpi_kernel_free_mutex(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_free_mutex(handle=%p)\n", handle);
        }
        if (handle == nullptr)
        {
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_free_mutex early (handle is nullptr)\n");
            }
            return;
        }
        delete static_cast<uacpi_mutex *>(handle);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_free_mutex\n");
        }
    }

    void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_map(addr=%lx, len=%zu)\n", addr, len);
        }
        auto virtual_address = allocate_virtual_memory_kernel(len);
        addr &= 0xfffffffffffff000;
        auto aligned_size = (len + 4096) & ~4095;
        for (size_t i = 0; aligned_size / 4096 > i; i++)
        {
            virtual_map(get_kernel_page_map(), addr + (i * 4096), virtual_address + (i * 4096), true, false, true,
                        false, false, false, 0, true);
        }
        void *ret = reinterpret_cast<void *>(virtual_address);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_map with return %p\n", ret);
        }
        return ret;
    }

    void uacpi_kernel_unmap(void *addr, uacpi_size len)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_unmap(addr=%p, len=%zu)\n", addr, len);
        }
        // eh, unmapped much? we don't check, we don't care.
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_unmap\n");
        }
    }

    uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_get_rsdp(out_rsdp_address=%p)\n", out_rsdp_address);
        }
        *out_rsdp_address = reinterpret_cast<uacpi_phys_addr>(get_rsdp_address() - get_higher_half_memory_offset());
        uacpi_status ret = UACPI_STATUS_OK;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_get_rsdp with return %d, *out_rsdp_address=%lx\n", ret,
                        *out_rsdp_address);
        }
        return ret;
    }

    struct io_map_t
    {
        uacpi_io_addr base;
        uacpi_size len;
    };

    uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_io_read(handle=%p, offset=%zu, byte_width=%u, "
                        "value=%p)\n",
                        handle, offset, byte_width, value);
        }
        if (!handle || !value || (byte_width != 1 && byte_width != 2 && byte_width != 4))
        {
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_io_read early: invalid argument(s)\n");
            }
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        auto _port = static_cast<io_map_t *>(handle);
        auto port = static_cast<uacpi_u16>(_port->base + offset);
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
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_io_read early: invalid byte_width\n");
            }
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_io_read with return %d, *value=%llu\n", UACPI_STATUS_OK, *value);
        }
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_io_write(handle=%p, offset=%zu, byte_width=%u, "
                        "value=%llu)\n",
                        handle, offset, byte_width, value);
        }
        if (!handle || (byte_width != 1 && byte_width != 2 && byte_width != 4))
        {
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_io_write early: invalid argument(s)\n");
            }
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        const auto _port = static_cast<io_map_t *>(handle);
        const auto port = static_cast<uacpi_u16>(_port->base + offset);
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
            if constexpr (enable_debug)
            {
                debug_print("Exiting uacpi_kernel_io_write early: invalid byte_width\n");
            }
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_io_write with return %d\n", UACPI_STATUS_OK);
        }
        return UACPI_STATUS_OK;
    }

    constexpr const char *log_level_to_string(uacpi_log_level level)
    {
        switch (level)
        {
        case UACPI_LOG_DEBUG:
            return "DEBUG";
        case UACPI_LOG_TRACE:
            return "TRACE";
        case UACPI_LOG_INFO:
            return "INFO";
        case UACPI_LOG_WARN:
            return "WARN";
        case UACPI_LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

#ifndef UACPI_FORMATTED_LOGGING
    void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *message)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_log(level=%d, message=%s)\n", level, message);
        }
        auto level_str = log_level_to_string(level);
        debug_print("[%s] %s", level_str, message);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_log\n");
        }
    }
#else
    UACPI_PRINTF_DECL(2, 3)
    void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *format, ...)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_log(uacpi_log_level, const uacpi_char*, ...)\n");
        }
        debug_print("uacpi_kernel_log(uacpi_log_level, const uacpi_char*, ...) not "
                    "implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_log\n");
        }
    }

    void uacpi_kernel_vlog(uacpi_log_level level, const uacpi_char *format, uacpi_va_list args)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_vlog(uacpi_log_level, const uacpi_char*, "
                        "uacpi_va_list)\n");
        }
        debug_print("uacpi_kernel_vlog(uacpi_log_level, const uacpi_char*, "
                    "uacpi_va_list) not implemented");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_vlog\n");
        }
    }
#endif

    uacpi_thread_id uacpi_kernel_get_thread_id(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_get_thread_id()\n");
        }
        uacpi_thread_id ret = nullptr;
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_get_thread_id with return %p\n", ret);
        }
        return ret;
    }

    struct mutex_t
    {
        bool locked;
    };

    uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 timeout)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_acquire_mutex(handle=%p, timeout=%u)\n", handle, timeout);
        }
        auto *mutex = static_cast<mutex_t *>(handle);
        while (timeout > 0)
        {
            if (!mutex->locked)
            {
                mutex->locked = true;
                if constexpr (enable_debug)
                {
                    debug_print("Exiting uacpi_kernel_acquire_mutex with return %d (mutex "
                                "acquired)\n",
                                UACPI_STATUS_OK);
                }
                return UACPI_STATUS_OK;
            }
            asm volatile("xor %%bx, %%bx" ::: "bx");
            --timeout;
        }
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_acquire_mutex with return %d (timeout)\n", UACPI_STATUS_TIMEOUT);
        }
        return UACPI_STATUS_TIMEOUT;
    }
    void uacpi_kernel_release_mutex(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_release_mutex(handle=%p)\n", handle);
        }
        auto *mutex = static_cast<mutex_t *>(handle);
        if (mutex->locked)
        {
            mutex->locked = false;
        }
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_release_mutex\n");
        }
    }

    struct pci_device_t
    {
        uacpi_pci_address pci_address; // its a struct bruh
    };

    uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_pci_read(device=%p, offset=%zu, byte_width=%u, "
                        "value=%p)\n",
                        device, offset, byte_width, value);
        }
        if (!device || !value)
        {
            debug_print("Invalid arguments: device=%p, value=%p\n", device, value);
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        if (byte_width != 1 && byte_width != 2 && byte_width != 4 && byte_width != 8)
        {
            debug_print("Unsupported byte width: %u\n", byte_width);
            return UACPI_STATUS_INVALID_ARGUMENT;
        }
        auto *pci_device = static_cast<pci_device_t *>(device);
        if (!pci_device)
        {
            debug_print("Device is not a valid PCI device.\n");
            return UACPI_STATUS_UNIMPLEMENTED;
        }
        auto p_segment = pci_device->pci_address.segment;
        auto p_bus = pci_device->pci_address.bus;
        auto slot = pci_device->pci_address.device;
        auto p_function = pci_device->pci_address.function;
        uint8_t buffer[8] = {0}; // Maximum byte width is 8 (uacpi_u64).
        pcie_raw_read(p_segment, p_bus, slot, p_function, offset, buffer, byte_width);
        memcpy(value, buffer, byte_width);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_pci_read with return %d, *value=%llu\n", UACPI_STATUS_OK, *value);
        }
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_pci_write(device=%p, offset=%zu, byte_width=%u, "
                        "value=%llu)\n",
                        device, offset, byte_width, value);
        }
        debug_print("uacpi_kernel_pci_write(uacpi_handle, uacpi_size, uacpi_u8, "
                    "uacpi_u64) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_pci_write with return %d\n", UACPI_STATUS_UNIMPLEMENTED);
        }
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_handle create_io_handle(uacpi_io_addr base, uacpi_size len)
    {
        if constexpr (enable_debug)
        {
            debug_print("create_io_handle(base=%lx, len=%zu)\n", base, len);
        }
        auto *map = new io_map_t;
        map->base = base;
        map->len = len;
        uacpi_handle ret = (uacpi_handle)map;
        if constexpr (enable_debug)
        {
            debug_print("Exiting create_io_handle with return %p\n", ret);
        }
        return ret;
    }

    uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_io_map(base=%lx, len=%zu, out_handle=%p)\n", base, len, out_handle);
        }
        *out_handle = create_io_handle(base, len);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_io_map with return %d, *out_handle=%p\n", UACPI_STATUS_OK, *out_handle);
        }
        return UACPI_STATUS_OK;
    }

    void uacpi_kernel_io_unmap(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_io_unmap(handle=%p)\n", handle);
        }
        delete static_cast<io_map_t *>(handle);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_io_unmap\n");
        }
    }

    uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_pci_device_open(..., out_handle=%p)\n", out_handle);
        }
        auto *dev = new pci_device_t;
        dev->pci_address = address;
        *out_handle = static_cast<uacpi_handle>(dev);
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_pci_device_open with return %d, *out_handle=%p\n", UACPI_STATUS_OK,
                        *out_handle);
        }
        return UACPI_STATUS_OK;
    }

    void uacpi_kernel_pci_device_close(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_pci_device_close(handle=%p)\n", handle);
        }
        debug_print("uacpi_kernel_pci_device_close(uacpi_handle) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_pci_device_close\n");
        }
    }

    uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_lock_spinlock(handle=%p)\n", handle);
        }
        uacpi_cpu_flags flags;
        asm volatile("pushf; pop %0" : "=rm"(flags));
        asm volatile("cli");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_lock_spinlock with return %lx\n", flags);
        }
        return flags;
    }

    void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_unlock_spinlock(handle=%p, flags=%lx)\n", handle, flags);
        }
        if (flags)
        {
            asm volatile("sti");
        }
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_unlock_spinlock\n");
        }
    }

    uacpi_status uacpi_kernel_schedule_work(uacpi_work_type work_type, uacpi_work_handler handler, uacpi_handle ctx)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_schedule_work(work_type, handler, ctx=%p)\n", ctx);
        }
        debug_print("uacpi_status uacpi_kernel_schedule_work(uacpi_work_type, "
                    "uacpi_work_handler, uacpi_handle ctx) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_schedule_work with return %d\n", UACPI_STATUS_UNIMPLEMENTED);
        }
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    struct interrupt_handler_t
    {
        uacpi_handle handle;
        uacpi_u32 irq;
        uacpi_interrupt_handler handler;
    };

    std::vector<interrupt_handler_t *> interrupt_handlers;

    uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler int_handler,
                                                        uacpi_handle ctx, uacpi_handle *out_irq_handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_install_interrupt_handler(irq=%u, int_handler, "
                        "ctx=%p, out_irq_handle=%p)\n",
                        irq, ctx, out_irq_handle);
        }
        auto *handler = new interrupt_handler_t;
        handler->irq = irq;
        handler->handle = ctx;
        handler->handler = int_handler;
        *out_irq_handle = static_cast<uacpi_handle>(handler);
        interrupt_handlers.push_back(handler);
        handle irq_handle;
        hook_irq(&irq_handle, true, get_current_processor_id(), irq, [](handle irq) {
            for (auto &interrupt_handler : interrupt_handlers)
            {
                if (interrupt_handler->irq == get_irq_from_handle(irq))
                {
                    interrupt_handler->handler(interrupt_handler->handle);
                }
            }
        });
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_install_interrupt_handler with return "
                        "%d, *out_irq_handle=%p\n",
                        UACPI_STATUS_OK, *out_irq_handle);
        }
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_wait_for_work_completion(void)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_wait_for_work_completion()\n");
        }
        debug_print("uacpi_status uacpi_kernel_wait_for_work_completion(void) not "
                    "implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_wait_for_work_completion with return %d\n", UACPI_STATUS_UNIMPLEMENTED);
        }
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler, uacpi_handle irq_handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_uninstall_interrupt_handler(irq_handle=%p)\n", irq_handle);
        }
        debug_print("uacpi_status uacpi_kernel_uninstall_interrupt_handler(\n"
                    "    uacpi_interrupt_handler, uacpi_handle irq_handle\n"
                    ") not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_uninstall_interrupt_handler with return %d\n",
                        UACPI_STATUS_UNIMPLEMENTED);
        }
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    void uacpi_kernel_signal_event(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_signal_event(handle=%p)\n", handle);
        }
        debug_print("uacpi_kernel_signal_event(uacpi_handle) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_signal_event\n");
        }
    }

    void uacpi_kernel_sleep(uacpi_u64 msec)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_sleep(msec=%llu)\n", msec);
        }
        debug_print("uacpi_kernel_sleep(uacpi_u64 msec) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_sleep\n");
        }
    }

    void uacpi_kernel_reset_event(uacpi_handle handle)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_reset_event(handle=%p)\n", handle);
        }
        debug_print("uacpi_kernel_reset_event(uacpi_handle) not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_reset_event\n");
        }
    }

    uacpi_status uacpi_kernel_pci_read8(uacpi_handle _device, uacpi_size offset, uacpi_u8 *value)
    {
        auto *device = static_cast<pci_device_t *>(_device);
        pcie_raw_read(device->pci_address.segment, device->pci_address.bus, device->pci_address.device,
                      device->pci_address.function, offset, (uint8_t *)value, sizeof(uacpi_u8));
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_pci_read16(uacpi_handle _device, uacpi_size offset, uacpi_u16 *value)
    {
        auto *device = static_cast<pci_device_t *>(_device);
        pcie_raw_read(device->pci_address.segment, device->pci_address.bus, device->pci_address.device,
                      device->pci_address.function, offset, (uint8_t *)value, sizeof(uacpi_u16));
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_pci_read32(uacpi_handle _device, uacpi_size offset, uacpi_u32 *value)
    {
        auto *device = static_cast<pci_device_t *>(_device);
        pcie_raw_read(device->pci_address.segment, device->pci_address.bus, device->pci_address.device,
                      device->pci_address.function, offset, (uint8_t *)value, sizeof(uacpi_u32));
        return UACPI_STATUS_OK;
    }

    uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_status uacpi_kernel_io_read8(uacpi_handle, uacpi_size offset, uacpi_u8 *out_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }
    uacpi_status uacpi_kernel_io_read16(uacpi_handle, uacpi_size offset, uacpi_u16 *out_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }
    uacpi_status uacpi_kernel_io_read32(uacpi_handle, uacpi_size offset, uacpi_u32 *out_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_status uacpi_kernel_io_write8(uacpi_handle, uacpi_size offset, uacpi_u8 in_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }
    uacpi_status uacpi_kernel_io_write16(uacpi_handle, uacpi_size offset, uacpi_u16 in_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }
    uacpi_status uacpi_kernel_io_write32(uacpi_handle, uacpi_size offset, uacpi_u32 in_value)
    {
        return UACPI_STATUS_UNIMPLEMENTED;
    }

    uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 timeout)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_wait_for_event(handle=%p, timeout=%u)\n", handle, timeout);
        }
        debug_print("uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16) "
                    "not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_wait_for_event with return false\n");
        }
        return false;
    }

    uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *req)
    {
        if constexpr (enable_debug)
        {
            debug_print("uacpi_kernel_handle_firmware_request(req=%p)\n", req);
        }
        debug_print("uacpi_status "
                    "uacpi_kernel_handle_firmware_request(uacpi_firmware_request*) "
                    "not implemented.\n");
        if constexpr (enable_debug)
        {
            debug_print("Exiting uacpi_kernel_handle_firmware_request with return %d\n", UACPI_STATUS_UNIMPLEMENTED);
        }
        return UACPI_STATUS_UNIMPLEMENTED;
    }
}
