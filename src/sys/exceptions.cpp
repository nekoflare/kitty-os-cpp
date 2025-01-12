#include <stdexcept>

namespace std
{

exception::~exception() noexcept = default;

const char *exception::what() const noexcept
{
    return "std::exception";
}

// Implementation of std::runtime_error
runtime_error::runtime_error(const std::string &msg)
{
}

runtime_error::~runtime_error() noexcept = default;

const char *runtime_error::what() const noexcept
{
    return "??";
}

} // namespace std