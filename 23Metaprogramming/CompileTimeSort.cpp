#include <algorithm>
#include <array>

template<typename T, T... Nums>
struct Sequence
{
    static constexpr auto toArray()
    {
        return std::array{Nums...};
    }
};

// concat multiple sequences
template<typename... Seqs>
struct SeqConcat;

template<typename... Seqs>
using SeqConcat_t = typename SeqConcat<Seqs...>::type;

template<typename Seq1, typename... Seqs>
struct SeqConcat<Seq1, Seqs...>
{
    using type = SeqConcat_t<Seq1, SeqConcat_t<Seqs...>>;
};
template<typename T, T... Nums1, T... Nums2>
struct SeqConcat<Sequence<T, Nums1...>, Sequence<T, Nums2...>>
{
    using type = Sequence<T, Nums1..., Nums2...>;
};

// partition
template<typename T>
struct Partition;

template<typename T, T Pivot>
struct Partition<Sequence<T, Pivot>>
{
    using LeftPart = Sequence<T>;   // < Pivot
    using RightPart = Sequence<T>;  // >= Pivot, but exclude Pivot itself, this is the key point
};

template<typename T, T Pivot, T Num1, T... Nums>
struct Partition<Sequence<T, Pivot, Num1, Nums...>>
{
    using LeftPart = SeqConcat_t<std::conditional_t<(Num1 < Pivot), Sequence<T, Num1>, Sequence<T>>,
                                 typename Partition<Sequence<T, Pivot, Nums...>>::LeftPart>;
    using RightPart = SeqConcat_t<std::conditional_t<(Num1 < Pivot), Sequence<T>, Sequence<T, Num1>>,
                                  typename Partition<Sequence<T, Pivot, Nums...>>::RightPart>;
};

// quick sort
template<typename Seq>
struct QuickSort;

template<typename T>
using QuickSort_t = typename QuickSort<T>::type;

template<typename T>
struct QuickSort<Sequence<T>> { using type = Sequence<T>; };

template<typename T, T Num>
struct QuickSort<Sequence<T, Num>> { using type = Sequence<T, Num>; };

template<typename T, T Pivot, T... Nums>
struct QuickSort<Sequence<T, Pivot, Nums...>>
{
    using type = SeqConcat_t<QuickSort_t<typename Partition<Sequence<T, Pivot, Nums...>>::LeftPart>,
                             Sequence<T, Pivot>,
                             QuickSort_t<typename Partition<Sequence<T, Pivot, Nums...>>::RightPart>>;
};

template<typename Range>
constexpr auto sort(Range&& range)
{
    std::sort(std::begin(range), std::end(range));
    return range;
}

using TestSequence = Sequence<int,
    62, 21, 27, 85,  4, 50, 40, 66, 60, 81, 
    16, 12, 22, 69, 23, 28, 10, 58, 15, 90,
    33,  9, 13,  5, 31, 52, 91, 65,  6, 39, 
    71, 86,  7,  0, 34, 57, 67, 38, 87, 98, 
    61,  1, 35, 54,  8, 70, 48, 99, 46, 42, 
    55, 89, 29, 80, 44, 47, 49, 72, 19, 51,
     3, 73,  2, 79, 94, 96, 82, 75, 77, 95, 
    56, 30, 36, 76, 64, 88, 24, 41, 53, 68,
    93, 43, 97, 63, 14, 84, 92, 18, 32, 45,
    83, 37, 59, 25, 20, 17, 74, 11, 78, 26>;

int main(int argc, char const *argv[])
{
    // modern C++
    {
        constexpr auto arr = sort(std::array{3, 2, 1, 4, 5});
        static_assert(std::is_sorted(arr.begin(), arr.end()));
    }
    // classic C++
    {
        constexpr auto arr = QuickSort_t<TestSequence>::toArray();
        static_assert(std::is_sorted(arr.begin(), arr.end()));
    }
    return 0;
}
