#include <iostream>

template<typename T>
int f(T) // #1
{
    return 1;
}
template<typename T>
int f(T*) // #2
{
    return 2;
}

int main(int argc, char const *argv[])
{
    std::cout << f(0) << std::endl; // #1
    std::cout << f(nullptr) << std::endl; // #1
    std::cout << f((int*)nullptr) << std::endl; // #2
    return 0;
}
