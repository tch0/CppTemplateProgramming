#include <iostream>

template<unsigned p, unsigned d>
struct DoIsPrime
{
    static constexpr bool value = (p%d != 0) && DoIsPrime<p, d-1>::value;
};

// partial sepcialization, the end point of recursion
template<unsigned p>
struct DoIsPrime<p, 2>
{
    static constexpr bool value = (p%2 != 0);
};

template<unsigned p>
struct IsPrime
{
    // start recursion with divisor from p/2
    static constexpr bool value = DoIsPrime<p, p/2>::value;
};

// sepcial cases
template<>
struct IsPrime<0> { static constexpr bool value = false; };
template<>
struct IsPrime<1> { static constexpr bool value = false; };
template<>
struct IsPrime<2> { static constexpr bool value = true; };
template<>
struct IsPrime<3> { static constexpr bool value = true; };

// since C++11, use constexpr
// only one return statement can be in constexpr function
constexpr bool doIsPrime11(unsigned p, unsigned d)
{
    return d != 2 ? (p % d != 0) && doIsPrime11(p, d-1)
                  : (p % 2 != 0);
}
constexpr bool isPrime11(unsigned p)
{
    return p < 4 ? !(p < 2) : doIsPrime11(p, p/2);
}

// since C++14
constexpr bool isPrime14(unsigned p)
{
    for (unsigned d = 2; d <= p/2; ++d)
    {
        if (p % d == 0)
        {
            return false;
        }
    }
    return p > 1;
}

int main(int argc, char const *argv[])
{
    std::cout << std::boolalpha << IsPrime<97>::value << std::endl;
    std::cout << IsPrime<107>::value << std::endl;
    std::cout << IsPrime<781>::value << std::endl; // false

    std::cout << isPrime11(97) << std::endl;
    std::cout << isPrime11(107) << std::endl;
    std::cout << isPrime11(781) << std::endl;

    std::cout << isPrime14(97) << std::endl;
    std::cout << isPrime14(107) << std::endl;
    std::cout << isPrime14(781) << std::endl;
    return 0;
}
