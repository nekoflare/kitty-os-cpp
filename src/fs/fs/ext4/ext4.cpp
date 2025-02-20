//
// Created by Neko on 23.01.2025.
//

#include "ext4.h"

#include <fs/filesystem.h>

filesystem ext4_structure = {.fs_name = "EXT4",

                             .is_valid_filesystem = [](disk_interface *disk_inf, size_t partition) {
                                 // ...

                                 return std::make_pair(nullptr, false);
                             }};