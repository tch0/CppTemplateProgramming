#include <iostream>

// template declearations in namespace scope
// class template
template<typename T1, typename T2>
class Foo {};

// function template
template<typename T>
void foo()
{
    std::cout << "template<typename T> void foo()" << std::endl;
}

// variable template
template<typename T>
int bar = 1;

// alias template
template<typename T>
using FooInt = Foo<int, T>;

// template declearations in class scope
class Buz
{
public:
    // nested class template
    template<typename T1, typename T2>
    class Foo {};

    // memeber function template
    template<typename T>
    void foo()
    {
        std::cout << "template<typename T> void Buz::foo()" << std::endl;
    }

    // static member variable template
    template<typename T>
    inline static int bar = 1;

    // member alias template
    template<typename T>
    using FooInt = Foo<int, T>;
};

int main(int argc, char const *argv[])
{
    Foo<int, double> f1;
    foo<int>();
    std::cout << bar<int> << std::endl;
    FooInt<double> f2;

    Buz buz;
    Buz::Foo<int, double> bf1;
    buz.foo<double>();
    std::cout << Buz::bar<double> << std::endl;
    Buz::FooInt<double> bf2;
    return 0;
}
