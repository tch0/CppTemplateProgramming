#include <type_traits>

template<bool Condition, typename TrueType, typename FalseType>
struct IfThenElse
{
    using type = TrueType;
};
template<typename TrueType, typename FalseType>
struct IfThenElse<false, TrueType, FalseType>
{
    using type = FalseType;
};

template<bool Condition, typename TrueType, typename FalseType>
using IfThenElse_t = typename IfThenElse<Condition, TrueType, FalseType>::type;

int main(int argc, char const *argv[])
{
    static_assert(std::is_same_v<IfThenElse_t<true, int, double>, int>);
    static_assert(std::is_same_v<IfThenElse_t<false, int, double>, double>);
    return 0;
}
