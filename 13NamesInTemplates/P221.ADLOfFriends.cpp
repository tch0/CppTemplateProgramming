#include <iostream>

template<typename T>
class C
{
    friend void f()
    {
        std::cout << "void f()" << std::endl;
    }
    friend void f(const C<T>*)
    {
        std::cout << "void f(const C<T>*)" << std::endl;
    }
};

void g(C<int>* p)
{
    // f(); // f was not declared
    f(p); // valid
}

int main(int argc, char const *argv[])
{
    C<int>* p = nullptr;
    g(p);
    return 0;
}
