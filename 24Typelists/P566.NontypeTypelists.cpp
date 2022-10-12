#include <type_traits>

template<typename... Elements>
struct Typelist {};

// extract first element
template<typename List>
struct Front;

template<typename Head, typename... Tail>
struct Front<Typelist<Head, Tail...>>
{
    using type = Head;
};

template<typename T>
using Front_t = typename Front<T>::type;

// pop first element, get the rest typelist
template<typename List>
struct Back;

template<typename Head, typename... Tail>
struct Back<Typelist<Head, Tail...>>
{
    using type = Typelist<Tail...>;
};

template<typename T>
using Back_t = typename Back<T>::type;

// push element to typelist
template<typename NewElement, typename List>
struct Cons;

template<typename NewElement, typename... Elements>
struct Cons<NewElement, Typelist<Elements...>>
{
    using type = Typelist<NewElement, Elements...>;
};

template<typename NewElement, typename List>
using Cons_t = typename Cons<NewElement, List>::type;

// transformation
template<typename List, template<typename> class MetaFunc, bool = false> // MetaFunc::type is the result of transformation
struct Transform;

template<typename... Elements, template<typename> class MetaFunc> 
struct Transform<Typelist<Elements...>, MetaFunc, false>
{
    using type = Typelist<typename MetaFunc<Elements>::type...>; // using pack expansion
};

template<template<typename> class MetaFunc>
struct Transform<Typelist<>, MetaFunc, true>
{
    using type = Typelist<>;
};

template<typename List, template<typename> class MetaFunc>
using Transform_t = typename Transform<List, MetaFunc>::type;


// value
template<typename T, T Value>
struct CTValue
{
    static constexpr T value = Value;
};

// multiplication of nontype values
template<typename T, typename U>
struct Multiply;

template<typename T, T Value1, T Value2>
struct Multiply<CTValue<T, Value1>, CTValue<T, Value2>>
{
    using type = CTValue<T, Value1 * Value2>;
};

template<typename T, typename U>
using Multiply_t = typename Multiply<T, U>::type;

// nontype typelist
template<typename T, T... Values>
using CTTypelist = Typelist<CTValue<T, Values>...>;

template<typename T>
struct Add1;
template<typename T, T Value>
struct Add1<CTValue<T, Value>>
{
    using type = CTValue<T, Value+1>;
};

int main(int argc, char const *argv[])
{
    // Multiply
    static_assert(std::is_same_v<Multiply_t<CTValue<int, 2>, CTValue<int, 3>>, CTValue<int, 6>>);
    // transform of CTTypelist
    static_assert(std::is_same_v<Transform_t<CTTypelist<int, 2, 3>, Add1>, CTTypelist<int, 3, 4>>);
    return 0;
}
