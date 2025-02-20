//
// Created by Neko on 23.01.2025.
//

#include "utils.h"

bool read_sector(disk_interface *disk_inf, char *buffer, const size_t sector_number)
{
    const size_t offset = sector_number * disk_inf->inf_blk_size;
    return disk_inf->read(disk_inf, buffer, offset, disk_inf->inf_blk_size) == STATUS_SUCCESS;
}