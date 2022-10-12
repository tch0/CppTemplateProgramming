#include <type_traits>

struct Nil {};

template<typename H, typename T>
struct Cons
{
    using Head = H;
    using Tail = T;
};

template<typename T>
struct CarImpl
{
    using type = typename T::Head;
};

template<typename T>
using Car = typename CarImpl<T>::type;

template<typename T>
struct CdrImpl
{
    using type = typename T::Tail;
};

template<typename T>
using Cdr = typename CdrImpl<T>::type;

int main(int argc, char const *argv[])
{
    static_assert(std::is_same_v<Car<Cons<int, Cons<double, Nil>>>, int>);
    static_assert(std::is_same_v<Cdr<Cons<int, Cons<double, Nil>>>, Cons<double, Nil>>);
    return 0;
}
