#include <iostream>
#include <utility>
#include <type_traits>

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

// expanding tuple
template<typename F, typename... Elements, std::size_t... Indices>
decltype(auto) applyImpl(F&& f, const Tuple<Elements...>& t, std::index_sequence<Indices...>)
{
    return f(get<Indices>(t)...);
}
template<typename F, typename... Elements>
decltype(auto) apply(F&& f, const Tuple<Elements...>& t)
{
    return applyImpl(std::forward<F>(f), t, std::make_index_sequence<sizeof...(Elements)>());
}

void f(int a, double b, std::string s)
{
    std::cout << a << ", " << b << ", " << s << std::endl;
}

using namespace std::literals;
int main(int argc, char const *argv[])
{
    auto t = makeTuple(1, 2.1, "hello"s);
    apply(f, t);
    return 0;
}
