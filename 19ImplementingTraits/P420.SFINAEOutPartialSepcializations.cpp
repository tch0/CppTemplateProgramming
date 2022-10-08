#include <iostream>
#include <type_traits>

template<typename...>
using VoidT = void;

// primary template
template<typename, typename=VoidT<>>
struct IsDefaultConstructible : std::false_type {};

// partial sepcilization for type that can be default constructed
template<typename T>
struct IsDefaultConstructible<T, VoidT<decltype(T())>> : std::true_type {};

template<typename T>
constexpr bool IsDefaultConstructible_v = IsDefaultConstructible<T>::value;

struct X {};
struct Y 
{
    Y(int) {};
};



int main(int argc, char const *argv[])
{
    static_assert(IsDefaultConstructible_v<X>);
    static_assert(!IsDefaultConstructible_v<Y>);
    // static_assert(IsDefaultConstructible2_v<X>);
    // static_assert(!IsDefaultConstructible2_v<Y>);
    return 0;
}
