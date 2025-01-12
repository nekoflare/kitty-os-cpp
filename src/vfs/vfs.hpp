#pragma once

#include <string>
#include <memory>
#include <vector>

class File;
class Filesystem;

class VFS
{
    public:
        static VFS& getInstance(); // Singleton instance of VFS

        bool mount(const std::string& path, std::shared_ptr<Filesystem> fs);
        bool unmount(const std::string& path);

        // fops
        std::shared_ptr<File> open(const std::string& path, int mode);
        bool close(std::shared_ptr<File> file);
        int read(std::shared_ptr<File> file, char* buffer, size_t size);
        int write(std::shared_ptr<File> file, const char* buffer, size_t size);

        // dir ops
        bool mkdir(const std::string& path);
        bool rmdir(const std::string& path);

    private:
        VFS() = default;
        struct MountPoint {
            std::string path;
            std::shared_ptr<Filesystem> fs;
        };
        std::vector<MountPoint> mountPoints;
};