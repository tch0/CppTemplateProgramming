#include <iostream>
#include <string>
#include <cstddef>
#include <concepts>
#include <functional>

template<typename T>
concept Hashable = requires(T a)
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<Hashable T>
void f(T)
{
    std::cout << "void f(T)" << std::endl;
}

template<typename T> requires Hashable<T>
void g(T)
{
    std::cout << "void g(T)" << std::endl;
}

template<typename T>
void h(T) requires Hashable<T>
{
    std::cout << "void h(T)" << std::endl;
}

using namespace std::string_literals;

struct X {};

int main(int argc, char const *argv[])
{
    f(1);
    g(2.0);
    h("hello"s);
    // f(X{});
    return 0;
}
