#include <type_traits>

// special cases
template<typename From, typename To, bool = std::is_void_v<To> || std::is_array_v<To> || std::is_function_v<To>>
struct IsConvertibleHelper
{
    using type = std::bool_constant<std::is_void_v<To> && std::is_void_v<From>>; // If From and To are both void, the convesion will be valid.
};

// normal cases
template<typename From, typename To>
struct IsConvertibleHelper<From, To, false>
{
private:
    static void aux(To);
    template<typename F, typename = decltype(aux(std::declval<F>()))>
    static std::true_type test(void*);
    template<typename>
    static std::false_type test(...);
public:
    using type = decltype(test<From>(nullptr));
};

template<typename From, typename To>
struct IsConvertible : IsConvertibleHelper<From, To>::type {};

template<typename From, typename To>
using IsConvertible_t = IsConvertible<From, To>::type;

template<typename From, typename To>
constexpr bool IsConvertible_v = IsConvertible<From, To>::value;

// special cases


int main(int argc, char const *argv[])
{
    static_assert(IsConvertible_v<int, double>);
    static_assert(IsConvertible_v<void, void>);
    static_assert(!IsConvertible_v<int, void>);
    static_assert(!IsConvertible_v<int[10], int[10]>);
    static_assert(!IsConvertible_v<int(), int()>);
    return 0;
}
