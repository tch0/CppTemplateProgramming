#include <type_traits>

template<typename T, typename = std::void_t<>>
struct IsNothrowConstructible : public std::false_type {};

template<typename T>
struct IsNothrowConstructible<T, std::void_t<decltype(T(std::declval<T>()))>> 
    : public std::bool_constant<noexcept(T(std::declval<T>()))> {};

template<typename T>
constexpr bool IsNothrowConstructible_v = IsNothrowConstructible<T>::value;

class A
{
public:
    A(A&&) = delete;
};

class B
{
public:
    B(B&&);
};


int main(int argc, char const *argv[])
{
    static_assert(!IsNothrowConstructible_v<A>);
    static_assert(!IsNothrowConstructible_v<B>);
    return 0;
}
