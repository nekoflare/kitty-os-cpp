#ifndef _ACPI_HPP
#define _ACPI_HPP

#include <limine.h>
#include <stddef.h>
#include <stdint.h>
#include <utility>

constexpr char MCFG_SIGNATURE[4] = {'M', 'C', 'F', 'G'};
constexpr char APIC_SIGNATURE[4] = {'A', 'P', 'I', 'C'};
constexpr char HPET_SIGNATURE[4] = {'H', 'P', 'E', 'T'};
constexpr char BGRT_SIGNATURE[4] = {'B', 'G', 'R', 'T'};

// RSDP structure
struct rsdp
{
    char signature[8]; // RSD PTR
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address; // Note: physical address
    uint32_t length;       // unused in our case.

    // since revision >= 2
    uint64_t xsdt_address; // physical address
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

// SDT structure
struct sdt
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

// RSDT structure
struct rsdt
{
    struct sdt sdt;
    // n * 4 where n are (length - sizeof(sdt)) / 4
} __attribute__((packed));

// XSDT structure
struct xsdt
{
    struct sdt sdt;
    // n * 8 where n are (length - sizeof(sdt)) / 8
} __attribute__((packed));

// APIC table header
struct apic
{
    struct sdt sdt; // Common SDT header
    uint32_t local_controller_address;
    uint32_t flags;
    // Followed by APIC entries
} __attribute__((packed));

// MCFG table header
struct mcfg
{
    struct sdt sdt; // Common SDT header
    uint64_t reserved;
} __attribute__((packed));

struct mcfg_entry
{
    uint64_t base_address;
    uint16_t segment_group;
    uint8_t start_bus;
    uint8_t end_bus;
    uint32_t reserved;
} __attribute__((packed));

// APIC entry types
enum ApicEntryType : uint8_t
{
    LOCAL_APIC = 0,
    IO_APIC = 1,
    INTERRUPT_SOURCE_OVERRIDE = 2,
    NON_MASKABLE_INTERRUPT = 4,
    LOCAL_APIC_ADDRESS_OVERRIDE = 5
};

// Generic APIC entry structure
struct apic_entry
{
    uint8_t type;
    uint8_t length;
    uint8_t data[]; // Variable-length entry
} __attribute__((packed));

// Specific APIC entry structures
struct local_apic_entry
{
    uint8_t type; // 0
    uint8_t length;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

struct io_apic_entry
{
    uint8_t type; // 1
    uint8_t length;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed));

// Interrupt Source Override (Type 2)
struct interrupt_source_override_entry
{
    uint8_t type; // 2
    uint8_t length;
    uint8_t bus;                      // Usually 0 (ISA)
    uint8_t source;                   // IRQ source
    uint32_t global_system_interrupt; // Global system interrupt
    uint16_t flags;
} __attribute__((packed));

// Non-Maskable Interrupt (Type 4)
struct non_maskable_interrupt_entry
{
    uint8_t type; // 4
    uint8_t length;
    uint8_t processor_id; // Processor ID (255 = all processors)
    uint16_t flags;
    uint8_t lint; // LINT# (0 or 1)
} __attribute__((packed));

// BGRT Status Orientation Offsets
enum class BgrtOrientation : uint8_t
{
    NoOffset = 0x0,
    Clockwise90 = 0x1,
    Clockwise180 = 0x2,
    Clockwise270 = 0x3
};

// BGRT Status Bitfield
struct bgrt_status
{
    uint8_t displayed : 1;   // Bit 0: 1 if the boot image graphic is displayed
    uint8_t orientation : 2; // Bits 1-2: Orientation offset
    uint8_t reserved : 5;    // Bits 3-7: Reserved (must be zero)
};

// BGRT Main Table
struct bgrt
{
    sdt header;              // Standard ACPI header
    uint16_t version;        // Version (must be 1)
    bgrt_status status;      // Status field
    uint8_t image_type;      // Image Type (0 = Bitmap, others reserved)
    uint64_t image_address;  // Physical address of the image bitmap
    uint32_t image_offset_x; // X-offset of the top-left corner of the image
    uint32_t image_offset_y; // Y-offset of the top-left corner of the image
} __attribute__((packed));

void initialize_acpi();
void *get_acpi_table(const char signature[4]);
void parse_apic_table();
void display_apic_entries();

void* get_rdsp_address();

// Getter functions
uint64_t get_lapic_address();

size_t get_local_apic_count();
local_apic_entry *get_local_apics();

size_t get_io_apic_count();
io_apic_entry *get_io_apics();

size_t get_iso_count();
interrupt_source_override_entry *get_isos();

size_t get_nmi_count();
non_maskable_interrupt_entry *get_nmis();

bgrt *get_bgrt_address();
uint64_t get_bgrt_bitmap_address();
std::pair<uint32_t, uint32_t> get_bgrt_image_offsets();
bgrt_status get_bgrt_status();
void display_bgrt_details();
void draw_bgrt_bitmap(limine_framebuffer *framebuffer, size_t x_pos, size_t y_pos);
int32_t get_bgrt_bitmap_width();
int32_t get_bgrt_bitmap_height();

#endif
