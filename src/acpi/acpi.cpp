#include <acpi/acpi.hpp>
#include <dDraw/bmp.hpp>
#include <dDraw/pixels.hpp>
#include <debug.hpp>
#include <dev/framebuffer.hpp>
#include <limine.h>
#include <mem/common_memory.hpp>
#include <mem/virtual_memory.hpp>
#include <string.h>

// doesnt have to be public. acpi will initialize it.
void initialize_bgrt();

limine_rsdp_request rsdp_request = {.id = LIMINE_RSDP_REQUEST, .revision = 0, .response = nullptr};

struct rsdp *rsdp = nullptr;
struct rsdt *rsdt = nullptr;
struct xsdt *xsdt = nullptr;
bool using_xsdt = false;
size_t table_count = 0;
uint64_t table_addresses[1024] = {0}; // a table that contains virtual addresses to all tables found.

size_t local_apic_count = 0;
local_apic_entry local_apics[256] = {};

size_t io_apic_count = 0;
io_apic_entry io_apics[256] = {};

size_t iso_count = 0; // Interrupt Source Override
interrupt_source_override_entry isos[256] = {};

size_t nmi_count = 0; // Non-Maskable Interrupt
non_maskable_interrupt_entry nmis[256] = {};

void* get_rdsp_address()
{
    return (void*) rsdp;
}

static void add_table(uint64_t physical_address)
{
    uint64_t virtual_address = physical_address + get_higher_half_offset();
    table_addresses[table_count] = virtual_address;
    table_count++;
}

static void print_table_signatures()
{
    debug_printf("Found tables: ");
    for (size_t table_index = 0; table_count > table_index; table_index++)
    {
        auto table_pointer = table_addresses[table_index];
        auto table_structure = reinterpret_cast<struct sdt *>(table_pointer);
        debug_printf("%.4s ", table_structure->signature);
    }
    debug_printf("\n");
}

static void traverse()
{
    if (using_xsdt)
    {
        auto xsdt_address = reinterpret_cast<uintptr_t>(xsdt);
        xsdt_address += sizeof(struct sdt);
        size_t entry_count = (xsdt->sdt.length - sizeof(struct sdt)) / 8;

        for (size_t table_index = 0; entry_count > table_index; table_index++)
        {
            auto address = reinterpret_cast<uint64_t *>(xsdt_address + table_index * 8);
            add_table(*address);
        }
    }
    else
    {
        // using rsdt
        auto rsdt_address = reinterpret_cast<uintptr_t>(rsdt);
        rsdt_address += sizeof(struct sdt);
        size_t entry_count = (rsdt->sdt.length - sizeof(struct sdt)) / 4;

        for (size_t table_index = 0; entry_count > table_index; table_index++)
        {
            auto address = reinterpret_cast<uint32_t *>(rsdt_address + table_index * 4);
            add_table(static_cast<uint64_t>(*address));
        }
    }
}

void *get_acpi_table(const char signature[4])
{
    for (size_t table_index = 0; table_count > table_index; table_index++)
    {
        auto table_pointer = table_addresses[table_index];
        auto table_structure = reinterpret_cast<struct sdt *>(table_pointer);

        if (memcmp(&table_structure->signature, signature, 4) == 0)
        {
            return reinterpret_cast<void *>(table_pointer);
        }
    }

    return nullptr; // no table found.
}

void initialize_acpi()
{
    auto rsdp_response = rsdp_request.response;
    if (rsdp_response == nullptr)
    {
        debug_printf("RSDP not found.\n");
        asm volatile("cli; hlt");
    }

    rsdp = reinterpret_cast<struct rsdp *>(rsdp_request.response->address);

    if (rsdp->revision >= 2)
    {
        // it's using XSDT
        xsdt = reinterpret_cast<struct xsdt *>(get_higher_half_offset() + rsdp->xsdt_address);
        using_xsdt = true;
    }
    else
    {
        // it's using RSDT
        rsdt = reinterpret_cast<struct rsdt *>(get_higher_half_offset() + rsdp->rsdt_address);
    }

    if (using_xsdt)
    {
        debug_printf("Using XSDT\n");
    }
    else
    {
        debug_printf("Using RSDT\n");
    }

    traverse();
    print_table_signatures();
    parse_apic_table();
    display_apic_entries();
    initialize_bgrt();
}

uint64_t lapic_address = 0;

uint64_t get_lapic_address()
{
    return lapic_address;
}

size_t get_local_apic_count()
{
    return local_apic_count;
}

local_apic_entry *get_local_apics()
{
    return local_apics;
}

size_t get_io_apic_count()
{
    return io_apic_count;
}

io_apic_entry *get_io_apics()
{
    return io_apics;
}

size_t get_iso_count()
{
    return iso_count;
}

