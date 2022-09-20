#include <iostream>

// valid non-type template arguments
template<typename T, T NonTypeParam>
class C;

// integer
C<int, 33>* c1;

// address of variable
int a;
C<int*, &a>* c2;
constexpr int* addressOfA()
{
    return &a;
}
C<int*, addressOfA()>* c3;

// address of function
void f();
void f(int);
C<void(*)(int), f>* c4;

struct X
{
    static bool b;
    int n;
    constexpr operator int() const { return 42; }
};

// reference of variable
C<bool&, X::b>* c5;

// data member pointer
C<int X::*, &X::n>* c6;

// integer, but through implicit conversion
C<long, X{}>* c7;

int main(int argc, char const *argv[])
{
    
    return 0;
}
