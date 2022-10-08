#include <type_traits>

template<typename... Args>
struct TypeList {};

template<typename T>
struct IsFunction : std::false_type {};

template<typename R, typename... Args>
struct IsFunction<R(Args...)> : std::true_type
{
    using RetType = R;
    using Params = TypeList<Args...>;
    static constexpr bool variadic = false;
};
template<typename R, typename... Args>
struct IsFunction<R(Args..., ...)> : std::true_type
{
     using RetType = R;
    using Params = TypeList<Args...>;
    static constexpr bool variadic = true; // C-style varargs
};

template<typename T>
constexpr bool IsFunction_v = IsFunction<T>::value;

void f();
int add(int a, int b);
int sum(int a, int b, ...);

struct X
{
    void f();
    int add(int a, int b);
    void g() const;
};

int main(int argc, char const *argv[])
{
    static_assert(IsFunction_v<decltype(f)>);
    static_assert(IsFunction_v<decltype(add)>);
    static_assert(IsFunction_v<decltype(sum)>);
    return 0;
}
