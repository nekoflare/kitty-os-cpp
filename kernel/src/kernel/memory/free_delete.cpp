//
// Created by Piotr on 30.05.2024.
//

#include "heap.hpp"

// Overload new operator
void* operator new(size_t size) {
    return kmalloc(size);
}

// Overload delete operator
void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

// Overload new[] operator
void* operator new[](size_t size) {
    return kmalloc(size);
}

// Overload delete[] operator
void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

// Overload delete[] operator
void operator delete[](void* ptr, std::size_t size) noexcept {
    kfree(ptr);
}

// Overload delete operator
void operator delete(void* ptr, std::size_t size) noexcept {
    kfree(ptr);
}