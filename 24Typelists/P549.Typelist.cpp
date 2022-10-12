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

int main(int argc, char const *argv[])
{
    
    return 0;
}
