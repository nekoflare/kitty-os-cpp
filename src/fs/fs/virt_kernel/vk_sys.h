//
// Created by Neko on 22.01.2025.
//

#ifndef VK_SYS_H
#define VK_SYS_H

#include <fs/filesystem.h>

/**
 * Returns a singleton object pointer to the filesystem for VKFS.
 * @return - Returns a singleton object pointer to the filesystem for VKFS.
 */
filesystem *create_vkfs();

#endif // VK_SYS_H
