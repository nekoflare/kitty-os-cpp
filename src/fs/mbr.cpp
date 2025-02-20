//
// Created by Neko on 23.01.2025.
//

#include "mbr.h"

#include <dbg/log.h>

#include "utils.h"

bool is_mbr(disk_interface *disk_inf)
{
    if (!disk_inf)
        return false;

    char buffer[512];
    if (!read_sector(disk_inf, buffer, 0))
    {
        debug_print("Failed to check if the disk is MBR: Couldn't read disk %s\n", di_to_path(disk_inf).c_str());
        return false;
    }

    // MBR signature is at offset 510-511 and must equal 0x55AA
    if (buffer[510] == static_cast<char>(0x55) && buffer[511] == static_cast<char>(0xAA))
    {
        return true;
    }
    return false;
}
