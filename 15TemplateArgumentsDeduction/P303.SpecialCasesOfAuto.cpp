#include <iostream>
#include <initializer_list>

template<typename T>
void print(T t)
{
    for (decltype(auto) val : t)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

auto g()
{
    // return {1, 2, 3}; // ERROR in syntax
    return std::initializer_list<int>{1, 2, 3}; // warning, valid in syntax, but still problematic logic.
}

// case 2:
// error: inconsistent deduction for auto return type: 'double' and then 'int'
// auto f(bool b)
// {
//     if (b) {
//         return 42.0;
//     }
//     else {
//         return 1;
//     }
// }

// case 3:
template<typename T, typename U>
auto addA(T t, U u) -> decltype(t+u)
{
    return t + u;
}
void addA(...);

template<typename T, typename U>
auto addB(T t, U u) -> decltype(auto)
{
    return t + u;
}
void addB(...);

struct X {};
using AddResA = decltype(addA(X(), X()));
// using AddResB = decltype(addB(X(), X())); // ERROR, deducation of type of t + u is not in immediate context, no SFINAE，

int main(int argc, char const *argv[])
{
    // case 1:
    // print({1, 2, 3}); // ERROR
    auto vals = {1, 2, 3};
    print(vals);

    // auto a {1, 2, 3}; // ERROR
    auto b {1};
    return 0;
}
