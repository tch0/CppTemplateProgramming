#include <iostream>

template<auto Value>
void f()
{
    std::cout << Value << std::endl;
}

int main(int argc, char const *argv[])
{
    std::cout << std::boolalpha;
    f<0>();
    f<false>();
    f<10ull>();
    return 0;
}
