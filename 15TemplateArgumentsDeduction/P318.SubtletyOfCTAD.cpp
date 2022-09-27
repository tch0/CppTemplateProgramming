#include <iostream>
#include <type_traits>

template<typename T, typename U>
struct X {
    X(const T&) // #1
    {
        std::cout << "X(const T&)" << std::endl;
    }
    X(T&&) // #2
    {
        std::cout << "X(T&&)" << std::endl;
    }
};
template<typename T> X(const T&) -> X<T, T&>;
template<typename T> explicit X(T&&) -> X<T, T>;

int main(int argc, char const *argv[])
{
    X x1 = 1; // call #2
    static_assert(std::is_same_v<decltype(x1), X<int, int&>>);
    X x2{2}; // call #2
    static_assert(std::is_same_v<decltype(x2), X<int, int>>);
    return 0;
}
