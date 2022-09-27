#include <iostream>

template<typename T> struct X { const T m; };

int main(int argc, char const *argv[])
{
    const auto N = 10u; // auto is unsigned
    auto* p = &N; // auto is const unsigned
    const auto X<int>::*pm = &X<int>::m; // auto is int, what the S is?
    // X<auto> xa = X<int>(); // ERROR
    // const int auto::*pm2 = &X<int>::m; // ERROR
    return 0;
}
