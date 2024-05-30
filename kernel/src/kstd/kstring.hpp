//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_KSTRING_HPP
#define KITTY_OS_CPP_KSTRING_HPP

#include <stdint.h>
#include <sys/types.h>

namespace kstd
{
    size_t strlen(const char* s);
    void* memset(void* ptr, int v, size_t num);
    void* memcpy(void* dest, const void* src, size_t num);
    void* memmove(void* dest, const void* src, size_t num);
    int memcmp(const void* ptr1, const void* ptr2, size_t num);
    void* memchr(const void* ptr, int value, size_t num);
    void* memrchr(const void* ptr, int value, size_t num);
    void* memccpy(void* dest, const void* src, int c, size_t num);
    void memswap(void* ptr1, void* ptr2, size_t num);
    char* strcpy(char* destination, const char* source);
    char* strcat(char* destination, const char* source);

    class string
    {
    private:
        char* data;
        size_t length;

    public:
        string() : data(nullptr), length(0) {}
        string(const char* str);
        ~string();

        string (const string& other);

        string& operator=(const string& other);

        size_t size() const;
        const char* c_str() const;

        string& operator+=(const string& other);
    };
}

#endif //KITTY_OS_CPP_KSTRING_HPP