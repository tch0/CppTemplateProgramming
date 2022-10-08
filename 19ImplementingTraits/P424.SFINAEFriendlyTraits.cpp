#include <type_traits>

template<typename, typename, typename = std::void_t<>>
struct HasPlus : public std::false_type {};

template<typename T1, typename T2>
struct HasPlus<T1, T2, std::void_t<decltype(std::declval<T1>() + std::declval<T2>())>> 
    : public std::true_type {};

template<typename T1, typename T2, bool = HasPlus<T1, T2>::value>
struct PlusResult
{
    using type = decltype(std::declval<T1>() + std::declval<T2>()); // for true
};
template<typename T1, typename T2>
struct PlusResult<T1, T2, false>
{
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;

int main(int argc, char const *argv[])
{
    static_assert(std::is_same_v<PlusResult_t<int, double>, double>);
    return 0;
}
