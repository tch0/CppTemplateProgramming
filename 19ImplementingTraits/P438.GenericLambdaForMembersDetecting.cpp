#include <utility>
#include <type_traits>
#include <vector>

// helper: checking validity of f(args...) for F f and Args... args
template<typename F, typename... Args,
    typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

// fallback if helpe SFINAE'd out
template<typename F, typename... Args>
std::false_type isValidImpl(...);

// define a lambda that takes a lambda f and return whether calling f with args is valid
inline constexpr auto isValid = [](auto f) {
    return [](auto&&... args) {
        return decltype(isValidImpl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

// helper template to represent a type as a value
template<typename T>
struct TypeT
{
    using type = T;
};
// helper to wrap a tyep as a value
template<typename T>
constexpr auto type = TypeT<T>{};

// helper to unwarp a wrapped type in unevaluated contexts
template<typename T>
T valueT(TypeT<T>); // no definition needed

// detecting member
constexpr auto hasSizeType = 
    isValid([](auto x) -> typename decltype(valueT(x))::size_type {});
template<typename T>
constexpr bool hasSizeType_v = hasSizeType(type<T>);

constexpr auto hasFirst = 
    isValid([](auto x) -> decltype((void)valueT(x).first) {});
template<typename T>
constexpr bool hasFirst_t = hasFirst(type<T>);

int main(int argc, char const *argv[])
{
    static_assert(hasSizeType_v<std::vector<int>>);
    static_assert(hasFirst_t<std::pair<int, double>>);
    return 0;
}
