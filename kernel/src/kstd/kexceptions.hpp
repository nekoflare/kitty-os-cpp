//
// Created by Piotr on 02.06.2024.
//

#ifndef KITTYKERNEL_KEXCEPTIONS_HPP
#define KITTYKERNEL_KEXCEPTIONS_HPP

#include "kexception.hpp"

namespace kstd {

    class runtime_exception : public exception {
    public:
        runtime_exception(const char* msg) : exception(msg) {}

        ~runtime_exception() noexcept override {}

        const char* what() const noexcept override {
                return exception::what();
        }
    };

} // namespace kstd

#endif // KITTYKERNEL_KEXCEPTIONS_HPP
