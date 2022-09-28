#include <iostream>

template<typename T = int>
void f(T)
{
    std::cout << "template<tyepname T> void f(T)" << std::endl;
}

// declaration of function template specialization
template<>
void f(int);

template<typename T = int>
void g(T a = 10)
{
    std::cout << "g: " << a << std::endl;
}

template<>
void g<int>(int a)
{
    std::cout << "g<int>: " << a << std::endl;
}

int main(int argc, char const *argv[])
{
    f(1);
    f(1.0);
    g(); // ouput: g<int>: 10
    return 0;
}

// declaration of function template specialization
template<>
void f(int)
{
    std::cout << "template<> void f<>(int)" << std::endl;
}
