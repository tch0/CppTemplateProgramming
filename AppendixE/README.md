<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [附录E：概念](#%E9%99%84%E5%BD%95e%E6%A6%82%E5%BF%B5)
  - [附录E内容](#%E9%99%84%E5%BD%95e%E5%86%85%E5%AE%B9)
    - [使用概念](#%E4%BD%BF%E7%94%A8%E6%A6%82%E5%BF%B5)
    - [定义概念](#%E5%AE%9A%E4%B9%89%E6%A6%82%E5%BF%B5)
    - [重载约束](#%E9%87%8D%E8%BD%BD%E7%BA%A6%E6%9D%9F)
    - [概念提示](#%E6%A6%82%E5%BF%B5%E6%8F%90%E7%A4%BA)
  - [C++20的约束与概念](#c20%E7%9A%84%E7%BA%A6%E6%9D%9F%E4%B8%8E%E6%A6%82%E5%BF%B5)
    - [概念](#%E6%A6%82%E5%BF%B5)
    - [约束](#%E7%BA%A6%E6%9D%9F)
    - [合取](#%E5%90%88%E5%8F%96)
    - [析取](#%E6%9E%90%E5%8F%96)
    - [原子约束](#%E5%8E%9F%E5%AD%90%E7%BA%A6%E6%9D%9F)
    - [约束规范化](#%E7%BA%A6%E6%9D%9F%E8%A7%84%E8%8C%83%E5%8C%96)
    - [requires子句](#requires%E5%AD%90%E5%8F%A5)
    - [requires表达式](#requires%E8%A1%A8%E8%BE%BE%E5%BC%8F)
    - [简单要求](#%E7%AE%80%E5%8D%95%E8%A6%81%E6%B1%82)
    - [类型要求](#%E7%B1%BB%E5%9E%8B%E8%A6%81%E6%B1%82)
    - [复合要求](#%E5%A4%8D%E5%90%88%E8%A6%81%E6%B1%82)
    - [嵌套要求](#%E5%B5%8C%E5%A5%97%E8%A6%81%E6%B1%82)
    - [约束的偏序](#%E7%BA%A6%E6%9D%9F%E7%9A%84%E5%81%8F%E5%BA%8F)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 附录E：概念

附录E介绍了C++17时还没有进入标准但即将进入C++20草案的概念是什么样子的，和C++20中已经进入标准的概念是有一点区别的。这里简单说明附录E内容后，将重点关注C++20中的约束与概念。

## 附录E内容

一个概念（a concept）是一个或者多个模板形参的约束集合。

### 使用概念

处理要求（requirements）：
- 例子：
```C++
template<typename T>
T max(T a, T b) requires LessThanComparable<T>
{
    return b < a ? a : b;
}
```
- 当需要使用概念对模板进行约束时，仅需要使用`requires`关键字，然后将约束写在参数列表后函数体前（也可以是模板参数列表后）。
- 这里的`LessThanComparable`是一个已经定义好的概念。这种概念是一个类似于布尔值的谓词，这个编译期常量表达式会产出一个`bool`类型的值。
- 因为是编译期的，所以对运行时的性能无任何影响。
- 如果约束产出了`false`，那么报错时会说明违反了哪个约束。
- `requires`子句并不需要一定表达为概念，所有`bool`类型的编译期常量表达式都可以使用。只是使用概念是一个更好的实践。

处理多个约束：
- `requires`子句可以不只有一个约束。多个约束之间可以使用`&&`或者`||`运算符。
- 其中的每一个约束都可以使用多个模板参数。

单个约束的快捷表示法：
- 比如上面的例子：
```C++
template<LessThanComparable T>
T max(T a, T b)
{
    return b < a ? a : b;
}
```
- 将`typename`关键字换为约束名称即可，这个表示形式和上面的例子在**功能上是等价**的。但是并非完完全全等价，如果重新声明改模板，必须使用同样形式。
- 多个约束使用`&&`运算时，可以选出其中一个概念这样用，剩余的依旧保留在`requires`子句中。

### 定义概念

概念在功能上就有点像`bool`类型的`constexpr`编译期常量，但是其类型不是显式定义的。例子：
```C++
template<typename T>
concept LessThanComparable = requires(T x, T y)
{
    { x < y } -> std::same_as<bool>;
};
```
- 概念需要模板参数，并使用关键字`concept`，接上概念名称之后需要一个`=`号，然后是一个用来建立类型`T`的约束的表达式（称之为约束表达式）。
- 在这个例子中，使用了`requires`表达式（requires expresssion），注意上面的`requires`子句（requires clause）不一样。有点像`noexcept`声明和`noexcept`表达式。
- `requires`表达式可以包含一个可选的形参列表，但是这些形参不会被任何实参所替换，而是作为占位的变量，用来在`requires`表达式的`{}`体中表达约束。
- `requires`表达式的body中可以写`{ x < y } -> bool;`这样的约束（这样的约束叫做复合要求），含义是`x < y`必须是一个合法的表达式，并且结果能够转换为`bool`。【注意在C++20中，不能直接写成`bool`而要写成一个类型约束比如`std::same_as<bool>`】。
- `{}`后面`->`前面可以有`noexcept`声明，表明`noexcept(...)`使用这个表达式的结果应该为`true`。
- 如果不需要返回值这个约束，那么`-> type`可以不用写。
- 如果只需要检查这个表达式中的合法性，那么外层的`{}`可以不用写，此时称之为简单要求（simple requirement）。
- 例子：
```C++
template<typename T>
concept Swappable = requires(T x, T y) {
    swap(x, y);
};
```
- `requires`表达式中也可以表达需要关联类型的存在，称之为类型要求（type requirement）：
```C++
template<typename Seq>
concept Sequence = requires(Seq seq)
{
    typename Seq::iterator; // need nested type iterator exist, this is called a type requirement
    { seq->begin() } -> Seq::iterator;
};
```
- 类型要求除了嵌套类型，也可以要求该类型作为另一个模板的模板参数时合法，例如`typename IteratorFor<Seq>`。
- 除此之外，requires表达式中还包含第三类的要求语句，即对其他概念的调用，也成为嵌套要求（nested requirement）。
```C++
template<typename Seq>
concept Sequence = requires(Seq seq)
{
    typename Seq::iterator;
    requires Iterator<typename Seq::Iterator>; // invoke another concept
    { seq->begin() } -> Seq::iterator;
};
```

### 重载约束

假定`IntegerLike StringLike`是已经定义好的两个概念，那么对于这两个重载：
```C++
template<IntegerLike T> void print(T val); // version 1
template<StringLike T> void print(T val); // version 2
```
- 调用时，如果满足前者不满足后者就会调用版本1，如果不满足前者满足后者就调用版本2，都满足则有歧义，和SFINAE是一个道理。

约束包含：
- 通常来说，通过约束来重载的函数他们的约束应该要互斥，不然约束都满足时会发生歧义。
- 但是就现实情况来说，约束更多地是互相包含而非互斥的，比如标准库的迭代器种类。
- 如果两个重载的约束是包含关系，并且同时匹配两个重载，那么将会选择其中包含另一个约束的那一个（也就是限制更多、范围更窄的那一个）。
- 至于两个概念是否具有包含关系的判断，则不在此处叙述。最简单的情况就是其中一个约束使用了另一个约束来定义，并且包含了其他约束，那么前者包含后者。（注意被包含的约束范围更广，包含另一个约束的约束范围更窄，不要套用集合的包含关系来理解）。

约束与标签分发：
- 回顾第二十章介绍的通过标签分发重载，这种方法可以和约束化的模板搭配起来使用。
```C++
template<typename T>
concept ForwardIterator = InputIterator<T> && requires {
    typename std::iteator_traits<T>::iteator_category;
    std::is_convertible_v<std::iterator_traits<T>::iterator_category, std::forward_iterator_tag>;
};
```
- 然后通过`requires InputIterator<T>`和`requires ForwardIterator<T>`不同约束重载即可根据迭代器种类分发。

### 概念提示

测试概念：
- 概念是布尔型的谓词，也同样是合法的布尔型常量表达式。
- 同样可以用来初始化`bool`类型变量，或者用在`static_assert`中。
- 当编写概念时，最好同时针对一些类型进行测试。
- 当设计概念时，询问以下问题：
    - 使用这个概念的接口或者算法需要目标对象类型的拷贝、移动吗？
    - 哪些转换是可接受的？需要哪些转换？
    - 模板需要的基本操作集合唯一吗？比如，模板使用`*=`或者`*`和`=`都能完成操作吗？

概念的粒度（concept granularity）：
- 一个很自然的想法是编写一个概念库，包含常用的约束，用的时候组合拼装起来就很方便。
- 但是一个很自然的问题是以什么样的粒度来提供这些概念，比如对于迭代器：提供了各种不同迭代器类型之后，还需要提供不同类型的区间吗，以及不同迭代器类型的序列？

## C++20的[约束与概念](https://zh.cppreference.com/w/cpp/language/constraints)

类模板、函数模板以及非模板函数（通常是类模板成员），可以关联到约束（constraint），约束指出了对模板实参的一些要求。这些要求被用于选择最恰当的函数重载和函数特化。

这种要求的集合称为概念（concept），每个概念都是一个谓词，在编译期求值，并在被用于约束时成为接口的一部分。

例子，对能够使用`std::hash<T>()`获取哈希值的类型定义概念：
```C++
template<typename T>
concept Hashable = requires(T a)
{
    { std::hash<T>{}(T) } -> std::convertible_to<std::size_t>;
};
```
- 在定义函数模板时，将这个概念作为约束，下面三种写法功能上等价。
```C++
template<Hashable T>
void f(T);

template<typename T> requires Hashable<T>
void g(T);

template<typename T>
void h(T) requires Hashable<T>;
```
- 在模板实例化的早期就会检查约束是否得到满足：
```C++
struct X {};
f(X{}); // ERROR
```
- g++中报错信息是这个样子的：会指出实例化失败时，不满足哪个概念的哪一个约束。报错会更加清晰。
```
Concept.cpp: In function 'int main(int, const char**)':
Concept.cpp:40:6: error: no matching function for call to 'f(X)'
   40 |     f(X{});
      |     ~^~~~~
Concept.cpp:14:6: note: candidate: 'template<class T>  requires  Hashable<T> void f(T)'
   14 | void f(T)
      |      ^
Concept.cpp:14:6: note:   template argument deduction/substitution failed:
Concept.cpp:14:6: note: constraints not satisfied
Concept.cpp: In substitution of 'template<class T>  requires  Hashable<T> void f(T) [with T = X]':
Concept.cpp:40:6:   required from here
Concept.cpp:8:9:   required for the satisfaction of 'Hashable<T>' [with T = X]
Concept.cpp:8:20:   in requirements with 'T a' [with T = X]
Concept.cpp:10:21: note: the required expression 'std::hash<_Tp>{}(a)' is invalid
   10 |     { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
      |       ~~~~~~~~~~~~~~^~~
cc1plus.exe: note: set '-fconcepts-diagnostics-depth=' to at least 2 for more detail
```

### 概念

概念（concept）是要求（requirement）的具名集合。概念的定义必须在命名空间作用域。形式：
```C++
template<template-parameter-list>
concept concept-name = constraint-expression;
```
- 概念不能提及自身，且不能受约束。
- 概念不能被显式实例化、显式特化、或者偏特化，即不能更改约束的原初定义的含义。
- 概念可以在标识表达式（id-expression）中命名，该标识表达式的值在约束表达式满足时是`true`，否则是`false`。
- 概念在作为以下内容的一部分时可以在类型约束中被命名：
    - 类型模板形参声明。
    - 占位类型说明符。
    - 复合要求。
- 概念在**类型约束**中接受的实参比他的形参列表要求少一个，按语境推导出的类型会作为第一个实参：
```C++
template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;
template<Derived<Base> T>
void f(T); // T is constrainted by Derived<T, Base>
```

### 约束

**约束**（constraint）是逻辑运算（`&& ||`）和操作数的序列，指定了对模板实参的要求，可以在`requires`表达式中出现。也可以直接作为概念的主体。

有三种类型的约束：
- 合取（conjunction）
- 析取（disjunction）
- 原子约束（atomic constraint）

确定与一个声明关联的约束时，会对包含遵守以下顺序对约束表达式进行规范化：
- 每个声明中受约束的类型模板形参（比如`Derived<Base> T`这种）、或者带占位符声明的非类型模板形参（比如`std::integral`这种）所引入的约束表达式，按照出现顺序。
- 模板形参列表之后的`requires`子句的约束表达式。
- 简写函数模板声明中每个拥有约束占位符（constraint placeholder）的形参所引入的约束表达式。
- 尾部的`requires`子句中的表达式。

受约束的声明只能以相同的语法形式重声明，不要求诊断。

### 合取

两个约束的合取通过在约束表达式中使用`&&`来构成。
```C++
template<typename T>
concept SignedIntegral = std::is_integral_v<T> && std::is_signed_v<T>;
```
- 合取只有在两个约束都满足时才会得到满足。
- 合取从左到右**短路求值**，如果不满足左侧约束，不会尝试对右侧约束进行模板实参替换，这是为了防止语境外的替换所导致的失败。

### 析取

两个约束的析取通过在约束表达式中使用`||`来构成。
```C++
template<class T = void> requires EqualityComparable<T> || Same<T, void>
struct equal_to;
```
- 如果其中一个约束得到满足，那么两个约束的析取就会得到满足。析取同样从左到右短路求值，如果满足左侧约束则不会对右侧约束进行模板实参替换。

### 原子约束

原子约束有一个表达式E，和一个从E内出现的模板形参到模板实参的映射组成，这种映射称为形参映射。
- 原子约束是在约束规范化的过程中形成的，E始终不会是逻辑与或者逻辑或表达式。
- 对原子约束是否满足的检查会通过替换形参和各个模板实参到表达式E中来进行。如果替换产生了无效的类型或者表达式，那么约束就不满足。否则在任何左值到右值转换后，E应当是`bool`类型的纯右值常量表达式，当且仅当其求值为`true`时该约束表达式得到满足。
- E在替换后必须严格为`bool`，不能有任何转换。
- 如果两个原子约束由在源码层面相同的表达式组成，且他们的形参映射等价，那么认为他们是等同的。

### 约束规范化

约束规范化（constraint normalization）是将约束表达式转换成一系列原子约束的析取和合取的过程。约束表达式的规范化形式定义如下：
- 表达式`(E)`的规范化形式（normal form）就是`E`的规范化形式。
- 表达式`E1 && E2`的规范化形式就是`E1`和`E2`的规范化形式的合取。
- 表达式`E1 || E2`的规范化形式就是`E1`和`E2`的规范化形式的析取。
- 表达式`C<A1, A2, ..., AN>`（其中C是某个概念名称）的规范化形式是以`A1, A2, ..., AN`对C的每个原子约束的形参映射中的C的对应模板形参进行替换之后，C的约束表达式的规范化形式。如果这种形参映射的替换产生了无效的类型或者表达式，那么程序非良构，不要求诊断。
- 任何其他表达式`E`的规范化形式是一条原子约束，它的表达式是`E`而它的形参映射是恒等映射。包括所有折叠表达式，甚至包括以`&& ||`运算符进行的折叠表达式。
- 用户定义的`&& ||`重载在约束规范化上无效。

### requires子句

关键字`requires`用来引入`requires`子句，指定对模板实参，或者对函数声明的约束。
- 这种时候，关键字`requires`必须后随某个常量表达式（可以写成`requires true`），可以使用某个具名概念、具名概念的合取或者析取、或者一个`requires`表达式。
- 表达式必须具有下列形式之一：
    - 初等表达式：比如`std::is_integral_v<T> Swappable<T>`或者任何带括号的表达式。
    - 以`&&`连接的初等表达式序列。
    - 以`||`连接的初等表达式序列。

### requires表达式

关键字`requires`也可以用来作为`requires`表达式的开始，其是一个`bool`类型的纯右值表达式，用来描述一些对模板实参的约束，这种表达式在约束得到满足时是`true`，否则是`false`。
- 语法：
    - `requires { requirement-sequence }`
    - `requires(optional-parameter-list) { requirement-sequence }`
- 形参列表：不能有默认实参，且不能包含包展开，这些形参没有存储期、链接期、或者生存期，只是用来辅助进行各个要求的制定。这些形参的要求序列在`}`前处于作用域中。
- 要求序列：要求（requirement）的序列，描述如下（分号结尾）。
- 要求序列中的要求可以是如下要求之一：
    - 简单要求（simple requirement）。
    - 类型要求（type requirement）。
    - 复合要求（compound requirement）。
    - 嵌套要求（nested requirement）。
- 要求可以提及处于作用域中的模板形参，由形参列表引入的局部形参，以及从它的外围语境中可见的任何声明。
- 对模板化实体（所有参数化的实体，不必然本身是模板）中所使用的`requires`表达式进行模板实参替换，可能导致在其要求中形成无效的类型或者表达式，或者违反这些要求的语义约束。这种情况下，该`requires`表达式值为`false`，而不会导致程序非良构。替换和语义约束按照词法顺序执行，在遇到能够确定`requires`表达式结果的条件时终止。如果替换和语义约束检查成功，那么该`requires`表达式的结果为`true`。
- 如果`requires`表达式在其约束中有无效表达式或者类型，而它并不在模板化实体的声明中出现，那么程序非良构。

### 简单要求

不以`requires`关键字起始的任意表达式语句。断言该表达式合法，该表达式不会求值操作数，只检查语义正确性。
```C++
template<typename T>
concept Addable = requires(T a, T b)
{
    a + b; // simple requirement: require that a + b is a valid expression.
};
````

### 类型要求

在关键字`typename`后随一个可以有限定的类型名。要求是该类型名合法：
- 可以用来要求某个嵌套具名类型存在。
- 或某个类模板特化指明一个类型（就是该特化要合法的意思）。
- 或某个个别名模板指明一个类型（这个别名是合法的）。
- 指明类模板特化的类型不要求类型完整。
```C++
template<typename T>
using Ref = T&;
template<typename T>
concept C = requires
{
    typename T::inner; // T has a nested type inner
    typename S<T>; // sepcialization S<T> is valid
    typename Ref<T>; // T& is valid
}
```

### 复合要求

形式：`{ expression } noexcept -> return-type-requirement`
- 其中`noexcept`和返回类型要求是可选的。
- 返回类型要求的形式是：`-> type-constraint`。
- 复合要求断言一个具名表达式的各项性质。以以下顺序进行替换和语义约束检查：
    - 如果存在的话替换模板实参到表达式。
    - 如果使用了`noexcept`，那么表达式必须不会抛出异常。
    - 如果出现了返回类型要求，那么：
        - 替换模板实参到返回值要求中。
        - `decltype((expression))`必须满足该类型约束所蕴含的约束。否则外围`requires`表达式为`false`。
```C++
template<typename T>
concept C2 = requires(T x)
{
    // expression *x must be valid
    // type T::inner must be valid
    // the type of *x must be able to convert to T::inner
    { *x } -> std::covertible_to<typename T::inner>;

    // x + 1 must be valid
    // std::same_as<decltype((x+1)), int> must be fulfilled
    // that means x + 1 must be prvalue of int
    { x + 1 } -> std::same_as<int>;
}
```

### 嵌套要求

嵌套要求形式：`requires constraint-expression;`。
- 可以用局部形参来指定额外的约束，约束表达式必须被替换的模板实参所满足。

### 约束的偏序

在任何进一步的分析之前，都会对各个约束进行规范化。对每个具名概念的主体和`requires`表达式进行替换，直到剩下不可分割的约束的合取与析取序列为止。

如果根据约束P和约束Q中的各个原子约束的同一性证明P蕴含Q（即P能推导出Q，亦即P发生的话，Q一定会发生，或者叫做P是Q的充分条件）。那么称之为P归入Q（P subsume Q，P包含Q）。

归入关系定义了约束的偏序，用于确定：
- 重载决议中非模板函数的最佳可行候选。
- 重载集中的非模板函数地址。
- 模板模板实参的最佳匹配。
- 类模板特化的偏序。
- 函数模板的偏序。

如果声明D1和D2均受约束，且D1关联的约束能归入D2关联的约束，或者D2没有约束，那么称D1与D2相比至少一样受约束。如果D1至少与D2一样受约束，而D2并非至少与D1一样受约束，那么D1比D2更受约束。

当重载选择时，同时满足两个重载的约束，且约束之间有归入关系时，将会优先匹配更受约束的那一个版本。

## 总结

- `requires`用在声明中称之为`requires`子句，其后需要跟约束表达式。
- `bool`类型的编译期常量表达式可以作为约束，`requires`表达式也可以作为约束，约束与约束之间可以合取和析取，以构造约束表达式。
- 定义概念时就是将一个约束表达式的值作为这个概念的值。概念是隐式的`bool`类型编译期常量。
- `requires`表达式可以有参数列表，其中包含了各种要求：普通要求仅要求表达式合法，类型要求要求类型合法，复合要求同时要求表达式和类型，可能还有异常要求，嵌套要求可以在`requires`表达式中使用`requires`表达式调用其他概念。
- 原子约束、约束规范化和约束的偏序相关。
- 两个声明的约束之间可能有偏序关系，当他们的约束有偏序关系时，称一个比另一个更受约束，重载决议时将选择更受约束的那一个版本。
- 概念在类型约束中接受的实参比他的形参列表要求少一个，按语境推导出的类型会作为第一个实参。最常见的地方是：
    - 简写到模板形参列表中代替`typename`的类型约束。
    - `requires`表达式的类型要求中，表达式的类型将作为类型约束的第一个参数。

个人总结：
- 虽然不能说对概念已经一清二楚，但基本语法已经知道了。
- 概念的偏序在普通场景应该是比较好判断的，但真复杂了是不好分辨的。按我的理解，应该尽量避免约束具有偏序关系的场景，最好是互斥的比较好。
- 概念还需要结合概念库内容才好用，还需要熟悉一下[概念库`<concepts>`](https://zh.cppreference.com/w/cpp/concepts)，速览一下内容：
    - 核心语言概念：`same_as` `derived_from` `convertible_to` `common_reference_with` `common_with` `integral` `signed_integral` `unsigned_integral` `floating_point` `assignable_from` `swappable swappable_with` `destructible` `constructible_from` `default_initializable` `move_constructible` `copy_constructible`
    - 比较概念：`boolean_testable` `equality_comparable equality_comparable_with` `totally_ordered totally_ordered_with` `three_way_comparable` `three_way_comparable_with`
    - 对象概念：`movable copyable semiregular` `regular`
    - 可调用概念：`invocable` `regular_invocable` `prediate` `relation` `equivalence_relation` `strict_weak_order`
    - 更多功能可在迭代器库、算法库、范围库中找到。
    - 很多概念的定义都是通过当前标准库中类型特性来定义的，标准库的类型特性也可以用在`requires`表达式中，用起来差别也并没有那么大。
