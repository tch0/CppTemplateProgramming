#include <iostream>
#include <vector>
#include <concepts>
#include <typeinfo>

class X
{
public:
    std::string str;
    int i;
    bool operator<(const X& x)
    {
        return str < x.str;
    }
};

class Y
{
public:
    std::string str;
    int i;
    bool operator>(const Y& x)
    {
        return str > x.str;
    }
};

template<typename T>
concept LessThanComparable = requires(T x, T y)
{
    { x < y } -> std::same_as<bool>;
};

template<typename T>
concept GreaterThanComparable = requires(T x, T y)
{
    { x > y } -> std::same_as<bool>;
};

template<typename T>
T max(T a, T b) requires LessThanComparable<T>
{
    std::cout << "max with LessThanComparable constraint" << std::endl;
    return b < a ? a : b;
}

template<typename T>
T max(T a, T b) requires GreaterThanComparable<T>
{
    std::cout << "max with GreaterThanComparable constraint" << std::endl;
    return a > b ? a : b;
}

int main(int argc, char const *argv[])
{
    X x1, x2;
    max(x1, x2);
    Y y1, y2;
    max(y1, y2);
    std::cout << typeid(max<X>).name() << std::endl; // F1XS_S_E
    std::cout << typeid(max<Y>).name() << std::endl; // F1YS_S_E
    return 0;
}
