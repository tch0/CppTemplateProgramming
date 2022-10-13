#include <iostream>
#include <type_traits>
#include <string>

template<typename... Types>
class Tuple;

template<typename Head, typename... Tail>
class Tuple<Head, Tail...>
{
private:
    Head head;
    Tuple<Tail...> tail;
public:
    // constructors
    Tuple() {} 
    template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
    Tuple(VHead&& _head, VTail&&... _tail)
        : head(std::forward<VHead>(_head))
        , tail(std::forward<VTail>(_tail)...) {}
    template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
    Tuple(const Tuple<VHead, VTail...>& other)
        : head(other.getHead())
        , tail(other.getTail()) {}
    template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
    Tuple(Tuple<VHead, VTail...>&& other)
        : head(std::move(other.getHead())
        , tail(std::move(other.getTail()))) {}

    Head& getHead() { return head; }
    const Head& getHead() const { return head; }
    Tuple<Tail...>& getTail() { return tail; }
    const Tuple<Tail...>& getTail() const { return tail; }
};

template<>
class Tuple<> {};

// get<>: get element of tuple recursively
template<unsigned N>
struct TupleGet
{
    template<typename Head, typename... Tail>
    static decltype(auto) apply(const Tuple<Head, Tail...>& t)
    {
        return TupleGet<N-1>::apply(t.getTail());
    }
    template<typename Head, typename... Tail>
    static decltype(auto) apply(Tuple<Head, Tail...>& t)
    {
        return TupleGet<N-1>::apply(t.getTail());
    }
    template<typename Head, typename... Tail>
    static decltype(auto) apply(Tuple<Head, Tail...>&& t)
    {
        return TupleGet<N-1>::apply(t.getTail());
    }
};

template<>
struct TupleGet<0>
{
    template<typename Head, typename... Tail>
    static const Head& apply(const Tuple<Head, Tail...>& t)
    {
        return t.getHead();
    }
    template<typename Head, typename... Tail>
    static Head& apply(Tuple<Head, Tail...>& t)
    {
        return t.getHead();
    }
    template<typename Head, typename... Tail>
    static Head&& apply(Tuple<Head, Tail...>&& t)
    {
        return t.getHead();
    }
    template<typename... Types>
    static int apply(const Tuple<Types...>& t) // fallback: for promptation
    {
        static_assert(sizeof...(Types) > 0, "The index if out of range of tuple.");
        return 0;
    }
};

// get<>
template<unsigned N, typename... Types>
decltype(auto) get(const Tuple<Types...>& t)
{
    return TupleGet<N>::apply(t);
};
template<unsigned N, typename... Types>
decltype(auto) get(Tuple<Types...>& t)
{
    return TupleGet<N>::apply(t);
};
template<unsigned N, typename... Types>
decltype(auto) get(Tuple<Types...>&& t)
{
    return TupleGet<N>::apply(t);
};

// makeTuple
template<typename... Types>
auto makeTuple(Types&&... elems)
{
    return Tuple<std::decay_t<Types>...>(std::forward<Types>(elems)...);
}

// comparison
bool operator==(const Tuple<>&, const Tuple<>&)
{
    return true;
}
template<typename Head1, typename... Tail1, typename Head2, typename... Tail2,
    typename = std::enable_if_t<sizeof...(Tail1) == sizeof...(Tail2)>>
bool operator==(const Tuple<Head1, Tail1...>& lhs, const Tuple<Head2, Tail2...>& rhs)
{
    return lhs.getHead() == rhs.getHead() && lhs.getTail() == rhs.getTail();
}

// output
template<typename... Args>
void printTuple(std::ostream& os, const Tuple<Args...>& t, bool isFirst = true)
{
    if constexpr (sizeof...(Args) > 0)
    {
        os << (isFirst ? "(" : ", ");
        os << t.getHead();
        printTuple(os, t.getTail(), false);
    }
    else
    {
        os << (isFirst ? "()" : ")");
    }
}
template<typename... Args>
std::ostream& operator<<(std::ostream& os, const Tuple<Args...>& t)
{
    printTuple(os, t);
    return os;
}

using namespace std::literals;

int main(int argc, char const *argv[])
{
    Tuple<int, double, std::string> t(1, 2.1, "hello");
    std::cout << get<2>(t) << std::endl;
    std::cout << get<2>(Tuple<int, double, std::string>(1, 2.1, "hello")) << std::endl;
    const Tuple<int, double, std::string> ct(1, 2.1, "hello");
    std::cout << get<2>(ct) << std::endl;
    auto t2 = makeTuple(1, 2.1, "hello");
    std::cout << get<2>(t2) << std::endl;
    std::cout << std::boolalpha;
    std::cout << (t == t2) << std::endl;

    std::cout << t << std::endl;
    std::cout << Tuple<>() << std::endl;
    return 0;
}
