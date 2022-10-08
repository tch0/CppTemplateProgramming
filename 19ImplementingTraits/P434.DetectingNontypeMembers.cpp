#include <type_traits>
#include <utility>
#include <vector>

#define DEFINE_HAS_MEMBER(Member)\
    template<typename, typename = std::void_t<>>\
    struct HasMember_##Member : public std::false_type {};\
    template<typename T>\
    struct HasMember_##Member<T, std::void_t<decltype(&T::Member)>> : public std::true_type {};\
    template<typename T>\
    constexpr bool HasMember_##Member##_v = HasMember_##Member<T>::value;

DEFINE_HAS_MEMBER(a)
DEFINE_HAS_MEMBER(b)
DEFINE_HAS_MEMBER(type);


struct X
{
    int a;
    int b();
    using type = int;
};

template<typename, typename = std::void_t<>>
struct Has_begin : public std::false_type {};

template<typename T>
struct Has_begin<T, std::void_t<decltype(std::declval<T>().begin())>> : public std::true_type {};

template<typename T>
constexpr bool Has_begin_t = Has_begin<T>::value;

int main(int argc, char const *argv[])
{
    static_assert(HasMember_a_v<X>);
    static_assert(HasMember_b_v<X>);
    static_assert(Has_begin_t<std::vector<int>>);
    return 0;
}
