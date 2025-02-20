//
// Created by Neko on 22.01.2025.
//

#ifndef VIRT_FS_H
#define VIRT_FS_H
#include "disk_interface.h"

/**
 * Same with the param info. vvvvv
 * @param interface - Source interface to install and load and begin to identify
 * filesystems.
 */
handle vfs_register_disk_interface(const disk_interface *interface);

/**
 * Prints available disk interfaces to debug con
 */
void vfs_print_disk_interfaces();

#endif // VIRT_FS_H
