#include "vk_sys.h"
#include <limine.h>

#include <cstring>
#include <dbg/log.h>
#include <functional>
#include <vector>

limine_module_request module_request = {.id = LIMINE_MODULE_REQUEST, .revision = 0, .response = nullptr};

struct open_file
{
    const char *path;
};

static std::vector<open_file> open_files;

static uint64_t last_uid = 1;

static filesystem vkfs_structure = {
    .fs_name = "VKFS0",

    .is_valid_filesystem =
        [](disk_interface *, size_t) {
            // We don't use disk interface, so we cannot associate it with any
            // medium of storage.

            return std::make_pair(nullptr,
                                  false); // False prevents associating with any disk interface.
        },

    .open_file = [](handle, const char *path) -> open_file * {
        for (const auto &[path] : open_files)
        {
            if (strcmp(path, path) == 0)
            {
                return nullptr;
            }
        }

        const auto module_count = module_request.response->module_count;
        const auto modules = module_request.response->modules;
        for (uint64_t i = 0; i < module_count; i++)
        {
            if (const auto module = modules[i]; strcmp(path, module->path) == 0)
            {
                open_files.push_back({path});
                return &open_files.back();
            }
        }

        return nullptr;
    },

    .close_file =
        [](handle, const handle file) {
            const auto file_to_close = static_cast<open_file *>(file);
            for (auto it = open_files.begin(); it != open_files.end(); ++it)
            {
                if (&*it == file_to_close)
                {
                    open_files.erase(it);
                    break;
                }
            }
        },

    .read = [](handle, const handle file, void *buffer, size_t size, const size_t offset) -> size_t {
        open_file f = *static_cast<open_file *>(file);
        const char *file_data = nullptr;

        const auto module_count = module_request.response->module_count;
        const auto modules = module_request.response->modules;
        uint64_t file_length = 0;
        for (uint64_t i = 0; i < module_count; i++)
        {
            if (const auto module = modules[i]; strcmp(f.path, module->path) == 0)
            {
                file_data = static_cast<const char *>(module->address);
                file_length = module->size;
            }
        }

        if (offset >= file_length)
        {
            return 0;
        }

        if (size + offset >= file_length)
        {
            if (size > file_length)
            {
                size = file_length;
            }
            else
            {
                size -= offset;
            }
        }

        const size_t bytes_to_read = (size + offset <= file_length) ? size : (file_length - offset);

        memcpy(buffer, file_data + offset, bytes_to_read);
        return bytes_to_read;
    }};

filesystem *create_vkfs()
{
    if (module_request.response == nullptr)
    {
        debug_print("Error: module_request.response is null\n");
        return nullptr;
    }

    debug_print("File count: %ld\n", module_request.response->module_count);

    if (module_request.response->modules == nullptr)
    {
        debug_print("Error: module_request.response->modules is null\n");
        return nullptr;
    }

    debug_print("Files on VKFS: \n");

    for (size_t i = 0; i < module_request.response->module_count; i++)
    {
        const auto entries = module_request.response->modules;

        if (entries[i] == nullptr)
        {
            debug_print("Error: entry at index %zu is null\n", i);
            continue;
        }

        const auto entry = entries[i];
        const auto path = entry->path;

        if (path == nullptr)
        {
            debug_print("Error: path for entry at index %zu is null\n", i);
            continue;
        }

        if (i < module_request.response->module_count - 1)
            debug_print("%s, ", path);
        else
            debug_print("%s", path);
    }

    debug_print("\n");

    return &vkfs_structure;
}
