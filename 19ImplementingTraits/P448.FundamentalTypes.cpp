#include <type_traits>

template<typename T>
struct IsFundamentalType : public std::false_type {};

template<typename T>
constexpr bool IsFundamentalType_v = IsFundamentalType<T>::value;

#define MK_FUNDAMENTAL_TYPE(T)\
    template<>\
    struct IsFundamentalType<T> : public std::true_type {}

MK_FUNDAMENTAL_TYPE(bool);
MK_FUNDAMENTAL_TYPE(char);
MK_FUNDAMENTAL_TYPE(signed char);
MK_FUNDAMENTAL_TYPE(unsigned char);
MK_FUNDAMENTAL_TYPE(wchar_t);
MK_FUNDAMENTAL_TYPE(char16_t);
MK_FUNDAMENTAL_TYPE(char32_t);
MK_FUNDAMENTAL_TYPE(signed short);
MK_FUNDAMENTAL_TYPE(unsigned short);
MK_FUNDAMENTAL_TYPE(signed int);
MK_FUNDAMENTAL_TYPE(unsigned int);
MK_FUNDAMENTAL_TYPE(signed long);
MK_FUNDAMENTAL_TYPE(unsigned long);
MK_FUNDAMENTAL_TYPE(signed long long);
MK_FUNDAMENTAL_TYPE(unsigned long long);
MK_FUNDAMENTAL_TYPE(float);
MK_FUNDAMENTAL_TYPE(double);
MK_FUNDAMENTAL_TYPE(long double);
MK_FUNDAMENTAL_TYPE(std::nullptr_t);


int main(int argc, char const *argv[])
{
    static_assert(IsFundamentalType_v<char>);
    return 0;
}
