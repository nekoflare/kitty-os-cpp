//
// Created by Piotr on 02.06.2024.
//

#ifndef KITTY_OS_CPP_KEXCEPTION_HPP
#define KITTY_OS_CPP_KEXCEPTION_HPP

namespace kstd {

    class exception {
    public:
        exception(const char* msg) : message(msg) {}
        virtual ~exception() noexcept {}

        virtual const char* what() const noexcept {
            return message;
        }

    private:
        const char* message;
    };

} // namespace kstd

#endif // KITTY_OS_CPP_KEXCEPTION_HPP
