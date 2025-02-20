//
// Created by Neko on 20.01.2025.
//

#include "heap.h"

#include "malloc.h"
#include "physical.h"
#include "virtual.h"
#include <dbg/log.h>

constexpr const char *debug_prefix = "=================================================";
bool enable_debug_messages_new_delete = false;

constexpr uint64_t heap_size = 16ULL * 1024 * 1024 * 1024; // 16 GB reserved for the heap
uint64_t heap_start = 0;                                   // Start of the heap (kernel virtual memory)
uint64_t heap_end = 0;                                     // Current program break

void enable_cxx_allocation_debug_statements()
{
    enable_debug_messages_new_delete = true;
}
void disable_cxx_allocation_debug_statements()
{
    enable_debug_messages_new_delete = false;
}

// Expands the heap by mapping `page_count` new pages.
// Checks that we do not exceed the reserved heap area.
void expand_heap(size_t page_count)
{
    if (heap_end + page_count * PAGE_SIZE > heap_start + heap_size)
    {
        debug_print("Heap expansion exceeds reserved space.\n");
        asm volatile("cli; hlt");
    }

    for (size_t i = 0; i < page_count; i++)
    {
        auto page_address = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!page_address)
        {
            debug_print("Couldn't allocate physical page\n");
            asm volatile("cli; hlt");
        }

        bool resp = virtual_map(get_kernel_page_map(), page_address, heap_end,
                                /*user=*/true, /*writable=*/false,
                                /*present=*/true,
                                /*copy_on_write=*/false, /*executable=*/false,
                                /*global=*/false, /*flags=*/0, /*cache_disable=*/true);

        if (!resp)
        {
            debug_print("Mapping physical page to virtual address failed.\n");
            asm volatile("cli; hlt");
        }

        heap_end += PAGE_SIZE;
    }
}

// Returns configuration information. Only supports name==30.
extern "C" long sysconf(int name)
{
    debug_print("%s sysconf(%d) = ", debug_prefix, name);
    switch (name)
    {
    case 30:
        debug_print("%ld\n", static_cast<long>(get_available_usable_memory() / PAGE_SIZE));
        return static_cast<long>(get_available_usable_memory() / PAGE_SIZE); // number of pages
    default:
        debug_print("sysconf: Unknown name %d\n", name);
        asm volatile("cli; hlt");
        return -1; // Return error code instead of halting
    }
}

// Increases the program break. Negative sizes (shrinking) are not supported.
extern "C" void *sbrk(const long size)
{
    debug_print("%s sbrk(%ld) = ", debug_prefix, size);

    if (size < 0)
    {
        debug_print("%x\n", -1);
        debug_print("sbrk: Negative sizes not supported\n");
        return reinterpret_cast<void *>(-1);
    }

    const auto old_break = heap_end;
    // Align the size to a multiple of PAGE_SIZE
    const long aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    size_t page_count = aligned_size / PAGE_SIZE;

    if (page_count > 0)
    {
        expand_heap(page_count);
    }
    debug_print("%lx\n", old_break);
    return reinterpret_cast<void *>(old_break);
}

// Maps a region of virtual memory of at least 'size' bytes and backs it with
// physical pages.
extern "C" void *mmap(void *ptr, long size, long prot, long type, long handle, long arg)
{
    debug_print("%s mmap(%p, %ld, %ld, %ld, %ld, %ld) = ", debug_prefix, ptr, size, prot, type, handle, arg);

    if (size <= 0)
    {
        debug_print("%x\n", -1);
        debug_print("mmap: Invalid size %ld\n", size);
        return reinterpret_cast<void *>(-1);
    }

    const long aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    auto km_vm = allocate_virtual_memory_kernel(aligned_size);
    if (!km_vm)
    {
        debug_print("?\n");
        debug_print("mmap: Failed to allocate kernel virtual memory.\n");
        asm volatile("cli; hlt");
        __builtin_unreachable();
    }

    size_t num_pages = aligned_size / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++)
    {
        uint64_t pg = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!pg)
        {
            debug_print("mmap: Failed to allocate physical page for page %zu.\n", i);
            asm volatile("cli; hlt");
            __builtin_unreachable();
        }

        bool mapped = virtual_map(get_kernel_page_map(), pg, km_vm + (i * PAGE_SIZE),
                                  /*user=*/true, /*writable=*/false,
                                  /*present=*/true,
                                  /*copy_on_write=*/false, /*executable=*/false,
                                  /*global=*/false, /*flags=*/0, /*cache_disable=*/true);
        if (!mapped)
        {
            debug_print("?\n");
            debug_print("mmap: Failed to map physical page %zu at virtual address %p.\n", i,
                        reinterpret_cast<void *>(km_vm + (i * PAGE_SIZE)));
            asm volatile("cli; hlt");
        }
    }

    debug_print("%lx\n", km_vm);
    return reinterpret_cast<void *>(km_vm);
}

// Unmaps a previously mapped region. (Currently not implemented.)
extern "C" long munmap(void *ptr, long size)
{
    debug_print("%s munmap(%p, %ld) = -1\n", debug_prefix, ptr, size);
    // Returning error code to indicate failure.
    return -1;
}

// Initializes the heap by reserving a large block of virtual memory.
void initialize_heap()
{
    heap_start = allocate_virtual_memory_kernel(heap_size);
    if (!heap_start)
    {
        debug_print("initialize_heap: Failed to allocate virtual memory for heap.\n");
        asm volatile("cli; hlt");
    }
    heap_end = heap_start;
}

// Overloaded new and delete operators that use malloc/free.
void *operator new(size_t size)
{
    asm volatile("cli");
    if (enable_debug_messages_new_delete)
        debug_print("new u8[%zu] = ", size);
    void *ptr = malloc(size);
    if (enable_debug_messages_new_delete)
        debug_print("%p\n", ptr);
    asm volatile("sti");
    return ptr;
}

void *operator new[](size_t size)
{
    asm volatile("cli");
    if (enable_debug_messages_new_delete)
        debug_print("new u8[%zu] = ", size);
    void *ptr = malloc(size);
    if (enable_debug_messages_new_delete)
        debug_print("%p\n", ptr);
    asm volatile("sti");
    return ptr;
}

void operator delete(void *ptr, size_t /*size*/) noexcept
{
    asm volatile("cli");
    if (enable_debug_messages_new_delete)
        debug_print("delete %p (sized)\n", ptr);
    if (ptr)
    {
        free(ptr);
    }
    asm volatile("sti");
}

void operator delete(void *ptr) noexcept
{
    asm volatile("cli");
    if (enable_debug_messages_new_delete)
        debug_print("delete %p (regular)\n", ptr);
    if (ptr)
    {
        free(ptr);
    }
    asm volatile("sti");
}

void operator delete[](void *ptr)
{
    asm volatile("cli");
    if (enable_debug_messages_new_delete)
        debug_print("delete[] %p (table)\n", ptr);
    if (ptr)
    {
        free(ptr);
    }
    asm volatile("sti");
}

void operator delete[](void *ptr, size_t /*size*/) noexcept
{
    asm volatile("cli");
    debug_print("delete[] %p (sized, table)\n", ptr);
    if (ptr)
    {
        free(ptr);
    }
    asm volatile("sti");
}
