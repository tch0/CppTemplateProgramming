#include <iostream>
#include <type_traits>

template<typename T>
class S
{
public:
    S(T) {}
};
template<typename T> S(T) -> S<T>; // deduction guide

template<typename T>
struct X
{
    T val;
};
template<typename T> X(T) -> X<T>;

int main(int argc, char const *argv[])
{
    // using of CTAD
    S x(12); // S<int>
    static_assert(std::is_same_v<decltype(x), S<int>>);
    // S* p = &x; // ERROR: not permitted

    // aggregate class
    X y1{1};
    X y2 = {2};
    // X y3(3); // ERROR
    // X y4 = 4; // ERROR
    return 0;
}
