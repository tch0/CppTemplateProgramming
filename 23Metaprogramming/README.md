<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十三章：元编程](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E4%B8%89%E7%AB%A0%E5%85%83%E7%BC%96%E7%A8%8B)
  - [现代C++元编程的发展状态](#%E7%8E%B0%E4%BB%A3c%E5%85%83%E7%BC%96%E7%A8%8B%E7%9A%84%E5%8F%91%E5%B1%95%E7%8A%B6%E6%80%81)
  - [反射元编程的维度](#%E5%8F%8D%E5%B0%84%E5%85%83%E7%BC%96%E7%A8%8B%E7%9A%84%E7%BB%B4%E5%BA%A6)
  - [模板递归实例化的代价](#%E6%A8%A1%E6%9D%BF%E9%80%92%E5%BD%92%E5%AE%9E%E4%BE%8B%E5%8C%96%E7%9A%84%E4%BB%A3%E4%BB%B7)
  - [计算完整性](#%E8%AE%A1%E7%AE%97%E5%AE%8C%E6%95%B4%E6%80%A7)
  - [递归实例化和递归模板参数](#%E9%80%92%E5%BD%92%E5%AE%9E%E4%BE%8B%E5%8C%96%E5%92%8C%E9%80%92%E5%BD%92%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0)
  - [枚举值与静态常量](#%E6%9E%9A%E4%B8%BE%E5%80%BC%E4%B8%8E%E9%9D%99%E6%80%81%E5%B8%B8%E9%87%8F)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十三章：元编程

元编程是编写编写程序的程序，也就是系统通过执行我们的代码来生成最终用于执行功能的代码。
- 和其他编程技术一样，都是为了一个更小的代价实现更多的功能。这个代价可以是代码尺寸、代码维护性等方面的。
- 元编程是用户定义的编译期计算。
- 底层动机通常是性能考虑（将计算尽可能从运行时提前到编译期，获得更佳性能）或者接口简化（将多种类型接口统一到模板中）。
- 模板元编程通常依赖于特征和类型函数。

## 现代C++元编程的发展状态

C++模板元编程一直都在进化，这里讨论常用的模板元编程手段。

值元编程：
- 在C++03之前，要做到在编译期编写一个复杂的程序，比如求一个整数的平方根，需要使用模板递归实例化。而在C++14之后（C++11就引入了，不过支持功能太少，C++14才算是比较可用），我们可以使用`constexpr`做到。
-  不赘述。
- [CompileTimeSort.cpp](CompileTimeSort.cpp)展示了一个编译期快速排序的古典做法与现代做法。也就是模板递归实例化与`constexpr`函数。

类型元编程：
- 第19章已经介绍了一些类型函数，将类型作为输入，产出一个新类型或者值。不过第19章的都是比较简单的标准库中的例子。
- 通过使用模板元编程的支柱——递归模板实例化，可以实现更复杂的类型编程。
- 看一个例子，得到一个数组的底层类型：
```C++
template<typename T>
struct RemoveAllExtents
{
    using type = T;
};
template<typename T, std::size_t SZ>
struct RemoveAllExtents<T[SZ]>
{
    using type = typename RemoveAllExtents<T>::type;
};
template<typename T>
struct RemoveAllExtents<T[]>
{
    using tyle = typename RemoveAllExtents<T>::type;
};
template<typename T>
using RemoveAllExtents_t = typename RemoveAllExtents<T>::type;
```
- 通过递归调用类型函数本身，我们可以得到任意维数的数组的底层类型。
- C++标准库提供了`std::remove_all_extents`做这件事情。

混合元编程：
- 运用值元编程和类型元编程，我们可以在编译期计算某些值。
- 但是我们对运行时效果更感兴趣，将这些元程序用在运行时代码中，称之为混合元编程。
- 看一个例子，计算两个`std::array`的点乘结果：
```C++
template<typename T, std::size_t N>
auto dotProduct(const std::array<T, N>& x, const std::array<T, N>& y)
{
    T reuslt{};
    for (std::size_t i = 0; i < N; ++i)
    {
        result += x[i] * y[i];
    }
    return result;
}
```
- 因为`std::array`的长度是编译期已知的，所以循环的部分其实是可以优化的，优化成`result += x[0]*y[0]; result += x[1]*y[1]; ...`。
- 虽然现代的编译期可能会对这个逻辑做循环展开（loop unrolling），优化掉循环开销。但为了讨论，这里还是讨论一下元编程做法：
```C++
template<typename T, std::size_t N>
struct DotProduct
{
    static inline T result(const T* a, const T* b)
    {
        return *a * *b + DotProduct<T, N-1>::result(a+1, b+1);
    }
};
template<typename T>
struct DotProduct<T, 0>
{
    static inline T result(const T*, const T*)
    {
        return T{};
    }
};

template<typename T, std::size_t N>
auto dotProduct2(const std::array<T, N>& x, const std::array<T, N>& y)
{
    return DotProduct<T, N>::result(x.begin(), y.begin());
}
```
- 通过内联展开加上模板递归实例化就可以避免循环的开销。注意必须内联才能避免开销，否则又会引入新的函数调用开销，好在一般来说这么简单的逻辑现代编译器应该都能内联展开。
- 这个例子混合了编译期计算和运行时计算。
- 我们可以注意到固定尺寸的数组在混合元编程中很有用，但其实**元组**（Tuple）才是混合元编程中最重要的容器。
- 一个元组是一个固定大小的混合类型容器，标准库提供了`std::tuple`。因为元组在现代C++程序中的重要性，[第二十五章](../25Tuples)将会详细地介绍如何编写一个元组。
- 无论是`std::array`还是元组还是单纯的结构/聚合类都可以用在模板元编程中，当然联合也是可以的。C++17为此引入了`std::variant`，[第二十六章](../26DiscriminatedUnions/)会编写一个类似的东西。
- `std::tuple std::variant`类似于结构，被称之为异质类型，使用这种类型的混合元编程也被称之为异质元编程。

单位类型的混合元编程：
- 混合元编程的另一个例子是计算不同单位类型的值。值的计算发生在运行时，但是结果单位的计算发生在编译期。
- 看例子，定义一个表示分数的单位类型`Ratio`：
```C++
template<unsigned N, unsigned D = 1>
struct Ratio
{
    static constexpr unsigned num = N; // numerator
    static constexpr unsigned den = D; // denominator
    using type = Ratio<num, den>; // represent the unit type: N/D
};

// implementation of adding two ratios
template<typename R1, typename R2>
struct RatioAddImpl
{
private:
    static constexpr unsigned den = R1::den * R2::den;
    static constexpr unsigned num = R1::num * R2::den + R2::num * R1::den;
public:
    using type = Ratio<num, den>;
};

template<typename R1, typename R2>
using RatioAdd_t = typename RatioAddImpl<R1, R2>::type;
```
- 然后就可以利用`Ratio`作为单位来定义类型了，比如时间间隔的单位。
```C++
// using Ratio as unit type
template<typename T, typename U = Ratio<1>>
class Duration
{
public:
    using value_type = T;
    using unit_type = typename U::type;
private:
    value_type val;
public:
    constexpr Duration(value_type v = 0) : val(v) {}
    constexpr value_type value() const
    {
        return value;
    }
};
```
- 比较有趣的`Duration`的加法操作，支持两个不同单位的`Duration`，在相加时就需要先统一到同一单位下。
```C++
// operator+ of Durations
template<typename T1, typename U1, typename T2, typename U2>
auto constexpr operator+(const Duration<T1, U1>& lhs, const Duration<T2, U2>& rhs)
{
    using VT = Ratio<1, RatioAdd_t<U1, U2>::den>; // result unit type
    auto val = lhs.value() * VT::den / U1::den * U1::num + rhs.val() * VT::den / U2::den * U2::num;
    return Duration<decltype(val), VT>(val);
}
```
- 这里的加法运算是在运行时进行，但是结果的单位类型却是在编译期确定的。
- 这里的`Duration`和`operator+`都是`constexpr`的，如果这里参与计算的值是编译期常量，那么连加法运算都能够在编译期执行。
- 测试：
```C++
template<typename T, typename U>
std::ostream& operator<<(std::ostream& os, const Duration<T, U>& d)
{
    os << d.value() << " " << U::num << "/" << U::den << "s";
    return os;
}

int main(int argc, char const *argv[])
{
    auto x = Duration<int, Ratio<1, 1000>>(13);
    auto y = Duration<int, Ratio<2, 3>>(10);
    std::cout << (x+y) << std::endl; // output: 20039 1/3000s
    return 0;
}
```
- C++标准库中的`<ratio>`和`<chrono>`库提供了时间日期以及编译期分数单位计算功能。

## 反射元编程的维度

在C++11引入`constexpr`前，经常会使用模板递归实例化来驱动值元编程，当然输入就不再是函数参数，而变成了非类型模板参数。
- 在现代C++中，通常来说不必再使用这种技巧，因为会造成二进制膨胀以及编译效率严重下降。能够使用`constexpr`那么`constexpr`就是最佳选择。
- 模板元编程可以有多个维度的选择：
    - 计算：进行编译期计算，`constexpr`和递归模板实例化。
    - 反射：检查程序的特性，比如一些检查成员的类型特征（见 [第十九章-检测成员](../19ImplementingTraits#%E6%A3%80%E6%B5%8B%E6%88%90%E5%91%98)）。目前静态反射仍在提案中，未并入标准。
    - 代码生成：为程序生成额外的代码。前面说过，模板实例化某种意义上也是代码生成，第十七章有相关讨论。

## 模板递归实例化的代价

模板递归实例化的代价是非常大的，[CompileTimeSort.cpp](CompileTimeSort.cpp)中的模板递归展开后接近15000行。

## 计算完整性

- 一个模板元程序可以包含：
    - 状态变量：模板参数。
    - 循环结构：通过递归实现。
    - 执行路径选择：条件表达式或者特化。
    - 整数算术。
- 如果递归实例化的深度无限，那么通过这些能力可以计算出所有能够计算的东西（图灵完备）。但是以这种方式使用模板可能并不是很方便，因为模板实例化需要巨量的编译资源。
- 所以现实中使用模板元编程时应该有节制，不要滥用。
- 特别地，可以用在一些性能敏感的地方压榨出程序的极致性能。

## 递归实例化和递归模板参数

主旨是避免递归实例化时递归出指数爆炸的东西。

## 枚举值与静态常量

- 在早期的C++中，枚举值才是唯一的能够创建常量表达式的机制。所以在早期的模板元编程代码中都是使用枚举值常量（`enum { value = xxx }`）这个技巧。
- C++98引入了类内静态常量初始化，但是依然存在缺陷，`static const`变量依然是一个左值，按引用传递时需要变量地址，会导致编译器实例化模板并给变量分配内存。
- C++11引入`constexpr`后，则解决了这个问题，并且现在能够不限制于整型常量，实现了真正的任意类型编译期常量，具有恰当的类型（而不是枚举类型），并且可以使用`auto`声明经推导得到类型。
- C++17引入`inline`变量。
- 现在的值元编程，普遍用法都已经是`static constexpr`常量在类内初始化，非常方便。不需要再使用早期的技巧。

## 后记

略。
