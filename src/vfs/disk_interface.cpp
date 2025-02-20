//
// Created by Neko on 22.01.2025.
//

#include "disk_interface.h"

void create_disk_interface(disk_interface *inf)
{
    inf->inf_name = "Default Disk Interface";
    inf->inf_serial = "0000-0000-0000-0000";
    inf->inf_model = "KittyOS Default Disk";
    inf->inf_manufacturer = "KittyOS";
    inf->inf_firmware_version = "1.0";
    inf->inf_disk_type = OTHER;

    inf->inf_capacity = 0;
    inf->inf_blk_size = 512;
    inf->inf_blk_count = 0;
    inf->inf_alignment = 512;

    inf->initialize = [](disk_interface *self) { return STATUS_FAILURE; };

    inf->read = [](disk_interface *self, char *buf, size_t offset, size_t length) { return STATUS_FAILURE; };

    inf->write = [](disk_interface *self, const char *buf, size_t offset, size_t length) { return STATUS_FAILURE; };

    inf->async_read = [](disk_interface *self, char *buf, size_t offset, size_t length,
                         const std::function<void(status)> &callback) {
        if (callback != nullptr)
            callback(STATUS_FAILURE);

        return STATUS_FAILURE;
    };

    inf->async_write = [](disk_interface *self, const char *buf, size_t offset, size_t length,
                          const std::function<void(status)> &callback) {
        if (callback != nullptr)
            callback(STATUS_FAILURE);

        return STATUS_FAILURE;
    };

    inf->supports_capability = [](disk_interface *self, uint32_t cap) { return false; };

    inf->get_last_error = [](disk_interface *self, error_info *out_error_info) {
        out_error_info->code = -1;
        out_error_info->description = "This device isn't properly set up.";

        return STATUS_SUCCESS;
    };

    inf->shutdown = [](disk_interface *self) { return STATUS_FAILURE; };

    inf->reset = [](disk_interface *self) { return STATUS_FAILURE; };
}

void uppercase(std::string &str)
{
    for (char &c : str)
    {
        c = std::toupper(c);
    }
}

std::string di_to_path(disk_interface *disk_inf)
{
    std::string inf_name = disk_inf->inf_name;
    std::replace(inf_name.begin(), inf_name.end(), " ", "_");
    uppercase(inf_name);
    return inf_name;
}
