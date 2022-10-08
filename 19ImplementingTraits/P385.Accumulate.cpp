#include <iostream>

template<typename T>
struct AccumulationTraits;
template<>
struct AccumulationTraits<char>
{
    using AccT = int;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<short>
{
    using AccT = int;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<int>
{
    using AccT = long;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<unsigned int>
{
    using AccT = unsigned long;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<float>
{
    using AccT = double;
    static constexpr AccT zero = 0;
};

template<typename T, typename AT = AccumulationTraits<T>>
auto accum(const T* beg, const T* end)
{
    using AccT = typename AT::AccT;
    AccT res = AT::zero;
    while (beg != end)
    {
        res += *beg;
        beg++;
    }
    return res;
}

int main(int argc, char const *argv[])
{
    int arr[3] = {1, 2, 3};
    std::cout << accum(arr, arr+3) << std::endl;
    return 0;
}
