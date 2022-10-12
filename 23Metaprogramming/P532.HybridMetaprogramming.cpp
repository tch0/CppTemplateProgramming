#include <iostream>
#include <type_traits>
#include <array>
#include <cassert>

template<typename T, std::size_t N>
auto dotProduct(const std::array<T, N>& x, const std::array<T, N>& y)
{
    T result{};
    for (std::size_t i = 0; i < N; ++i)
    {
        result += x[i] * y[i];
    }
    return result;
}

template<typename T, std::size_t N>
struct DotProduct
{
    static inline T result(const T* a, const T* b)
    {
        return *a * *b + DotProduct<T, N-1>::result(a+1, b+1);
    }
};
template<typename T>
struct DotProduct<T, 0>
{
    static inline T result(const T*, const T*)
    {
        return T{};
    }
};

template<typename T, std::size_t N>
auto dotProduct2(const std::array<T, N>& x, const std::array<T, N>& y)
{
    return DotProduct<T, N>::result(x.begin(), y.begin());
}

int main(int argc, char const *argv[])
{
    std::array<int, 3> arr1 = {1, 2, 3};
    auto arr2 = arr1;
    assert(dotProduct(arr1, arr2) == dotProduct2(arr1, arr2));
    return 0;
}
