<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第八章：编译期编程](#%E7%AC%AC%E5%85%AB%E7%AB%A0%E7%BC%96%E8%AF%91%E6%9C%9F%E7%BC%96%E7%A8%8B)
  - [模板元编程](#%E6%A8%A1%E6%9D%BF%E5%85%83%E7%BC%96%E7%A8%8B)
  - [使用constexpr计算](#%E4%BD%BF%E7%94%A8constexpr%E8%AE%A1%E7%AE%97)
  - [偏特化的路径选择](#%E5%81%8F%E7%89%B9%E5%8C%96%E7%9A%84%E8%B7%AF%E5%BE%84%E9%80%89%E6%8B%A9)
  - [SFINAE/替换失败不是错误](#sfinae%E6%9B%BF%E6%8D%A2%E5%A4%B1%E8%B4%A5%E4%B8%8D%E6%98%AF%E9%94%99%E8%AF%AF)
  - [编译期if](#%E7%BC%96%E8%AF%91%E6%9C%9Fif)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第八章：编译期编程

C++有许多特性支持编译期编程：
- C++98之前，甚至就已经支持了编译期编程，包括循环和路径选择。不过很多人认为这是对模板的滥用，因为需要不直观的方式书写，比如模板递归和重载。
- 有了偏特化，我们可以在编译期根据不同约束选择不同的类模板。
- 有了SFINAE原则，可以根据不同类型和约束在不同函数模板之间做选择。
- 在C++11和C++14中，因为`constexpr`的引入，提供了更符合直觉的编译期编程风格。
- C++17引入了编译期`if`，可以根据编译期条件和约束抛弃一部分语句。甚至在模板外都能够工作。

## 模板元编程

C++的模板实例化是编译期的（与之相比动态语言的泛型是在运行时的），这就导致了C++的实例化过程结合某些特性之后创造了一种原始的递归的编译期编程语言，使用这种语言进行编程的风格就叫做**模板元编程**。23章将会详细讲述模板元编程，这里这是提纲挈领概括一下可能性。模板元编程是C++中的黑魔法一样的终极存在，用得好可以将很多运算从运行时更改为编译期，从而提升程序性能，代价是巨大的编译时间的延长。

先看一个例子，编译期计算一个数是否是质数：
```C++
template<unsigned p, unsigned d>
struct DoIsPrime
{
    static constexpr bool value = (p%d != 0) && DoIsPrime<p, d-1>::value;
};

// partial sepcialization, the end point of recursion
template<unsigned p>
struct DoIsPrime<p, 2>
{
    static constexpr bool value = (p%2 != 0);
};

template<unsigned p>
struct IsPrime
{
    // start recursion with divisor from p/2
    static constexpr bool value = DoIsPrime<p, p/2>::value;
};

// sepcial cases
template<>
struct IsPrime<0> { static constexpr bool value = false; };
template<>
struct IsPrime<1> { static constexpr bool value = false; };
template<>
struct IsPrime<2> { static constexpr bool value = true; };
template<>
struct IsPrime<3> { static constexpr bool value = true; };
```
- 用`constexpr`静态变量实现计算，递归以实现循环，偏特化作为递归终止条件。所有的计算都在编译期通过模板特化完成，这就是（古早）模板元编程的基本玩法。
- C++11引入`constexpr`前，通常做法是定义为类内枚举常量以避免静态变量的类外定义。引入`constexpr`之后，因为默认`inline`，所以不再需要类外定义。
    - 枚举值做法：`enum { value = (p%d != 0) && DoIsPrime<p, d-1>::value }`，算是模板元编程中的一个典型技巧。
- 可以说这种做法相当笨拙，很多人都说模板元编程也是一门函数式语言，也是图灵完全的。其他的不好说，但如果可以算一门函数式编程语言，那么一定是世界上最丑的函数式编程语言。

## 使用constexpr计算

- C++11刚引入`constexpr`时还有很严格的限制，比如`constexpr`函数只能有一条返回语句。
- C++14移除了大部分限制，这让`constexpr`函数可以更加灵活地做到更多事情。
- 但是依然需要`constexpr`函数内所有事情都能够在编译期做到，当前能在编译期做到的事情还不包括堆内存分配和异常处理。
- 在C++11中上述质数判断程序可以简化为：
```C++
// since C++11, use constexpr
constexpr bool doIsPrime11(unsigned p, unsigned d)
{
    return d != 2 ? (p % d != 0) && doIsPrime11(p, d-1)
                  : (p % 2 != 0);
}
constexpr bool isPrime11(unsigned p)
{
    return p < 4 ? !(p < 2) : doIsPrime11(p, p/2);
}
```
- C++14起，还可以进一步简化：
```C++
constexpr bool isPrime14(unsigned p)
{
    for (unsigned d = 2; d <= p/2; ++d)
    {
        if (p % d == 0)
        {
            return false;
        }
    }
    return p > 1;
}
```
- C++14起和普通函数看起来已经完全没有任何区别，仅仅只是添加一个`constexpr`声明以及要遵守计算都能够发生在编译期的要求而已。
- 相比最早期的编译期计算要进行大量的模板实例化，会造成程序编译时间大量增加和二进制的夸张膨胀。使用`constexpr`可以说非常优美了。
- 注意在需要编译期常量的场景中调用`constexpr`函数时，一定会在编译期求值。
- 当块作用域中调用`constexpr`函数，且参数都是编译期常量时，编译器会自行选择是编译期求值还会运行时求值，通常来说编译器会尽可能在编译期求值。
- 当参数不是编译期常量时，一定不会编译期求值。如果在需要编译期常量的场景中这样使用，那么就会报错。

## 偏特化的路径选择

例子：
- 某些带约束的类模板要做到不同情况不同实现，可以通过偏特化所有情况，并且不给出主模板定义来做：
```C++
template<int SZ, bool = isPrime(SZ)>
struct Helper;
template<int SZ>
struct Helper<SZ, false>
{
    ...
}
template<int SZ>
struct Helper<SZ, true>
{
    ...
}
```
- 也可以主模板使用一个默认情况下的默认实现，并为其他情况偏特化。
```C++
template<int SZ, bool = isPrime(SZ)>
struct Helper
{
    ... // default for false
}
template<int SZ>
struct Helper<SZ, true>
{
    ...
}
```

对于函数模板，因为不支持偏特化，需要使用其他机制来做到不同约束下的不同实现：
- 使用类模板的静态函数。
- 使用`enable_if`。
- 使用SFINAE机制，接下来介绍。
- 使用C++17起的编译期`if`特性。
- 第20章会详述。

## SFINAE/替换失败不是错误

[SFINAE](https://zh.cppreference.com/w/cpp/language/sfinae)的全称是Subtitution Failure Is Not An Error，也就是替换失败不是错误。（读作sfee-nay）
- C++中为不同的参数列表重载函数是一件非常常见的事情。此时需要从候选函数集中选出最佳匹配，而函数模板也可以重载，并且通常模板参数经由推导而来。但函数模板可能会面临一些问题：某些函数模板可能在模板实参替换形参后完全没有意义（语法上是错误的），此时C++标准的选择是忽略这些模板，而非报告错误。
- 这种机制就叫做SFINAE，替换失败并非错误。
- 也就是说在这种情况下，不合法的模板将会被忽略，从候选函数集合中剔除。
- 这里说的**替换**（Substitution）与实例化是两个不同的过程：
    - 替换发生在实例化之前，替换是指使用模板实参替换模板形参的过程，进而确定这个函数模板是否应该继续留在候选函数集合中。
    - 函数模板不合法/替换失败则会忽略该函数模板，合法则留在集合中，最终从集合中选出最佳匹配。如果最佳匹配时函数模板，则会进行实例化。
    - 注意替换过程不会检查函数体，所有工作都只处理函数模板声明，函数重载决议只需要函数签名，不需要函数体。
- CppReference上SFINAE的定义是：在函数模板的重载决议中会应用此规则：当模板形参在替换成显式指定的类型或推导出的类型失败时，从重载集中丢弃这个特化，而非导致编译失败。

SFINAE和重载决议：
- 在阅读C++标准库文档时如果看到“在某些情况下该重载不参与重载决议”，就意味着这里需要使用SFINAE了。
- 现在，SFINAE时常被作为一个动词，当说到SFINAE out一个函数模板就是指在某些约束下把这个模板非法化的意思。
- 为了简化SFINAE的实现难度，标准库提供了`enable_if`。用法前面有介绍，通常用在模板形参列表最后添加一个不命名模板参数。

使用`decltype`的表达式SFINAE：
- 很多时候要找到一个公式化的编译期表达式来放在`enable_if`中并不简单。
- 看一个例子：
```C++
template<typename T>
typename T::size_type len(T const& t)
{
    return t.size();
}
```
- 其中`typename T::size_type`被用作SFINAE，如果模板实参没有嵌套类型`size_type`，那么替换失败，这个函数模板将在重载决议时被排除。
- 但是如果有`size_type`但是没有`size`成员函数，但这个函数模板又是最佳匹配时（注意其他重载有可能针对该类型是合法且才是应该调用的哪一个，但这个重载却是最佳匹配），从而导致编译错误。
- 这时就应该将含有`size`成员函数的约束加入到SFINAE的条件中。
- 但这种情况通过`enable_if`是不好写的。
- 这时可以选择使用`auto`作为返回值类型，然后使用尾置返回值配合`decltype`将所有需要满足的约束放在其中，配合逗号表达式做到SFINAE。
```C++
template<typename T>
auto len(T const& t) -> decltype((void)(t.size()), T::size_type())
{
    return t.size();
}
```
- 此时就需要含有`size()`成员函数才能编过，逗号表达式的类型是最后一个子表达式类型，其含义未发生任何变化，但新增了约束。
- 其中`void`类型转换的作用是为了避免用户重载逗号运算符使表达式含义发生改变。但通常来说是不推荐重载逗号运算符的，如果你清楚逗号运算符绝不会被重载，那么去掉这个转换也无所谓。
- `delctype`中的表达式不会求值。

最后更多SFINAE的细节参见[文档](https://zh.cppreference.com/w/cpp/language/sfinae)。15章和19章也会介绍更多细节和用途。

替代方案：
- SFINAE是比较古典的方案了（早于C++11）。
- SFINAE的替代，只要可用通常都比SFINAE更好：
    - 标签分发，典型例子是不同迭代器类型不同实现如`std::distance`，新增标签参数通过重载区分两种不同实现。
    - 编译期`if`，将不同约束下的实现合并到同一个模板中，通过编译期`if`来选择。
    - 概念。
- 如果只想条件性的编译时错误（而非为了在某种约束下在两个同样匹配程度的函数模板中禁用不合理的那一个以避免二义，也就是使用SFINAE单纯是为了在不满足约束时报错。这种用法是存在合理性的，如果不做任何约束等到实例化失败时才报错，报错信息可能会篇幅更长且更难以理解），`static_assert`通常比SFINAE合适。

## 编译期if

偏特化、SFINAE、`std::enable_if`使我们能够使一整个模板有效或者无效。C++17引入了一个更加灵活的机制——编译期`if`，提供了使某一段特定的代码根据编译期条件有效或者无效的机制。
- 语法：`if constexpr(...)`，即是在`if`后添加`constexpr`即可，条件中需要是编译期表达式。
- 根据编译期表达式决定是使用then部分还是else部分，使用其中一部分时，不会为另一部分生成代码，也就是说未生成的那一部分不需要是合法的（这是编译期`if`和普通`if`的本质差别）。
- 例子，在没有编译期`if`时，要输出一个`std::tuple`可能需要这样写，使用类模板的静态成员函数配合类模板偏特化（当然还有很多其他写法，比如将整数包装在一个类模板中，用函数模板重载来做）：
```C++
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
```
- 有了编译期`if`之后就变得非常简单和清晰：
```C++
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
```
- 14章会提供更多细节。

## 总结

- 模板提供了编译期计算的能力：使用递归来迭代，偏特化和`?:`来进行选择。
- 通过`constexpr`函数，可以用普通函数替换大部分在编译期表达式中调用的编译期计算。
- 通过使用偏特化，可以根据编译期约束选择不同的类模板实现。
- 函数模板中模板实参对形参的替换不一定会产生合法的代码，此时函数重载会被忽略，这个原则称为SFINAE。
- SFINAE被用来提供仅针对特定类型或者满足特定约束时才有效的函数模板。
- 编译期`if`提供了根据编译期条件丢弃一部分代码的能力，在模板外也可用。
