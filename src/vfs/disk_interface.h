#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <functional>
#include <kernel.h>
#include <string>

enum disk_type
{
    HDD,
    NVME_SSD,
    SATA_SSD,
    SSHD,
    OTHER
};

#define DISK_FLAG_READ_ONLY 0x01      // Disk is read-only
#define DISK_FLAG_BOOTABLE 0x02       // Disk is bootable
#define DISK_FLAG_HOT_PLUGGABLE 0x04  // Disk supports hot-plugging
#define DISK_FLAG_ENCRYPTED 0x08      // Disk is encrypted
#define DISK_FLAG_SUPPORTS_TRIM 0x10  // Disk supports TRIM commands
#define DISK_FLAG_LOW_POWER_MODE 0x20 // Disk is in low power mode
#define DISK_FLAG_HEALTH_WARNING 0x40 // Disk has health issues
#define DISK_FLAG_WRITE_CACHE 0x80    // Write caching is enabled
#define DISK_FLAG_SMART_ENABLED 0x100 // SMART is enabled

struct error_info
{
    int code;
    const char *description;
};

struct disk_interface
{
    const char *inf_name;             // Interface name
    const char *inf_serial;           // Serial number
    const char *inf_model;            // Model
    const char *inf_manufacturer;     // Manufacturer
    const char *inf_firmware_version; // Firmware version
    disk_type inf_disk_type;          // Disk type

    size_t inf_capacity;  // Disk capacity
    size_t inf_blk_size;  // Block size (usually 512 bytes)
    size_t inf_blk_count; // Amount of blocks on device
    size_t inf_alignment; // Alignment constraint for I/O operations

    // Core operations
    std::function<status(disk_interface *self)> initialize;
    std::function<status(disk_interface *self, char *buf, size_t offset, size_t length)> read;
    std::function<status(disk_interface *self, const char *buf, size_t offset, size_t length)> write;

    // Asynchronous operations
    std::function<status(disk_interface *self, char *buf, size_t offset, size_t length,
                         std::function<void(status)> callback)>
        async_read;

    std::function<status(disk_interface *self, const char *buf, size_t offset, size_t length,
                         std::function<void(status)> callback)>
        async_write;

    // Capabilities
    std::function<bool(disk_interface *self, uint32_t cap)> supports_capability;

    // Error handling
    std::function<status(disk_interface *self, error_info *out_error_info)> get_last_error;

    // Lifecycle management
    std::function<status(disk_interface *self)> shutdown;
    std::function<status(disk_interface *self)> reset;

    handle di_private;
};

std::string di_to_path(disk_interface *disk_inf);

/**
 * Creates a new disk interface and fills it out with default values.
 * @param inf - A pointer to the disk interface that will be overwritten
 */
void create_disk_interface(disk_interface *inf);

#endif // DISK_INTERFACE_H
