#include <iostream>

template<typename T>
int f(T) // #1
{
    std::cout << "template<typename T> int f(T)" << std::endl;
    return 0;
}

template<typename T>
int f(T*) // #2
{
    std::cout << "template<typename T> int f(T*)" << std::endl;
    return 0;
}

int main(int argc, char const *argv[])
{
    int a;
    f<int>(1); // #1
    f<int*>(&a); // #1
    f<int>(&a); // #2
    return 0;
}
