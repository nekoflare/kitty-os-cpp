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
    int strcmp(const char* str1, const char* str2);
    char* strdup(const char* str);
    char tolower(char c);
    char toupper(char c);
    bool isalpha(char c);

    class string
    {
    private:
        char* data;
        size_t length;
        size_t capacity; // Add capacity to manage memory

    public:
        // Iterator class definition
        class iterator {
        private:
            char* ptr;

        public:
            iterator(char* p) : ptr(p) {}

            char& operator*() const {
                return *ptr;
            }

            iterator& operator++() {
                ++ptr;
                return *this;
            }

            iterator operator++(int) {
                iterator temp = *this;
                ++ptr;
                return temp;
            }

            bool operator==(const iterator& other) const {
                return ptr == other.ptr;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };

        // Constructors and destructor
        string() : data(nullptr), length(0), capacity(0) {}

        string(const char* str) : data(nullptr), length(0), capacity(0) {
            size_t len = strlen(str);
            reserve(len + 1); // +1 for null terminator
            strcpy(data, str);
            length = len;
        }

        ~string() {
            if (data) {
                delete[] data;
            }
        }

        string(const string& other) : data(nullptr), length(0), capacity(0) {
            reserve(other.length + 1); // +1 for null terminator
            strcpy(data, other.data);
            length = other.length;
        }

        // Assignment operator
        string& operator=(const string& other) {
            if (this != &other) {
                reserve(other.length + 1); // +1 for null terminator
                strcpy(data, other.data);
                length = other.length;
            }
            return *this;
        }

        // Capacity management
        void reserve(size_t newCapacity) {
            if (newCapacity > capacity) {
                char* newData = new char[newCapacity];
                if (data) {
                    strcpy(newData, data);
                    delete[] data;
                }
                data = newData;
                capacity = newCapacity;
            }
        }

        // Size and empty check
        size_t size() const {
            return length;
        }

        bool empty() const {
            return length == 0;
        }

        // Access to C-string
        const char* c_str() const {
            return data;
        }

        // Concatenation operators
        string& operator+=(const string& other) {
            size_t newLength = length + other.length;
            reserve(newLength + 1); // +1 for null terminator
            strcat(data, other.data);
            length = newLength;
            return *this;
        }

        string& operator+=(const char other) {
            size_t newLength = length + 1;
            reserve(newLength + 1); // +1 for null terminator
            data[length] = other;
            data[length + 1] = '\0'; // Null-terminate the string
            length = newLength;
            return *this;
        }

        // Clear the string
        void clear() {
            if (data) {
                delete[] data;
                data = nullptr;
            }
            length = 0;
            capacity = 0;
        }

        // Iterator methods
        iterator begin() {
            return iterator(data);
        }

        iterator end() {
            return iterator(data + length);
        }

        // Const iterator methods
        iterator begin() const {
            return iterator(data);
        }

        iterator end() const {
            return iterator(data + length);
        }
    };
}

#endif //KITTY_OS_CPP_KSTRING_HPP