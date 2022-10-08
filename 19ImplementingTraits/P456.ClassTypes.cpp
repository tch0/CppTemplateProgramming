#include <type_traits>

template<typename T, typename = std::void_t<>>
struct IsClass : std::false_type {};
template<typename T>
struct IsClass<T, std::void_t<int T::*>> : std::true_type {};
template<typename T>
constexpr bool IsClass_v = IsClass<T>::value;

struct X {};

int main(int argc, char const *argv[])
{
    static_assert(IsClass_v<X>);
    static_assert(!IsClass_v<int>);
    return 0;
}
