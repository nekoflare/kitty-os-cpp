//
// Created by Piotr on 04.06.2024.
//

#ifndef KITTY_OS_CPP_RAMVFS_HPP
#define KITTY_OS_CPP_RAMVFS_HPP

#include <kstd/kstdio.hpp>
#include <kstd/kstring.hpp>
#include <kernel/memory/pmm.hpp>

struct ramvfs_entry_object
{
    const char* vfs_entry_obj_name;
    bool is_file;
    bool is_directory;
    bool is_memory_link;
    bool perm_readable;
    bool perm_writable;
    ramvfs_entry_object* top;
    ramvfs_entry_object* next;
    const char* content;
    size_t content_size;
};

struct ramvfs_object
{
    const char* vfs_obj_name;
    size_t file_count;
    size_t dir_count;
    size_t link_count;
    size_t used_memory;
    ramvfs_entry_object* next;

    const char* curr_dir;
    ramvfs_entry_object* curr_dir_obj;
};

void ramvfs_allocate_object(ramvfs_object** obj, const char* vfs_object_name);
void ramvfs_print_object_information(ramvfs_object* obj);

bool ramvfs_create_file(ramvfs_object* obj, const char* file_name, const char* contents, size_t content_size);
bool ramvfs_create_directory(ramvfs_object* obj, const char* dir_name);

bool ramvfs_directory_exists(ramvfs_object* obj, const char* dir_name);
bool ramvfs_file_exists(ramvfs_object* obj, const char* file_name);

bool ramvfs_change_directory(ramvfs_object* obj, const char* directory);

#endif //KITTY_OS_CPP_RAMVFS_HPP
