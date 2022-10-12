#include <iostream>

template<typename T>
struct RemoveAllExtents
{
    using type = T;
};

template<typename T, std::size_t SZ>
struct RemoveAllExtents<T[SZ]>
{
    using type = typename RemoveAllExtents<T>::type;
};

template<typename T>
struct RemoveAllExtents<T[]>
{
    using tyle = typename RemoveAllExtents<T>::type;
};

template<typename T>
using RemoveAllExtents_t = typename RemoveAllExtents<T>::type;


int main(int argc, char const *argv[])
{
    int a[1][2][3];
    static_assert(std::is_same_v<RemoveAllExtents_t<decltype(a)>, int>);
    return 0;
}
