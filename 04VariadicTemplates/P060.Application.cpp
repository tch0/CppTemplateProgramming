#include <iostream>
#include <string>
#include <vector>
#include <array>

// application of variadic templates

// perfect forwarding
template<auto Func, typename... Args> // Func is a non-type template paramter (and should be a function pointer)
void call(Args&&... args)
{
    Func(args...);
}

// fold expression
template<typename T>
class AddSpace
{
friend std::ostream& operator<<(std::ostream& os, AddSpace<T> s)
{
    return os << s.ref << " ";
}
private:
    T const& ref;
public:
    AddSpace(T const& r) : ref(r) {}
};
template<typename... Args>
void print(Args... args)
{
    (std::cout << ... << AddSpace(args)) << std::endl; // (init op ... op pack)
}

// variadic expression
template<auto Value, typename... Args>
void addAndPrintValue(Args... args)
{
    print(args + Value ...);
}

// variadic indices
template<typename Container, typename... Idx>
void printElems(const Container& c, Idx... idx)
{
    print(c[idx] ...); // also variadic expression
}

template<std::size_t... Idx, typename Container>
void printIdx(const Container& c)
{
    print(c[Idx]...);
}

// variadic class template
template <std::size_t...>
struct Indices {};

template<typename T, std::size_t... Idx>
void printByIndex(T t, Indices<Idx...>)
{
    print(std::get<Idx>(t)...);
}

// variadic deduction guides


// variadic base classes and using
template<typename... Bases>
struct Overloader : Bases...
{
    using Bases::operator()...;
};

int main(int argc, char const *argv[])
{
    call<print<int, double, std::string>>(1, 2.0, "hello");
    addAndPrintValue<10>(1, 2.0, 10, -1);
    printElems(std::vector<int>{1, 2, 3, 4, 5}, 0, 2, 4);
    printIdx<0, 2, 4>(std::vector<int>{1, 2, 3, 4, 5});
    printByIndex(std::array<int, 5>{1, 2, 3, 4, 5}, Indices<0, 2, 4>());
    return 0;
}
