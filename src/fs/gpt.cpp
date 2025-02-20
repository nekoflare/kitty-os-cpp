//
// Created by Neko on 23.01.2025.
//

#include "gpt.h"
#include "utils.h"
#include <cstring>
#include <dbg/log.h>

bool is_gpt(disk_interface *disk_inf)
{
    if (!disk_inf)
        return false;

    char buffer[512];
    if (!read_sector(disk_inf, buffer, 1))
    {
        debug_print("Failed to check if the disk is MBR: Couldn't read disk %s\n", di_to_path(disk_inf).c_str());
        return false;
    }

    // GPT header starts with the ASCII string "EFI PART" (8 bytes) at the
    // beginning of sector 1
    if (std::memcmp(buffer, "EFI PART", 8) == 0)
    {
        return true;
    }
    return false;
}
