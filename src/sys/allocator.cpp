#include <memory>

namespace std
{

template struct allocator<char>;
template struct allocator<short>;
template struct allocator<int>;
template struct allocator<long>;
template struct allocator<long long>;
template struct allocator<float>;
template struct allocator<double>;

} // namespace std
