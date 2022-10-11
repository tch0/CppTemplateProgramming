#include <iostream>

template<typename Derived>
class EqualityCompare
{
    friend bool operator!=(const Derived& x1, const Derived& x2)
    {
        return !(x1 == x2);
    }
};

class X : public EqualityCompare<X>
{
    friend bool operator==(const X& x1, const X& x2)
    {
        return true;
    }
};

int main(int argc, char const *argv[])
{
    std::cout << std::boolalpha;
    std::cout << (X() == X()) << std::endl;
    std::cout << (X() != X()) << std::endl;
    return 0;
}
