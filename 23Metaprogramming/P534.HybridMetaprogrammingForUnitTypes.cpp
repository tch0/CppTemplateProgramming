#include <type_traits>
#include <iostream>

template<unsigned N, unsigned D = 1>
struct Ratio
{
    static constexpr unsigned num = N; // numerator
    static constexpr unsigned den = D; // denominator
    using type = Ratio<num, den>; // represent the unit type: N/D
};

// implementation of adding two ratios
template<typename R1, typename R2>
struct RatioAddImpl
{
private:
    static constexpr unsigned den = R1::den * R2::den;
    static constexpr unsigned num = R1::num * R2::den + R2::num * R1::den;
public:
    using type = Ratio<num, den>;
};

template<typename R1, typename R2>
using RatioAdd_t = typename RatioAddImpl<R1, R2>::type;

// using Ratio as unit type
template<typename T, typename U = Ratio<1>>
class Duration
{
public:
    using value_type = T;
    using unit_type = typename U::type;
private:
    value_type val;
public:
    constexpr Duration(value_type v = 0) : val(v) {}
    constexpr value_type value() const
    {
        return val;
    }
};

// operator+ of Durations
template<typename T1, typename U1, typename T2, typename U2>
auto constexpr operator+(const Duration<T1, U1>& lhs, const Duration<T2, U2>& rhs)
{
    using VT = Ratio<1, RatioAdd_t<U1, U2>::den>; // result unit type
    auto val = lhs.value() * VT::den / U1::den * U1::num + rhs.value() * VT::den / U2::den * U2::num;
    return Duration<decltype(val), VT>(val);
}

template<typename T, typename U>
std::ostream& operator<<(std::ostream& os, const Duration<T, U>& d)
{
    os << d.value() << " " << U::num << "/" << U::den << "s";
    return os;
}

int main(int argc, char const *argv[])
{
    static_assert(std::is_same_v<RatioAdd_t<Ratio<2, 3>, Ratio<5, 7>>, Ratio<29, 21>>);
    auto x = Duration<int, Ratio<1, 1000>>(13);
    auto y = Duration<int, Ratio<2, 3>>(10);
    std::cout << (x+y) << std::endl; // 20039 1/3000s
    return 0;
}
