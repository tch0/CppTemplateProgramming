#include <iostream>

class A {};
class B : public A {};
class C : public B {};
class D : public A, public B {};

int main(int argc, char const *argv[])
{
    std::cout << sizeof(A) << std::endl; // 1
    std::cout << sizeof(B) << std::endl; // 1
    std::cout << sizeof(C) << std::endl; // 1
    std::cout << sizeof(D) << std::endl; // 2
    return 0;
}
