#include <iostream>

template<int N, typename... Ts>
void f(double (&)[N], Ts... ps) // can not deduce N
{
}

template<typename T> void foo(T) {}
template<typename T>
void bar(T) // 1
{
    std::cout << "version 1" << std::endl;
}
template<typename T>
void bar(T*) // 2
{
    std::cout << "version 2" << std::endl;
}

int main(int argc, char const *argv[])
{
    double a[10];
    f<10>(a, 1);

    auto pfoo = &foo<int>;
    // auto pbar = &bar<int>; // ambiguous
    void (*pbar1)(int);
    pbar1 = &bar<int>; // valid, pbar1 is version 1
    pbar1(1);
    void (*pbar2)(int*);
    pbar2 = &bar<int>; // valid, pbar2 is version 2
    pbar2(nullptr);
    return 0;
}
