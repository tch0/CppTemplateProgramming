#include <type_traits>
#include <string>

template<typename T>
struct RParam
{
    using type = std::conditional_t<(sizeof(T) <= 2*sizeof(void*))
                                     && std::is_trivially_copy_constructible_v<T>
                                     && std::is_trivially_move_constructible_v<T>,
                                    T,
                                    const T&>;
};
template<typename T>
using RParam_t = typename RParam<T>::type;

template<typename T1, typename T2>
void foo(RParam_t<T1> a, RParam_t<T2> b)
{
}

int main(int argc, char const *argv[])
{
    foo<int, int>(1, 2);
    foo<std::string, int>("hello", 2);
    return 0;
}
