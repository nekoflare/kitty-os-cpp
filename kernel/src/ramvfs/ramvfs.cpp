//
// Created by Piotr on 04.06.2024.
//

#include "ramvfs.hpp"

template <typename T>
static void ramvfs_handle_allocation_fail(T** obj)
{
    if (static_cast<void*>(obj) == nullptr)
    {
        kstd::printf("[RamVFS] Failed to create new object. Allocation fail.\n");
        return;
    }
}

void ramvfs_allocate_object(ramvfs_object** obj, const char* vfs_object_name)
{
    auto newObj = new ramvfs_object;
    ramvfs_handle_allocation_fail(&newObj);

    auto von = kstd::strlen(vfs_object_name) + 1;

    char* vfs_obj_name = new char[von];
    ramvfs_handle_allocation_fail(&vfs_obj_name);

    kstd::memset(vfs_obj_name, 0, von);
    kstd::strcpy(vfs_obj_name, vfs_object_name);

    newObj->vfs_obj_name = vfs_obj_name;
    newObj->file_count = 0;
    newObj->dir_count = 0;
    newObj->link_count = 0;
    newObj->used_memory = sizeof(ramvfs_object) + von + 1;
    newObj->next = nullptr;
    newObj->curr_dir = "";
    newObj->curr_dir_obj = nullptr;

    *obj = newObj;
}

void ramvfs_print_object_information(ramvfs_object* obj)
{
    kstd::printf("[ start ]\n");
    kstd::printf("VFS name: %s\n", obj->vfs_obj_name);
    kstd::printf("Files: %zu\n", obj->file_count);
    kstd::printf("Directories: %zu\n", obj->dir_count);
    kstd::printf("Links: %zu\n", obj->link_count);
    mem_size size = pmm_calculate_effective_size(obj->used_memory);
    kstd::printf("Used memory: %f (%sB)\n", size.size, size.prefix);
    kstd::printf("Current directory: \"%s\"\n", obj->curr_dir);
    kstd::printf("[ end ]\n");
}

bool ramvfs_create_file(ramvfs_object* obj, const char* file_name, const char* contents, size_t content_size)
{
    if (ramvfs_file_exists(obj, file_name))
    {
        return false; // File already exists
    }

    size_t von = kstd::strlen(file_name);
    char* fname = new char[von + 1];
    kstd::memset(fname, 0, von + 1);
    kstd::strcpy(fname, file_name);

    // Traverse to the end of the linked list
    ramvfs_entry_object* current = obj->next;
    ramvfs_entry_object* prev = nullptr;

    while (current != nullptr)
    {
        prev = current;
        current = current->next;
    }

    // Create a new entry object
    ramvfs_entry_object* new_entry = new ramvfs_entry_object;
    new_entry->vfs_entry_obj_name = fname;
    new_entry->is_directory = false;
    new_entry->is_memory_link = false;
    new_entry->is_file = true;
    new_entry->perm_readable = true;
    new_entry->perm_writable = true;
    new_entry->top = nullptr;
    new_entry->next = nullptr;
    new_entry->content = contents;
    new_entry->content_size = content_size;

    if (prev == nullptr)
    {
        // If prev is still nullptr, this means the list was initially empty
        obj->next = new_entry;
    }
    else
    {
        // Otherwise, link the new entry at the end of the list
        prev->next = new_entry;
    }

    obj->used_memory += sizeof(ramvfs_entry_object);
    obj->used_memory += content_size;
    obj->used_memory += von + 1;

    obj->file_count++;

    return true;
}

bool ramvfs_directory_exists(ramvfs_object* obj, const char* dir_name)
{
    ramvfs_entry_object* current = obj->next;

    while (current != nullptr)
    {
        if (current->is_directory && kstd::strcmp(current->vfs_entry_obj_name, dir_name) == 0)
        {
            return true;
        }
        current = current->next;
    }

    return false;
}

bool ramvfs_file_exists(ramvfs_object* obj, const char* file_name)
{
    ramvfs_entry_object* current = obj->next;

    while (current != nullptr)
    {
        if (current->is_file && kstd::strcmp(current->vfs_entry_obj_name, file_name) == 0)
        {
            return true;
        }
        current = current->next;
    }

    return false;
}

bool ramvfs_create_directory(ramvfs_object* obj, const char* dir_name)
{
    if (ramvfs_directory_exists(obj, dir_name))
    {
        return false; // Directory already exists
    }

    size_t von = kstd::strlen(dir_name);
    char* dname = new char[von + 1];
    kstd::memset(dname, 0, von + 1);
    kstd::strcpy(dname, dir_name);

    // Traverse to the end of the linked list
    ramvfs_entry_object* current = obj->next;
    ramvfs_entry_object* prev = nullptr;

    while (current != nullptr)
    {
        prev = current;
        current = current->next;
    }

    // Create a new entry object
    ramvfs_entry_object* new_entry = new ramvfs_entry_object;
    new_entry->vfs_entry_obj_name = dname;
    new_entry->is_directory = true;
    new_entry->is_memory_link = false;
    new_entry->is_file = false;
    new_entry->perm_readable = true;
    new_entry->perm_writable = true;
    new_entry->top = nullptr;
    new_entry->next = nullptr;
    new_entry->content = nullptr;
    new_entry->content_size = 0;

    if (prev == nullptr)
    {
        // If prev is still nullptr, this means the list was initially empty
        obj->next = new_entry;
    }
    else
    {
        // Otherwise, link the new entry at the end of the list
        prev->next = new_entry;
    }

    obj->used_memory += sizeof(ramvfs_entry_object);
    obj->used_memory += von + 1;

    obj->dir_count++;

    return true;
}


bool ramvfs_change_directory(ramvfs_object* obj, const char* directory)
{
    ramvfs_entry_object* current = obj->next;

    // Traverse the linked list to find the directory
    while (current != nullptr)
    {
        if (current->is_directory && kstd::strcmp(current->vfs_entry_obj_name, directory) == 0)
        {
            // Update the current directory
            obj->curr_dir = current->vfs_entry_obj_name;
            obj->curr_dir_obj = current;
            return true;
        }
        current = current->next;
    }

    // Directory not found
    return false;
}