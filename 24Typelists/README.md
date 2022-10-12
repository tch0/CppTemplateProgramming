<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十四章：Typelists](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E5%9B%9B%E7%AB%A0typelists)
  - [解析Typelist](#%E8%A7%A3%E6%9E%90typelist)
  - [Typelist算法](#typelist%E7%AE%97%E6%B3%95)
  - [非类型Typelist](#%E9%9D%9E%E7%B1%BB%E5%9E%8Btypelist)
  - [使用包扩展优化算法](#%E4%BD%BF%E7%94%A8%E5%8C%85%E6%89%A9%E5%B1%95%E4%BC%98%E5%8C%96%E7%AE%97%E6%B3%95)
  - [Cons风格的Typelist](#cons%E9%A3%8E%E6%A0%BC%E7%9A%84typelist)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十四章：Typelists

我么在程序设计中需要使用各种各样的数据结构，元编程中也一样。
- 对于类型元编程来说，最重要的数据结构就是类型列表（typelist）。
- typelist，顾名思义，就是一个类型的列表。模板元程序可以操纵这些typelist，并最终体现在可执行程序中。
- 本章主要讨论typelist的相关技术，看一看类型元编程一般是怎么工作的。

## 解析Typelist

一个typelist就是一个可以被模板元程序操纵的类型列表：
- 并且提供一个列表需要的所有操作：遍历列表、添加元素、移除元素。
- 但是typelist不同于运行时的可变的列表，typelist不能修改，因为模板元编程是函数式的。
- 也就是说对typelist的添加、修改会得到新的typelist，而不是将其修改为新typelist。
- 一个典型实现：
```C++
template<typename... Elements>
struct Typelist {};
```
- 对typelist的操作通常来说需要将其分为两部分，通常是第一个元素与后续元素构成的typelist。
```C++
// extract first element
template<typename List>
struct Front;

template<typename Head, typename... Tail>
struct Front<Typelist<Head, Tail...>>
{
    using type = Head;
};

template<typename T>
using Front_t = typename Front<T>::type;

// pop first element, get the rest typelist
template<typename List>
struct Back;

template<typename Head, typename... Tail>
struct Back<Typelist<Head, Tail...>>
{
    using type = Typelist<Tail...>;
};

template<typename T>
using Back_t = typename Back<T>::type;

// push element to typelist
template<typename NewElement, typename List>
struct Cons;

template<typename NewElement, typename... Elements>
struct Cons<NewElement, Typelist<Elements...>>
{
    using type = Typelist<NewElement, Elements...>;
};

template<typename NewElement, typename List>
using Cons_t = typename Cons<NewElement, List>::type;
```

## Typelist算法

有了前面的3个基本操作，就可以实现更多操作了：查找、转换、翻转等。

索引：
```C++
template<typename List, unsigned N>
struct GetNth : public GetNth<Back_t<List>, N-1> {};

template<typename List>
struct GetNth<List, 0> : public Front<List> {};

template<typename List, unsigned N>
using GetNth_t = typename GetNth<List, N>::type;
```

寻找最佳匹配：
- 比如查找typelist中存储空间最大的类型。
```C++
template<typename List>
struct LargestType
{
private:
    using First = Front_t<List>;
    using Rest = typename LargestType<Back_t<List>>::type;
public:
    using type = std::conditional_t<(sizeof(First) > sizeof(Rest)), First, Rest>;
};

template<>
struct LargestType<Typelist<>>
{
    using type = char; // the smallest type
};

template<typename List>
using LargestType_t = typename LargestType<List>::type;
```

添加类型到Typelist末尾：
```C++
template<typename List, typename NewElement>
struct Append;

template<typename... Elements, typename NewElement>
struct Append<Typelist<Elements...>, NewElement>
{
    using type = Typelist<Elements..., NewElement>;
};

template<typename List, typename NewElement>
using Append_t = typename Append<List, NewElement>::type;
```
- 当然也可以经由`Cons_t Back_t Front_t`然后递归实现，只是没有必要。

反转Typelist：
```C++
template<typename List>
struct Reverse
{
    using type = Append_t<typename Reverse<Back_t<List>>::type, Front_t<List>>;
};

template<>
struct Reverse<Typelist<>>
{
    using type = Typelist<>;
};

template<typename List>
using Reverse_t = typename Reverse<List>::type;
```
- 有了反转之后，顺便实现移除尾元素：
```C++
template<typename List>
using PopBack_t = Reverse_t<Back_t<Reverse_t<List>>>;
```

转换Typelist：
```C++
template<typename List, template<typename> class MetaFunc> // MetaFunc::type is the result of transformation
struct Transform
{
    using type = Cons_t<typename MetaFunc<Front_t<List>>::type, typename Transform<Back_t<List>, MetaFunc>::type>;
};

template<template<typename> class MetaFunc>
struct Transform<Typelist<>, MetaFunc>
{
    using type = Typelist<>;
};

template<typename List, template<typename> class MetaFunc>
using Transform_t = typename Transform<List, MetaFunc>::type;
```
- 需要传入以类模板形式传入一个类型函数作为转换操作。

累加Typelist：
```C++
template<typename List,
         template<typename, typename> class F,
         typename Init>
struct Accumulate : public Accumulate<Back_t<List>, F, typename F<Init, Front_t<List>>::type> {};

template<template<typename, typename> class F, typename Init>
struct Accumulate<Typelist<>, F, Init>
{
    using type = Init;
};

template<typename List,
         template<typename, typename> class F,
         typename Init>
using Accumulate_t = typename Accumulate<List, F, Init>::type;
```
- 然后使用这个累加算法来实现前面的查找：
```C++
// using Accumulate implement LargestType
template<typename X, typename Y>
struct GE : public std::conditional<(sizeof(X) > sizeof(Y)), X, Y> {};

template<typename List>
using LargestType2_t = Accumulate_t<List, GE, char>;
```

插入排序：细节繁杂，有什么用？略。

总结：其实就是非常基础的函数式算法，不过是用模板这种晦涩的方式写出来，以及操作的实体是类型罢了。

## 非类型Typelist

除了类型列表，某些时候，一个非类型的typelist（话说不应该叫valuelist吗？）也是有用的。
- 有很多方法产生一个编译期的非类型typelist。
- 首先定义值：
```C++
template<typename T, T Value>
struct CTValue
{
    static constexpr T value = Value;
};
```
- 运算，比如相乘：
```C++
template<typename T, typename U>
struct Multiply;

template<typename T, T Value1, T Value2>
struct Multiply<CTValue<T, Value1>, CTValue<T, Value2>>
{
    using type = CTValue<T, Value1 * Value2>;
};

template<typename T, typename U>
using Multiply_t = typename Multiply<T, U>::type;
```
- 然后定义非类型typelist：
```C++
// nontype typelist
template<typename T, T... Values>
using CTTypelist = Typelist<CTValue<T, Values>...>;
```
- 然后就可以使用前面提供的一系列操作了。
```C++
template<typename T>
struct Add1;
template<typename T, T Value>
struct Add1<CTValue<T, Value>>
{
    using type = CTValue<T, Value+1>;
};

static_assert(std::is_same_v<Transform_t<CTTypelist<int, 2, 3>, Add1>, CTTypelist<int, 3, 4>>);
```

可推导的非类型参数：
- C++17开始，非类型模板参数可以使用`auto`推导其类型，可以写作这样：
```C++
template<auto Value>
struct CTValue
{
    static constexpr auto value = Value;
};
```
- 用的时候就不需要显式指定类型了。
- 如果需要异质的非类型值列表也可以使用`auto`推导非类型模板参数：
```C++
template<auto... Value>
class ValueList {};

int x;
using MyList = ValueList {1, 'a', 2.1, &x};
```

## 使用包扩展优化算法

实现Typelist算法时，包扩展是一个非常好的机制：
```C++
template<typename List, template<typename> class MetaFunc, bool = false> // MetaFunc::type is the result of transformation
struct Transform;

template<typename... Elements, template<typename> class MetaFunc> 
struct Transform<Typelist<Elements...>, MetaFunc, false>
{
    using type = Typelist<typename MetaFunc<Elements>::type...>; // using pack extension
};

template<template<typename> class MetaFunc>
struct Transform<Typelist<>, MetaFunc, true>
{
    using type = Typelist<>;
};

template<typename List, template<typename> class MetaFunc>
using Transform_t = typename Transform<List, MetaFunc>::type;
```
- 通过偏特化将元素捕捉到一个参数包中，即可方便地利用参数包扩展实现转换函数了。

## Cons风格的Typelist

另一种定义typelist的风格是`cons`风格，熟悉Lisp的话立刻就能触发肌肉记忆：
```C++
struct Nil {};

template<typename H, typename T>
struct Cons
{
    using Head = H;
    using Tail = T;
};

template<typename T>
struct CarImpl
{
    using type = typename T::Head;
};

template<typename T>
using Car = typename CarImpl<T>::type;

template<typename T>
struct CdrImpl
{
    using type = typename T::Tail;
};

template<typename T>
using Cdr = typename CdrImpl<T>::type;
```
- 凭借上面的东西就可以在C++中使用模板元编程造出一个Lisp子集了。

## 后记

- Boost.MPL元编程库被广泛用于操纵typelist。
- 略。
