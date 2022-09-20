#include <iostream>

template<typename T, T Root, template<T> class Buf>
void foo()
{
}

template<int>
class Buffer
{
};

template<typename... Args, typename Last>
void foo(Last value);

template<typename... Args> class Bar;
template<typename... Args1, typename... Args2>
auto buz(Bar<Args1...>, Bar<Args2...>);

template<typename... Args> class TypeList;
template<typename X, typename Y> class Zip;
template<typename... Xs, typename... Ys, typename T>
class Zip<TypeList<Xs...>, TypeList<Ys..., T>>;

// template<typename... Ts, Ts... vals> class WhatEver; // ERROR

template<typename... Ts>
struct Outter
{
    template<Ts... Values> struct Inner {};
};

int main(int argc, char const *argv[])
{
    foo<int, 10, Buffer>();
    return 0;
}