interrupt_source_override_entry *get_isos()
{
    return isos;
}

size_t get_nmi_count()
{
    return nmi_count;
}

non_maskable_interrupt_entry *get_nmis()
{
    return nmis;
}

void parse_apic_table()
{
    auto apic_table = reinterpret_cast<struct apic *>(get_acpi_table(APIC_SIGNATURE));
    if (apic_table == nullptr)
    {
        debug_printf("APIC table not found.\n");
        return;
    }

    lapic_address = apic_table->local_controller_address;

    debug_printf("Parsing APIC table...\n");
    debug_printf("Local APIC address: 0x%08X\n", apic_table->local_controller_address);
    debug_printf("Flags: 0x%08X\n", apic_table->flags);

    // Parse APIC entries
    uintptr_t current_entry = reinterpret_cast<uintptr_t>(apic_table) + sizeof(struct apic);
    uintptr_t table_end = reinterpret_cast<uintptr_t>(apic_table) + apic_table->sdt.length;

    while (current_entry < table_end)
    {
        auto entry = reinterpret_cast<struct apic_entry *>(current_entry);

        switch (entry->type)
        {
        case LOCAL_APIC:
            if (local_apic_count < 256)
            {
                local_apics[local_apic_count] = *reinterpret_cast<struct local_apic_entry *>(entry);
                local_apic_count++;
            }
            break;

        case IO_APIC:
            if (io_apic_count < 256)
            {
                io_apics[io_apic_count] = *reinterpret_cast<struct io_apic_entry *>(entry);
                io_apic_count++;
            }
            break;

        case INTERRUPT_SOURCE_OVERRIDE:
            if (iso_count < 256)
            {
                isos[iso_count] = *reinterpret_cast<struct interrupt_source_override_entry *>(entry);
                iso_count++;
            }
            break;

        case NON_MASKABLE_INTERRUPT:
            if (nmi_count < 256)
            {
                nmis[nmi_count] = *reinterpret_cast<struct non_maskable_interrupt_entry *>(entry);
                nmi_count++;
            }
            break;

        default:
            debug_printf("Unknown APIC entry type: %u\n", entry->type);
            break;
        }

        current_entry += entry->length;
    }
}

void display_apic_entries()
{
    debug_printf("Local APICs (%zu):\n", get_local_apic_count());
    for (size_t i = 0; i < get_local_apic_count(); i++)
    {
        auto &entry = get_local_apics()[i];
        debug_printf("  Processor ID=%u, APIC ID=%u, Flags=0x%08X\n", entry.processor_id, entry.apic_id, entry.flags);
    }

    debug_printf("IO APICs (%zu):\n", get_io_apic_count());
    for (size_t i = 0; i < get_io_apic_count(); i++)
    {
        auto &entry = get_io_apics()[i];
        debug_printf("  IO APIC ID=%u, Address=0x%08X, GSI Base=%u\n", entry.io_apic_id, entry.io_apic_address,
                     entry.global_system_interrupt_base);
    }

    debug_printf("Interrupt Source Overrides (%zu):\n", get_iso_count());
    for (size_t i = 0; i < get_iso_count(); i++)
    {
        auto &entry = get_isos()[i];
        debug_printf("  Bus=%u, Source=%u, GSI=%u, Flags=0x%04X\n", entry.bus, entry.source,
                     entry.global_system_interrupt, entry.flags);
    }

    debug_printf("Non-Maskable Interrupts (%zu):\n", get_nmi_count());
    for (size_t i = 0; i < get_nmi_count(); i++)
    {
        auto &entry = get_nmis()[i];
        debug_printf("  Processor ID=%u, Flags=0x%04X, LINT=%u\n", entry.processor_id, entry.flags, entry.lint);
    }
}

bgrt *bgrt_table = nullptr; // Pointer to the BGRT table, if found

// Initialize BGRT by locating the table
void initialize_bgrt()
{
    bgrt_table = reinterpret_cast<bgrt *>(get_acpi_table(BGRT_SIGNATURE));
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table not found.\n");
        return;
    }

    debug_printf("BGRT table found at address: %p\n", bgrt_table);
    debug_printf("Version: %u\n", bgrt_table->version);
    debug_printf("Image Type: %u\n", bgrt_table->image_type);
    debug_printf("Image Address: 0x%016lX\n", bgrt_table->image_address);
    debug_printf("Image Offset: (%u, %u)\n", bgrt_table->image_offset_x, bgrt_table->image_offset_y);
}

bgrt *get_bgrt_address()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
    }
    return bgrt_table;
}

uint64_t get_bgrt_bitmap_address()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return 0;
    }
    return bgrt_table->image_address;
}

