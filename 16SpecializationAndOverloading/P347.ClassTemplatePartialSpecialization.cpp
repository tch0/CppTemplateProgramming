#include <iostream>

class Foo
{
public:
    int* pi;
    void* pv;
    double* pd;
};

class Bar
{
public:
    int* pi;
    void* pv;
    double* pd;
};

template<typename T>
class X
{
public:
    static void f()
    {
        std::cout << "X<T>::f()" << std::endl;
    }
};

// partial specialization
template<typename C>
class X<void* C::*> // #1
{
public:
    static void f()
    {
        std::cout << "X<void* C::*>::f()" << std::endl;
    }
};

template<typename T, typename C>
class X<T* C::*> // #2
{
public:
    static void f()
    {
        std::cout << "X<T* C::*>::f()" << std::endl;
    }
};

template<typename T>
class X<T* Foo::*> // #3
{
public:
    static void f()
    {
        std::cout << "X<T* Foo::*>" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    X<int>::f(); // generic version
    X<decltype(&Bar::pv)>::f(); // #1
    X<decltype(&Bar::pd)>::f(); // #2
    X<decltype(&Foo::pi)>::f(); // #2
    // X<decltype(&Foo::pv)>::f(); // ambiguous
    return 0;
}
