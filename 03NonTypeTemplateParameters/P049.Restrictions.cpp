#include <iostream>

void f()
{
    std::cout << "void f()" << std::endl;
}

// function pointer as template parameter
template<void(*Func)()>
void f1()
{
    Func();
}

class Foo
{
public:
    Foo() : x(0) {}
    void print()
    {
        std::cout << "void Foo::print()" << std::endl;
    }
    double x;
};
Foo foo;

// obejct pointer as template parameter
template<Foo* pFoo>
void f2()
{
    pFoo->print();
}

// member function pointer as temlate parameter
template<void(Foo::*pnf)()>
void f3()
{
    Foo foo;
    (foo.*pnf)();
}

// data memeber pointer as template paramter
template<double Foo::*pData>
void f4()
{
    Foo foo;
    std::cout << foo.*pData << std::endl;
}

// lvalue reference to obejct as template parameter
template<Foo& rfoo>
void f5()
{
    rfoo.print();
}

// lvalue reference to function as template parameter
template<void(&func)()>
void f6()
{
    func();
}

// nullptr_t as template parameter
template<std::nullptr_t p>
void f7()
{
    std::cout << "void f7()" << std::endl;
}

int main(int argc, char const *argv[])
{
    f1<f>();
    f2<&foo>();
    f3<&Foo::print>();
    f4<&Foo::x>();
    f5<foo>();
    f6<f>();
    f7<nullptr>();
    static Foo foo2;
    f2<&foo2>();
    return 0;
}
