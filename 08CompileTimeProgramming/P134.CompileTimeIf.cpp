#include <iostream>
#include <tuple>
#include <string>

namespace impl_class_template
{
template<std::size_t N, typename Tuple>
class PrintTuple
{
public:
    static void print(std::ostream& os, const Tuple& t)
    {
        PrintTuple<N-1, Tuple>::print(os, t);
        os << ", " << std::get<N-1>(t);
    }
};

template<typename Tuple>
class PrintTuple<1, Tuple>
{
public:
    static void print(std::ostream& os, const Tuple& t)
    {
        os << std::get<0>(t);
    }
};

template<typename... Args>
std::ostream& operator<<(std::ostream& os, const std::tuple<Args...>& t)
{
    os << "(";
    PrintTuple<sizeof...(Args), std::tuple<Args...>>::print(os, t);
    return os << ")";
}
}

namespace impl_if_constexpr
{
template<std::size_t Index, typename... Args>
std::ostream& printTuple(std::ostream& os, const std::tuple<Args...>& t)
{
    if constexpr (Index == 0) // first element
    {
        os << "(";
    }
    if constexpr (Index < sizeof...(Args) - 1)
    {
        os << std::get<Index>(t) << ", ";
        return printTuple<Index + 1, Args...>(os, t);
    }
    else // last element
    {
        return os << std::get<Index>(t) << ")";
    }
}

template<typename... Args>
std::ostream& operator<<(std::ostream& os, const std::tuple<Args...>& t)
{
    return printTuple<0, Args...>(os, t);
}
}

int main(int argc, char const *argv[])
{
    {
        using namespace impl_class_template;
        std::tuple<int, std::string, double, std::string> t{1, "hello", 2.0, "world"};
        std::cout << t << std::endl;
    }
    {
        using namespace impl_if_constexpr;
        std::tuple<int, std::string, double, std::string> t{1, "hello", 2.0, "world"};
        std::cout << t << std::endl;
    }
    return 0;
}
