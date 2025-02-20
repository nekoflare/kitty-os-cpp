//
// Created by Neko on 22.01.2025.
//

#include "virt_fs.h"

#include <dbg/log.h>
#include <vector>

std::vector<disk_interface> interfaces;

void handle_new_interface(const disk_interface *interface)
{
}

handle vfs_register_disk_interface(const disk_interface *interface)
{
    interfaces.push_back(*interface);

    handle_new_interface(interface);

    return reinterpret_cast<handle>(interfaces.size() - 1);
}

void vfs_print_disk_interfaces()
{
    for (const auto &inf : interfaces)
    {
        const auto name = inf.inf_name;

        debug_print("%s, ", name);
    }

    debug_print("\n");
}