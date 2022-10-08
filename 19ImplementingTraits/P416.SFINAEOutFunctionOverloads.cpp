#include <iostream>
#include <type_traits>

template<typename T>
struct IsDefaultConstructible
{
private:
    // test for types who has a default constructor
    template<typename U, typename = decltype(U())>
    static char test(void*);
    // test fallback
    template<typename>
    static long test(...);
public:
    static constexpr bool value = std::is_same_v<decltype(test<T>(nullptr)), char>;
};
template<typename T>
constexpr bool IsDefaultConstructible_v = IsDefaultConstructible<T>::value;

// implementation 2

template<typename T>
struct IsDefaultConstructible2Helper
{
private:
    // test for types who has a default constructor
    template<typename U, typename = decltype(U())>
    static std::true_type test(void*);
    // test fallback
    template<typename>
    static std::false_type test(...);
public:
    using type = decltype(test<T>(nullptr));
};

template<typename T>
struct IsDefaultConstructible2 : public IsDefaultConstructible2Helper<T>::type
{};

template<typename T>
constexpr bool IsDefaultConstructible2_v = IsDefaultConstructible2<T>::value;

struct X {};
struct Y 
{
    Y(int) {};
};

int main(int argc, char const *argv[])
{
    static_assert(IsDefaultConstructible_v<X>);
    static_assert(!IsDefaultConstructible_v<Y>);
    static_assert(IsDefaultConstructible2_v<X>);
    static_assert(!IsDefaultConstructible2_v<Y>);
    return 0;
}
