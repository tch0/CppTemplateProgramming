#include <iostream>
#include <type_traits>
#include <vector>

// detect size type member type
template<typename, typename = std::void_t<>>
struct HasSizeType : public std::false_type {};

template<typename T>
struct HasSizeType<T, std::void_t<typename std::decay_t<T>::size_type>> : public std::true_type {};

template<typename T>
constexpr bool HasSizeType_t = HasSizeType<T>::value;

// detect any member type
// can be achieved by macros for now
#define DEFINE_HAS_TYPE(MemType)\
    template<typename, typename = std::void_t<>>\
    struct HasMemberType_##MemType : public std::false_type {};\
    template<typename T>\
    struct HasMemberType_##MemType<T, std::void_t<typename std::decay_t<T>::MemType>> : public std::true_type {};\
    template<typename T>\
    constexpr bool HasMemberType_##MemType##_t = HasMemberType_##MemType<T>::value;

DEFINE_HAS_TYPE(size_type)
DEFINE_HAS_TYPE(value_type)

int main(int argc, char const *argv[])
{
    static_assert(HasSizeType_t<std::vector<int>>);
    static_assert(HasSizeType_t<std::vector<int>&>);
    static_assert(HasMemberType_size_type_t<std::vector<int>>);
    static_assert(HasMemberType_size_type_t<std::vector<int>&>);
    static_assert(HasMemberType_value_type_t<std::vector<int>>);
    static_assert(HasMemberType_value_type_t<std::vector<int>&>);
    return 0;
}
