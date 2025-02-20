//
// Created by Neko on 23.01.2025.
//

#ifndef VDI_H
#define VDI_H
#include <vfs/disk_interface.h>

void create_vdi(disk_interface *disk_inf, const char *vdi_name, char *buffer, size_t buf_len);

#endif // VDI_H
