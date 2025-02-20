//
// Created by Neko on 23.01.2025.
//

#include "vdi.h"

#include <cstring>
#include <vfs/disk_interface.h>

struct vdi_identity_structure
{
    char *buffer;
    size_t buf_len;
};

status vdi_read(const disk_interface *self, char *buf, const size_t offset, const size_t length)
{
    const auto handle = self->di_private;
    const auto vdi_identity = static_cast<vdi_identity_structure *>(handle);

    if (offset + length >= vdi_identity->buf_len)
    {
        return STATUS_INVALID_ARGUMENTS;
    }

    const auto _buf = vdi_identity->buffer;

    std::memcpy(buf, _buf, length);

    return STATUS_FAILURE;
}

status vdi_write(const disk_interface *self, const char *buf, const size_t offset, const size_t length)
{
    const auto handle = self->di_private;
    const auto vdi_identity = static_cast<vdi_identity_structure *>(handle);

    if (offset + length >= vdi_identity->buf_len)
    {
        return STATUS_INVALID_ARGUMENTS;
    }

    const auto _buf = vdi_identity->buffer;

    std::memcpy(_buf, buf, length);

    return STATUS_FAILURE;
}

void create_vdi(disk_interface *disk_inf, const char *vdi_name, char *buffer, size_t buf_len)
{
    disk_interface di;
    create_disk_interface(&di);

    di.inf_name = vdi_name;
    di.read = vdi_read;
    di.write = vdi_write;

    *disk_inf = di;
}
