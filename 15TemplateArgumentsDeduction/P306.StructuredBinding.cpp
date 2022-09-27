#include <iostream>
#include <utility>
#include <tuple>
#include <type_traits>

struct MaybeInt
{
    bool valid;
    int value;
};
MaybeInt g()
{
    return {true, 10};
}

int main(int argc, char const *argv[])
{
    const auto &&[b, N] = g();
    auto [i, d] = std::tuple<int, double>{5, 10.4};
    std::cout << std::boolalpha;
    std::cout << std::is_rvalue_reference_v<decltype(i)> << std::endl;
    std::cout << std::is_rvalue_reference_v<decltype(d)> << std::endl;
    std::cout << std::is_lvalue_reference_v<decltype(i)> << std::endl;
    std::cout << std::is_lvalue_reference_v<decltype(d)> << std::endl;
    std::cout << std::is_reference_v<decltype(i)> << std::endl;
    std::cout << std::is_reference_v<decltype(d)> << std::endl;
    std::cout << std::is_same_v<decltype(i), int> << std::endl;
    std::cout << std::is_same_v<decltype(d), double> << std::endl;
    return 0;
}
