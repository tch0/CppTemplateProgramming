#include <iostream>
#include <type_traits>
#include <string>
#include <cassert>
#include <utility>

// ======================================== tuple implementation ========================================
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
    Tuple(const Head& _head, const Tuple<Tail...>& _tail)
        : head(_head)
        , tail(_tail) {}
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

// ======================================== tuple algorithms ========================================

// 1. tuple as typelist

// front type of element
template<typename T>
struct Front;
template<typename Head, typename... Tail>
struct Front<Tuple<Head, Tail...>>
{
    using type = Head;
};
template<typename T>
using Front_t  = typename Front<T>::type;

// remove front type of tuple
template<typename T>
struct PopFront;
template<typename Head, typename... Tail>
struct PopFront<Tuple<Head, Tail...>>
{
    using type = Tuple<Tail...>;
};
template<typename T>
using PopFront_t  = typename PopFront<T>::type;

// push type to the front
template<typename T, typename Element>
struct PushFront;
template<typename... Types, typename Element>
struct PushFront<Tuple<Types...>, Element>
{
    using type = Tuple<Element, Types...>;
};
template<typename T, typename Element>
using PushFront_t = typename PushFront<T, Element>::type;

// push type to the back
template<typename T, typename Element>
struct PushBack;
template<typename... Types, typename Element>
struct PushBack<Tuple<Types...>, Element>
{
    using type = Tuple<Types..., Element>;
};
template<typename T, typename Element>
using PushBack_t = typename PushBack<T, Element>::type;

// 2. Adding to and removing from a tuple

// pushFront
template<typename... Types, typename V>
auto pushFront(const Tuple<Types...>& tuple, const V& value)
{
    return PushFront_t<Tuple<Types...>, V>(value, tuple);
}

// pushBack
template<typename V>
Tuple<V> pushBack(const Tuple<>&, const V& value) // basis case
{
    return Tuple<V>(value);
}
template<typename Head, typename... Tail, typename V>
auto pushBack(const Tuple<Head, Tail...>& tuple, const V& value) // recursive case
{
    return Tuple<Head, Tail..., V>(tuple.getHead(), pushBack(tuple.getTail(), value));
}

// popFront
template<typename... Types>
auto popFront(const Tuple<Types...>& tuple)
{
    return tuple.getTail();
}

// 3. Reversing a tuple
// reverse
Tuple<> reverse(const Tuple<>& t)
{
    return t;
}
template<typename Head, typename... Tail>
auto reverse(const Tuple<Head, Tail...>& t)
{
    return pushBack(reverse(t.getTail()), t.getHead());
}

// popBack
template<typename... Types>
auto popBack(const Tuple<Types...>& t)
{
    return reverse(popFront(reverse(t)));
}

// 4/5. Reversal with Index lists

// Auxiliary typelist operations for std::interger_sequence to reverse
template<typename T1, typename T2>
struct Append;
template<typename T, T Num1, T... Nums>
struct Append<std::integer_sequence<T, Nums...>, std::integer_sequence<T, Num1>>
{
    using type = std::integer_sequence<T, Nums..., Num1>;
};
template<typename T1, typename T2>
using Append_t = typename Append<T1, T2>::type;

template<typename T>
struct Reverse;
template<typename T>
using Reverse_t = typename Reverse<T>::type;
template<typename T>
struct Reverse<std::integer_sequence<T>> // basis case
{
    using type = std::integer_sequence<T>;
};
template<typename T, T Num1, T... Nums>
struct Reverse<std::integer_sequence<T, Num1, Nums...>> // recursive case
{
    using type = Append_t<Reverse_t<std::integer_sequence<T, Nums...>>, std::integer_sequence<T, Num1>>;
};

template<typename... Elements, unsigned... Indices>
auto reverseImpl(const Tuple<Elements...>& t, std::integer_sequence<unsigned, Indices...>)
{
    return makeTuple(get<Indices>(t)...);
}
template<typename... Types>
auto reverse2(const Tuple<Types...>& t)
{
    return reverseImpl(t, Reverse_t<std::make_integer_sequence<unsigned, sizeof...(Types)>>());
}

using namespace std::literals;

int main(int argc, char const *argv[])
{
    Tuple<int> t(10);
    // pushFront
    auto t2 = pushFront(pushFront(t, 2.1), "hello"s);
    assert(t2 == makeTuple("hello"s, 2.1, 10));
    // pushBack
    assert(pushBack(t2, "yes"s) == makeTuple("hello"s, 2.1, 10, "yes"s));
    // popFront
    assert(popFront(t2) == makeTuple(2.1, 10));
    // reverse
    assert(reverse(t2) == makeTuple(10, 2.1, "hello"s));
    // popBack
    assert(popBack(t2) == makeTuple("hello"s, 2.1));
    // reverse2
    assert(reverse2(t2) == makeTuple(10, 2.1, "hello"s));
    return 0;
}
