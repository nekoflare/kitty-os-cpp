//
// Created by Neko on 23.01.2025.
//

#ifndef UTILS_H
#define UTILS_H
#include <vfs/disk_interface.h>

bool read_sector(disk_interface *disk_inf, char *buffer, size_t sector_number);

#endif // UTILS_H
