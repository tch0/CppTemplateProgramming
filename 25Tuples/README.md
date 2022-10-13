<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十五章：元组](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E4%BA%94%E7%AB%A0%E5%85%83%E7%BB%84)
  - [基本元组设计](#%E5%9F%BA%E6%9C%AC%E5%85%83%E7%BB%84%E8%AE%BE%E8%AE%A1)
  - [基本元组操作](#%E5%9F%BA%E6%9C%AC%E5%85%83%E7%BB%84%E6%93%8D%E4%BD%9C)
  - [元组算法](#%E5%85%83%E7%BB%84%E7%AE%97%E6%B3%95)
  - [展开元组](#%E5%B1%95%E5%BC%80%E5%85%83%E7%BB%84)
  - [优化元组](#%E4%BC%98%E5%8C%96%E5%85%83%E7%BB%84)
  - [元组下标](#%E5%85%83%E7%BB%84%E4%B8%8B%E6%A0%87)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十五章：元组

在前面的章节，我们经常使用同质数据结构，同质数据结构扩展了C/C++数组的概念，几乎用在了所有的C++程序中。除此之外C/C++还提供了异质的数据结构——类/结构。
- 本章讨论的元组也是一种异质数据结构，和结构的组织方式比较类似。
- 元组和结构非常类似，区别在于结构通过名字引用成员，而元组通过位置/下标。
- 通过位置引用成员，并且能够从typelist构建使得元组比结构更适合用在模板元编程中。
- 另一个观察视角是元组是typelist在可执行程序中的体现。在模板元编程中，使用typelist生成元组用以存储一些数据是很常见的。
- 本章介绍元组的操作和实现，这个实现是`std::tuple`的简化版本。

## 基本元组设计

存储：
- `Tuple`保存每个模板参数类型的值一份，并且依次给下标`0 ~ N-1`，通过`get<N>(tuple)`来访问。
- 元组通常采用递归实现，也就是将元组分为`Head`和`Tail`两部分，`Head`是第一个元素，`Tail`是剩余元素的`Tuple`。
- 一个典型实现是将`Tail`作为成员：
```C++
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
    Tuple(const Head& _head, const Tail&... _tail) : head(_head), tail(_tail...) {}
    // ...
    Head& getHead() { return head; }
    const Head& getHead() const { return head; }
    Tuple<Tail...>& getTail() { return tail; }
    const Tuple<Tail...>& getTail() const { return tail; }
};

template<>
class Tuple<> {};
```
- 然后实现`get<>`：
```C++
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
```
- 需要注意的是，需要对三个版本重载：左值引用、const左值引用、右值引用。其中`TupleGet<0>`的最后一个重载是错误情况，旨在提供错误提示。

构造：
- 上面的构造并不全面，我们可能还会想要支持逐元素的移动构造，最好的方法是使用万能引用：
```C++
template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
Tuple(VHead&& _head, VTail&&... _tail)
    : head(std::forward<VHead>(_head))
    , tail(std::forward<VTail>(_tail)...) {}
```
- 还必须使用`std::enable_if`保证元素数量与参数数量一致。
- 然后是从其他兼容的Tuple拷贝或者移动构造：
```C++
template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
Tuple(const Tuple<VHead, VTail...>& other)
    : head(other.getHead())
    , tail(other.getTail()) {}
template<typename VHead, typename... VTail, typename = std::enable_if_t<sizeof...(Tail) == sizeof...(VTail)>>
Tuple(Tuple<VHead, VTail...>&& other)
    : head(std::move(other.getHead())
    , tail(std::move(other.getTail()))) {}
```
- 通过推导辅助构造`Tuple`的函数模板：
```C++
template<typename... Types>
auto makeTuple(Types&&... elems)
{
    return Tuple<std::decay_t<Types>...>(std::forward<Types>(elems)...);
}
```

## 基本元组操作

比较：
```C++
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
```

输出：
```C++
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
```

## 元组算法

元组算法通常来说同时需要编译期和运行时计算，就像typelist一样，在`Tuple`上应用算法可能得到完全不一样的类型的新`Tuple`，需要留意生成代码的性能。

元组作为typelist：
```C++
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

```
- 将上一章的typelist算法针对`Tuple`偏特化，主要用来决定元组算法的结果类型。

添加和移除元素：
```C++
// pushFront
template<typename... Types, typename V>
auto pushFront(const Tuple<Types...>& tuple, const V& value)
{
    return PushFront_t<Tuple<Types...>, V>(value, tuple); // when V is array?
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
```

反转元组，并借助反转元组实现移除尾元素：
```C++
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
```

索引列表（Index Lists）：
- 上面的反转列表是递归实现的，比较低效，元素多的时候拷贝数量会非常大。
- 一个理想的实现，反转列表应该只拷贝每个元素一次。
- 对于固定数量的元组，我们可以通过重载实现。
```C++
auto reversed = makeTuple(get<3>(t), get<2>(t), get<1>(t), get<0>(t));
```
- 更好的方式使用索引序列（index lists, aka index sequences，比如4,3,2,1,0这种）通过包扩展直接调用`makeTuple`或者构造：
```C++
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
```
- 这里实现时使用了标准库的`std::integer_sequence`，因为没有提供反转、添加移除元素等基础设施，所以需要先自己造一遍。更好的方式其实是使用上一章提供的typelist基础设施，并将整数序列实现为非类型typelist，借由上一章的基础设施即可快速实现。

洗牌与选择：
- 上面的`reverse2`实现即可作为洗牌的实现，只需要构造并传入用于洗牌的整数序列即可，略。
- 当然也可以对`Tuple`进行排序，略。

## 展开元组 

很多时候，我们会需要将元组中的元素展开，直接展开是无法展开的，我们需要通过包展开配合索引列表实现：
- 最常见的用途是将元组中元素作为参数调用一个函数：
```C++
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
```
- 标准库`<tuple>`中提供了`std::apply`做这件事情。还提供了`std::make_from_tuple`用于从`std::tuple`构造对象。

## 优化元组

元组用途广泛，所以思考如何在运行时与编译期去优化它是值得的。

元组与空基类优化：
- 因为空元组`Tuple<>`作为成员始终会占据一个字节，可以通过继承实现`Tuple`以实现空基类优化。
```C++
template<typpename Head, typename... Tail>
class Tuple<Head, Tail...> : private Tuple<Tail...>;
```
- 这样实现有一个问题，基类会在成员之前初始化，并且通常编译器会将基类成员存放在前，最终会造成逆序存储与初始化。
- 可以将第一个成员也作为基类解决：
```C++
template<typpename Head, typename... Tail>
class Tuple<Head, Tail...> : private TupleElement<Head>, private Tuple<Tail...>;
```
- 这样实现还是存在问题，比如`Tuple<int, int>`将会无法区分两个成员，因为两个元素分别存放在直接和间接继承的两个不同`TupleElement<int>`中。可以通过在`TupleElement`中引入一个索引（含义可以是高度）作为非类型模板参数解决。
```C++
template<typename Head, typename... Tail>
class Tuple<Head, Tail> : private TupleElement<Head, sizeof...(Tail)>,  private Tuple<Tail...>;
```
- 利用上面提到的非类型模板参数可以实现常数时间（指编译期实例化代价）取成员，而不需要递归。

## 元组下标

定义一个模板`operator[]`，传入一个包装了下标的编译期常量，比如`CTValue<unsigned, 0>`这种值，可以实现为元组重载下标。相对来说比较麻烦，直接用`get<>`就行，略。

## 后记

- `Boost.Tuple`是最流行的Tuple实现，最终在C++11并入了标准。
- 元组是最广泛使用的异质集合，但不是唯一一个。`Boost.Fusion`提供了通用异质集合实现，并且提供了在异质集合上编写程序的框架。
- `Boost.Hana`吸取了`Boost.MPL`和`Boost.Fusion`的许多经验，提供了非常强大、丰富组合的异质计算。
