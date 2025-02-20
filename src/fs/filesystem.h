//
// Created by Neko on 22.01.2025.
//

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <vfs/disk_interface.h>

using file_user = uint64_t;

struct file_permission
{
    uint64_t read : 1;
    uint64_t write : 1;
    uint64_t execute : 1;
    uint64_t _delete : 1;
    uint64_t edit_metadata : 1;
};

struct file_metadata
{
    size_t size;
    std::vector<std::pair<file_user, file_permission>> permissions;
    uint64_t creation_date;
    uint64_t modification_date;
    uint64_t access_date;
};

struct filesystem
{
    const char *fs_name; // A name for the filesystem, e.g: FAT, NTFS, APFS, ...

    // Checks if the given disk interface could be associated with the filesystem.
    std::function<std::pair<handle, bool>(disk_interface *disk_inf, size_t partition)> is_valid_filesystem;

    std::function<handle(handle fs, const char *path)> open_file;
    std::function<void(handle fs, handle file)> close_file;
    std::function<size_t(handle fs, handle file, void *buffer, size_t size, size_t offset)> read;
    std::function<size_t(handle fs, handle file, const void *buffer, size_t size, size_t offset)> write;
    std::function<bool(handle fs, handle file, file_metadata *metadata)> get_file_metadata;
    std::function<bool(handle fs, const char *path, file_metadata metadata)> create_file;
    std::function<bool(handle fs, const char *path)> delete_file;
    std::function<std::vector<std::string>(handle fs, const char *dir_path)> list_directory;
};

/**
 * Lets kernel use the filesystem.
 * @param fs - The pointer to the filesystem.
 * @return - Gives status if registering was sucessful.
 */
status register_filesystem(filesystem *fs);

/**
 * Prints out (to the terminal or other device) available filesystems.
 */
void list_available_filesystems();

/**
 *
 * @param disk_inf - Disk interface for the filesystem to check.
 * @return - nullptr or pointer to filesystem on found filesystem that can be
 * associated with the disk device.
 */
filesystem *find_filesystem(disk_interface *disk_inf);

#endif // FILESYSTEM_H
