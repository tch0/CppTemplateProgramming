#include <iostream>

namespace X
{
template<typename T>
void f(T)
{
    std::cout << "template<typename T> void f(T)" << std::endl;
}
} // namespace X


namespace N
{
using namespace X; // ignored when ADL
enum E {e1};
void f(E)
{
    std::cout << "N::f(N::E)" << std::endl;
}
} // namespace N

void f(int)
{
    std::cout << "::f(int)" << std::endl;
}

int main(int argc, char const *argv[])
{
    ::f(N::e1); // qualified name, just ordinary lookup, no ADL
    f(N::e1); // unqualified name: ordinary lookup finds ::f() and ADL find() N::f(), N::f() is preferred
    return 0;
}
