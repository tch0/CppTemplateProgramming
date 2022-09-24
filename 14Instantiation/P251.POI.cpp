#include <iostream>

class MyInt
{
public:
    MyInt(int i);
};

MyInt operator-(const MyInt&);
bool operator>(const MyInt&, const MyInt&);

using Int = MyInt;

template<typename T>
void f(T i)
{
    if (i > 0)
    {
        g(-i);
    }
}

void g(Int)
{
    f<Int>(42);
}
// POI of f<Int>

int main(int argc, char const *argv[])
{
    
    return 0;
}
