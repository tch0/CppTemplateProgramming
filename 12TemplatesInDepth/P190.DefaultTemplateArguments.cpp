#include <iostream>

template<typename T1, typename T2, typename T3 = char> struct Foo;
template<typename T1, typename T2 = char, typename T3> struct Foo; // valid: T3 already has a default template argument
// template<typename T1, typename T2, typename T3 = char> struct Foo; // ERROR: default template argument cannot be repeated

int main(int argc, char const *argv[])
{
    
    return 0;
}
