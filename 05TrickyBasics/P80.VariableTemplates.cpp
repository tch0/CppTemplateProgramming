#include <iostream>

template<typename T>
class Foo
{
public:
    static int var1;
    inline static int var2; // inline so declaration is also definition, but can not initialize twice
    inline static constexpr int var3 = 2;
    template<typename U>
    static int var4;
};

// definition of static data member of class template
template<typename T>
int Foo<T>::var1 = 10;

// sepcialization of var1
template<>
int Foo<int>::var1 = 99;

// var2 can be specilized, but can not have a redefinition
template<>
int Foo<int>::var2 = -1;

// definition of static template data member of class template
template<typename T>
template<typename U>
int Foo<T>::var4 = -1;

// definition of static template data member of specialization of class template
// this do not work for Foo<int>::var4<bool>, why?
template<>
template<typename U>
int Foo<int>::var4 = -2;

// full specialization
template<>
template<>
int Foo<int>::var4<int> = -3;

class Bar
{
public:
    // declaration
    template<typename T>
    static int var;
    static const int cvar; // do not initialize, need outter definition and initialization
    static const int cvar2 = 1024; // initialize inside class, do not need a outter definition
    static constexpr int cevar = -1; // only initialize inside
    inline static int var2;
};

// definition
template<typename T>
int Bar::var = -1;

// specilization of template static data member of normal class
template<>
int Bar::var<bool> = 0;

// outter definition
const int Bar::cvar = -1;

int main(int argc, char const *argv[])
{
    std::cout << Foo<bool>::var1 << " " 
              << Foo<bool>::var2 << " "
              << Foo<bool>::var3 << std::endl;
    std::cout << Foo<int>::var1 << " " 
              << Foo<int>::var2 << " "
              << Foo<int>::var3 << std::endl;
    std::cout << Foo<bool>::var4<int> << std::endl; // -1
    std::cout << Foo<int>::var4<bool> << std::endl; // -1, why not -2?
    std::cout << Foo<int>::var4<int> << std::endl; // -3

    std::cout << Bar::var<int> << std::endl; // -10
    std::cout << Bar::var<bool> << std::endl; // 0
    std::cout << Bar::cvar << std::endl;
    std::cout << Bar::cvar2 << std::endl;
    std::cout << Bar::cevar << std::endl;
    std::cout << Bar::var2 << std::endl;
    return 0;
}
