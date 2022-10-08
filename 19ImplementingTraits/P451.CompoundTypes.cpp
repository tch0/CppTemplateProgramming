#include <type_traits>

// pointer
template<typename T>
struct IsPointer : public std::false_type {};

template<typename T>
struct IsPointer<T*> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsPointer_v = IsPointer<T>::value;

// reference
// lvalue reference
template<typename T>
struct IsLvalueReference : public std::false_type {};

template<typename T>
struct IsLvalueReference<T&> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsLvalueReference_v = IsLvalueReference<T>::value;

// rvalue reference
template<typename T>
struct IsRvalueReference : public std::false_type {};

template<typename T>
struct IsRvalueReference<T&&> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsRvalueReference_v = IsRvalueReference<T>::value;

// lvalue and rvalue reference
template <class T> struct IsReference      : std::false_type {};
template <class T> struct IsReference<T&>  : std::true_type {};
template <class T> struct IsReference<T&&> : std::true_type {};
template<typename T>
constexpr bool IsReference_v = IsReference<T>::value;

// array
template<typename T>
struct IsArray : public std::false_type {};
template<typename T, std::size_t N>
struct IsArray<T[N]> : public std::true_type
{
    using base = T;
    static constexpr std::size_t size = N;
};
template<typename T>
struct IsArray<T[]> : public std::true_type
{
    using base = T;
    static constexpr std::size_t size = 0;
};
template<typename T>
constexpr bool IsArray_v = IsArray<T>::value;

// pointer to member
template<typename T>
struct IsMemberPointer : public std::false_type {};
template<typename T, typename C>
struct IsMemberPointer<T C::*> : public std::true_type
{
    using memberType = T;
    using classType = C;
};
template<typename T>
constexpr bool IsMemberPointer_v = IsMemberPointer<T>::value;

// for test
struct X
{
    int a;
    void f();
};

int main(int argc, char const *argv[])
{
    static_assert(IsPointer_v<int*>);
    static_assert(std::is_same_v<IsPointer<int*>::base, int>);
    static_assert(IsLvalueReference_v<int&>);
    static_assert(std::is_same_v<IsLvalueReference<int&>::base, int>);
    static_assert(IsRvalueReference_v<int&&>);
    static_assert(std::is_same_v<IsRvalueReference<int&&>::base, int>);
    static_assert(IsReference_v<int&>);
    static_assert(IsReference_v<int&&>);
    static_assert(!IsArray_v<int>);
    static_assert(IsArray_v<int[10]>);
    static_assert(IsArray_v<int[]>);
    static_assert(IsMemberPointer_v<decltype(&X::a)>);
    static_assert(IsMemberPointer_v<decltype(&X::f)>);
    return 0;
}
