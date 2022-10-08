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

class SumPolicy
{
public:
    template<typename T1, typename T2>
    static void accumulate(T1& total, const T2& value)
    {
        total += value;
    }
};

class MultPolicy
{
public:
    template<typename T1, typename T2>
    static void accumulate(T1& total, const T2& value)
    {
        total *= value;
    }
};

template<typename T, typename Policy = SumPolicy, typename Traits = AccumulationTraits<T>>
auto accum(const T* beg, const T* end)
{
    using AccT = typename Traits::AccT;
    AccT res = Traits::zero;
    while (beg != end)
    {
        Policy::accumulate(res, *beg);
        beg++;
    }
    return res;
}

int main(int argc, char const *argv[])
{
    int arr[3] = {1, 2, 3};
    std::cout << accum(arr, arr+3) << std::endl;
    std::cout << accum<int, MultPolicy>(arr, arr+3) << std::endl;
    return 0;
}