std::pair<uint32_t, uint32_t> get_bgrt_image_offsets()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return {0, 0};
    }

    // Copy the packed fields into local variables
    uint32_t offset_x = bgrt_table->image_offset_x;
    uint32_t offset_y = bgrt_table->image_offset_y;

    return {offset_x, offset_y};
}

bgrt_status get_bgrt_status()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return {0, 0, 0}; // Default invalid status
    }
    return bgrt_table->status;
}

void display_bgrt_details()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return;
    }

    debug_printf("=== BGRT Table Details ===\n");
    debug_printf("Address: %p\n", bgrt_table);
    debug_printf("Version: %u\n", bgrt_table->version);
    debug_printf("Image Type: %u\n", bgrt_table->image_type);
    debug_printf("Image Address: 0x%016lX\n", bgrt_table->image_address);
    debug_printf("Image Offset: (%u, %u)\n", bgrt_table->image_offset_x, bgrt_table->image_offset_y);
    debug_printf("Status: Displayed=%u, Orientation=%u\n", bgrt_table->status.displayed,
                 bgrt_table->status.orientation);
}

void draw_bgrt_bitmap(limine_framebuffer *framebuffer, size_t x_pos, size_t y_pos)
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized. Cannot draw bitmap.\n");
        return;
    }

    // Fetch the physical address of the image
    uint64_t image_physical_address = bgrt_table->image_address;
    if (image_physical_address == 0)
    {
        debug_printf("BGRT image address is invalid.\n");
        return;
    }

    // Map the physical address to a virtual address
    uint8_t *bitmap_data = reinterpret_cast<uint8_t *>(image_physical_address + get_higher_half_offset());

    // Parse the BMP header
    auto bmp_header = reinterpret_cast<BITMAPFILEHEADER *>(bitmap_data);
    auto dib_header = reinterpret_cast<BITMAPINFOHEADER *>(bitmap_data + sizeof(BITMAPFILEHEADER));

    // Validate BMP signature
    if (bmp_header->bfType != 0x4D42)
    { // 'BM' in little endian
        debug_printf("Invalid BMP signature.\n");
        return;
    }

    // Get image properties
    int32_t width = dib_header->biWidth;
    int32_t height = dib_header->biHeight;
    uint16_t bits_per_pixel = dib_header->biBitCount;
    uint32_t pixel_array_offset = bmp_header->bfOffBits;

    if (bits_per_pixel != 24)
    { // We only handle 24-bit bitmaps here
        debug_printf("Unsupported BMP format: Only 24-bit images are supported.\n");
        return;
    }

    // Pixel array starts at this offset
    uint8_t *pixel_array = bitmap_data + pixel_array_offset;

    // Determine row padding
    size_t row_padding = (4 - (width * 3) % 4) % 4;

    // Render the image (BMP stores rows bottom-to-top)
    for (int32_t row = 0; row < height; row++)
    {
        for (int32_t col = 0; col < width; col++)
        {
            size_t bmp_x = col;
            size_t bmp_y = height - row - 1; // Flip vertically

            // Calculate the position in the pixel array
            size_t pixel_index = bmp_y * (width * 3 + row_padding) + bmp_x * 3;

            // Extract BGR values
            uint8_t b = pixel_array[pixel_index];
            uint8_t g = pixel_array[pixel_index + 1];
            uint8_t r = pixel_array[pixel_index + 2];

            // Draw the pixel to the framebuffer
            draw_pixel(framebuffer, x_pos + col, y_pos + row, r, g, b);
        }
    }

    debug_printf("BGRT bitmap drawn at (%zu, %zu).\n", x_pos, y_pos);
}

// Function to get bitmap width
int32_t get_bgrt_bitmap_width()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return -1; // Indicate an error
    }

    uint64_t image_address = get_bgrt_bitmap_address();
    if (image_address == 0)
    {
        debug_printf("BGRT image address is invalid.\n");
        return -1; // Indicate an error
    }

    auto *virtual_address = reinterpret_cast<uint8_t *>(image_address + get_higher_half_offset());
    auto *info_header = reinterpret_cast<BITMAPINFOHEADER *>(virtual_address + sizeof(BITMAPFILEHEADER));

    return info_header->biWidth;
}

// Function to get bitmap height
int32_t get_bgrt_bitmap_height()
{
    if (bgrt_table == nullptr)
    {
        debug_printf("BGRT table is not initialized.\n");
        return -1; // Indicate an error
    }

    uint64_t image_address = get_bgrt_bitmap_address();
    if (image_address == 0)
    {
        debug_printf("BGRT image address is invalid.\n");
        return -1; // Indicate an error
    }

    auto *virtual_address = reinterpret_cast<uint8_t *>(image_address + get_higher_half_offset());
    auto *info_header = reinterpret_cast<BITMAPINFOHEADER *>(virtual_address + sizeof(BITMAPFILEHEADER));

    return info_header->biHeight;
}