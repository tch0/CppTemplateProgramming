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

// 1. Indexing
template<typename List, unsigned N>
struct GetNth : public GetNth<Back_t<List>, N-1> {};

template<typename List>
struct GetNth<List, 0> : public Front<List> {};

template<typename List, unsigned N>
using GetNth_t = typename GetNth<List, N>::type;

// 2. Finding the best match
template<typename List>
struct LargestType
{
private:
    using First = Front_t<List>;
    using Rest = typename LargestType<Back_t<List>>::type;
public:
    using type = std::conditional_t<(sizeof(First) > sizeof(Rest)), First, Rest>;
};

template<>
struct LargestType<Typelist<>>
{
    using type = char; // the smallest type
};

template<typename List>
using LargestType_t = typename LargestType<List>::type;

// 3. Push element to back of typelist
template<typename List, typename NewElement>
struct Append;

template<typename... Elements, typename NewElement>
struct Append<Typelist<Elements...>, NewElement>
{
    using type = Typelist<Elements..., NewElement>;
};

template<typename List, typename NewElement>
using Append_t = typename Append<List, NewElement>::type;

// 4. Reversing a typelist
template<typename List>
struct Reverse
{
    using type = Append_t<typename Reverse<Back_t<List>>::type, Front_t<List>>;
};

template<>
struct Reverse<Typelist<>>
{
    using type = Typelist<>;
};

template<typename List>
using Reverse_t = typename Reverse<List>::type;

// pop back
template<typename List>
using PopBack_t = Reverse_t<Back_t<Reverse_t<List>>>;

// 5. Transforming a typelist
template<typename List, template<typename> class MetaFunc> // MetaFunc::type is the result of transformation
struct Transform
{
    using type = Cons_t<typename MetaFunc<Front_t<List>>::type, typename Transform<Back_t<List>, MetaFunc>::type>;
};

template<template<typename> class MetaFunc>
struct Transform<Typelist<>, MetaFunc>
{
    using type = Typelist<>;
};

template<typename List, template<typename> class MetaFunc>
using Transform_t = typename Transform<List, MetaFunc>::type;

// 6. Accumulating typelists
template<typename List,
         template<typename, typename> class F,
         typename Init>
struct Accumulate : public Accumulate<Back_t<List>, F, typename F<Init, Front_t<List>>::type> {};

template<template<typename, typename> class F, typename Init>
struct Accumulate<Typelist<>, F, Init>
{
    using type = Init;
};

template<typename List,
         template<typename, typename> class F,
         typename Init>
using Accumulate_t = typename Accumulate<List, F, Init>::type;

// using Accumulate implement LargestType
template<typename X, typename Y>
struct GE : public std::conditional<(sizeof(X) > sizeof(Y)), X, Y> {};

template<typename List>
using LargestType2_t = Accumulate_t<List, GE, char>;

int main(int argc, char const *argv[])
{
    // Front
    static_assert(std::is_same_v<Front_t<Typelist<int, double, char, unsigned>>, int>);
    // Back
    static_assert(std::is_same_v<Back_t<Typelist<int, double, char, unsigned>>, Typelist<double, char, unsigned>>);
    // Cons
    static_assert(std::is_same_v<Cons_t<bool, Typelist<int, double, char, unsigned>>, Typelist<bool, int, double, char, unsigned>>);
    // GetNth
    static_assert(std::is_same_v<GetNth_t<Typelist<int, double, char, unsigned>, 2>, char>);
    // LargestType
    static_assert(std::is_same_v<LargestType_t<Typelist<int, double, char, unsigned>>, double>);
    // Append
    static_assert(std::is_same_v<Append_t<Typelist<int, double, char, unsigned>, bool>, Typelist<int, double, char, unsigned, bool>>);
    // Reverse
    static_assert(std::is_same_v<Reverse_t<Typelist<int, double, char, unsigned>>, Typelist<unsigned, char, double, int>>);
    // PopBack_t
    static_assert(std::is_same_v<PopBack_t<Typelist<int, double, char, unsigned>>, Typelist<int, double, char>>);
    // Transform
    static_assert(std::is_same_v<Transform_t<Typelist<int, double, char, unsigned>, std::add_const>, Typelist<const int, const double, const char, const unsigned>>);
    // Accumulate
    static_assert(std::is_same_v<LargestType2_t<Typelist<int, double, char, unsigned>>, double>);
    return 0;
}
