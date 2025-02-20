//
// Created by Neko on 22.01.2025.
//

#include "filesystem.h"
#include <vector>

#include <cstring>
#include <dbg/log.h>

std::vector<filesystem *> filesystems;

void list_available_filesystems()
{
    debug_print("Available filesystems supported by the kernel: ");
    for (const auto &fs : filesystems)
    {
        debug_print("%s, ", fs->fs_name);
    }
    debug_print("\n");
}

status register_filesystem(filesystem *fs)
{
    for (const auto &this_fs : filesystems)
    {
        if (strcmp(fs->fs_name, this_fs->fs_name) != 0)
        {
            return STATUS_FAILURE;
        }
    }

    filesystems.push_back(fs);

    return STATUS_SUCCESS;
}