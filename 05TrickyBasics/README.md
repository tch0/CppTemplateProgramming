<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第五章：模板基础技术](#%E7%AC%AC%E4%BA%94%E7%AB%A0%E6%A8%A1%E6%9D%BF%E5%9F%BA%E7%A1%80%E6%8A%80%E6%9C%AF)
  - [typename关键字](#typename%E5%85%B3%E9%94%AE%E5%AD%97)
  - [零初始化](#%E9%9B%B6%E5%88%9D%E5%A7%8B%E5%8C%96)
  - [使用this->](#%E4%BD%BF%E7%94%A8this-)
  - [为内建数组和字符串字面值重载或特化](#%E4%B8%BA%E5%86%85%E5%BB%BA%E6%95%B0%E7%BB%84%E5%92%8C%E5%AD%97%E7%AC%A6%E4%B8%B2%E5%AD%97%E9%9D%A2%E5%80%BC%E9%87%8D%E8%BD%BD%E6%88%96%E7%89%B9%E5%8C%96)
  - [成员模板](#%E6%88%90%E5%91%98%E6%A8%A1%E6%9D%BF)
  - [变量模板](#%E5%8F%98%E9%87%8F%E6%A8%A1%E6%9D%BF)
  - [模板模板参数](#%E6%A8%A1%E6%9D%BF%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第五章：模板基础技术

本章讨论一些模板实践相关的基础技术。

## typename关键字

- `typename`关键字是起初为了说明模板中的使用到的嵌套从属名称（nested dependent names）是类型而引入的。
- 某些场景下模板无法解析一个嵌套从属名称到底是类型还是一个非类型成员，所以一律当做非类型成员处理，如果是类型则需要通过`typename`显式指出。
- 通常来说，`typename`应该被用在依赖于模板参数的嵌套从属类型上。
- 但有特殊情况，某些时候虽然是嵌套从属类型但不能使用：基类列表中、和构造函数成员初始化列表中初始化基类时不能使用。
- 当`typename`用在模板形参列表中时，与`class`含义相同。
- C++20放宽了这个限制，某些场景中能够推导出嵌套从属类型是一个类型可能不再需要添加。但加上总是没有问题的。

## 零初始化

- 因为基础类型比如`int double`没有默认构造函数，当在模板中需要将他们进行默认值初始化时，就不能像自定义类型一样使用`T x;`。
- 所以为了让自定义类型和内置类型都能够初始化为合适的默认值，就需要使用`T x{};`，对于自定义类型执行默认初始化，内置类型则进行零初始化。
- 在C++之前，还不能这样用，可以用`T x = T();`代替。C++17之前，这种方式只能用于拷贝构造不是explicit的情况，C++17之后一定会进行复制消除，便没有了这个限制。
- 在类模板中初始化模板参数类型的成员时（构造函数初始化列表中），同样可以使用`x{}`和`x()`。
    - C++11之后，还可以类内初始化，直接定义时`T x{};`。
- 注意默认参数是不能这样初始化的，可以使用`f(T t = T{})`或者`f(T t = T())`。

## 使用this->

对于类模板的基类也依赖模板参数的情况，如果使用基类的成员（数据或函数）。派生类是不知道基类针对特定类型是否做了特化，是否还有这个成员。不一定所有对基类成员的引用都是使用的预期中的基类成员，比如还可能引用的是一个同名的全局名称，解决方法：
```C++
class Foo
{
public:
    void bar() {}
};

template<typename T>
class DerivedFoo : public Foo<T>
{
public:
    using Foo<T>::bar; // solution 1
    void derivedBar()
    {
        bar(); // invalid without using declaration, there are no arguments to 'bar' that depend on a template parameter, so a declaration of 'bar' must be available
        this->bar(); // solution 2
        Foo<T>::bar(); // solution 3
    }
};
```
- 在基类函数调用前加上`this->`。
- 使用`using`声明使基类名称可见（推荐做法）。
- 显式使用作用域运算符指定使用基类函数。
    - 这样会导致不支持多态，如果是派生类非虚函数中调用基类虚函数的话不推荐这样做。
    - 当然如果是在重写的虚函数中调用基类实现，那么这就是标准做法。
- 做法1和2是标准做法。做法3通常来说只应用在重写的虚函数中。

## 为内建数组和字符串字面值重载或特化

当数组作为函数参数时，如果使用引用传递，则推导时不会退化，如果使用值传递则会退化为指针。所以数组作为参数的行为会根据函数声明而不同，并且字符串字面量本质上也是字符数组，因为末尾有空字符，和普通数组亦有区别。在一个实现中统一这些情况很多时候不太现实：
- 所以如果数组或者字符串字面量可以作为函数参数时，通常会选择针对数组和字符串字面量做特化。从而在其中实现不同的处理逻辑，很多库函数都有这样的处理。并且通常是使用引用传递，以避免数组退化。
- 典型实现手段：
```C++
template<typename T, int N, int M>
bool less(T(&a)[N], T(&b)[M])
{
    for (int i = 0; i < N && i < M; ++i)
    {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return N < M;
}
```
- 在类模板也会有类似处理，可以针对数组对类模板做偏特化，具体实现时可以有许多类型的偏特化：
    - 已知大小数组。
    - 已知大小数组的引用。
    - 未知大小数组。
    - 未知大小数组引用。
    - 指针。
- 未知数组大小的情况是，`extern int x[];`但是定义还在后面或者其他编译单元中的数组。这语法属实有点太生僻了。
- 通常来说针对已知大小数组即可。

## 成员模板

类成员同样可以是模板：
- 首先，还是要看文档：
    - [成员模板](https://zh.cppreference.com/w/cpp/language/member_template)
    - [模板全特化](https://zh.cppreference.com/w/cpp/language/template_specialization)
    - [模板偏特化](https://zh.cppreference.com/w/cpp/language/partial_specialization)
- 这个成员可以是嵌套类成员，也可以是成员函数。

成员模板函数：
- 通过定义成员模板函数，最常见的用途就是可以将不同模板参数的模板类相互操作，只要这个操作是合法的，比如将`Stack<int>`赋给`Stack<double>`。
- 类外实现需要将两个模板参数列表依次写出来。
- 当然普通类也是可以定义成员模板函数的。
- 需要注意的是，两个使用不同模板参数的同一模板类不是同一类型，在定义中也是这样，所以通常这种函数只能访问公有接口。
- 如果需要能够访问私有成员，可以将不同模板参数的同一模板类定义为友元。
```C++
template<typename T>
class A
{
    template<typename>
    friend class A; // define self with other template arguments as friend
    ...
}
```
- 其中因为模板参数未被使用，就像函数参数一样，可以不将其写出来。

成员模板函数的特化：
- 成员函数模板可以被全特化。
- 成员函数模板的全特化只需要定义而不需要声明，并且只能在类外做。
- 可以对全特化的模板类定义成员模板，也就是不写出完整的类定义。此时其类定义是根据非特化版本的类模板声明来做的，所以第一个类模板的模板参数列表`template<>`必须写。
- 也可以对全特化的模板类仅特化成员模板，不写出特化模板类的定义。此时将会有两个空模板参数列表`template<> template<>`。
- 对全特化的模板类特化成员模板（全特化模板类定义已写出），那么特化模板类时属于类的第一个模板参数列表`template<>`就不写（因为写在了特化类模板时，特化的类模板就是一个普通类）。
- 总体的原则是：不能在非全特化的模板里面搞出全特化的东西。也就是说不能对一个模板类的成员模板做全特化。对一个全特化的类模板或者普通类的成员模板函数是可以做全特化的。
- 例子：[P074.MemeberTemplate.cpp](P074.MemeberTemplate.cpp)

特殊成员函数模板：
- 拷贝构造、赋值运算符也可以是成员函数模板。但是他们不会抑制编译器合成版本的生成。
- 因为他们是模板，不调用就不会生成代码。
- 也就是说模板版本的构造函数和拷贝控制成员仅仅是为了其他模板参数类型准备的。相同的模板参数则绝不会调用这个函数。

`.template`构造：
- 调用类模板的成员模板可以使用`.template`后在接其成员函数模板。
```C++
foob.template print<bool, bool>(); 
(&foob)->template print<bool, bool>();
```
- 在前面调用对象是模板参数相关的无法分辨这是一个普通函数还是模板函数的情况下，就必须白表明这是一个模板。不然可能无法分辨后面的`<>`是操作符还是代表模板参数的角括号。

泛型Lambda和成员模板：
- C++14引入了泛型模板，详见[Lambda表达式](https://zh.cppreference.com/w/cpp/language/lambda)。
- 泛型lambda可以显式使用模板，也可以使用`auto`则等价于隐式的模板定义。
- 泛型lambda就等价于函数对象定义了泛型的`operator()`运算符。是定义成员模板的一个快捷方式。
```C++
[](auto x, auto y) { return x + y; }
// equivalent to an specific implicitly defined function object by compiler
class SomeRandomClass
{
public:
    template<typename T1, typename T2>
    auto operator()(T1 x, T2 y) const {
        return x + y;
    }
};
```

## 变量模板

C++14起，变量也可以针对特定类型做参数化，见[变量模板](https://zh.cppreference.com/w/cpp/language/variable_template)。格式：
```C++
template<tempalte-parameter-list>
variable-declaration;
```
- 除非显式特化或者显式实例化，变量模板只在该变量求值时才隐式实例化。
- 还有变量不存在会影响语义时也会隐式实例化。
- 在C++14前，为实现变量模板类似效果，通常会使用类模板的静态数据成员，或者返回所需值的`constexpr`函数模板。
- 在类作用域中使用时，变量模板声明一个静态数据成员模板。
- 静态数据成员模板也需要一个定义，这种定义可以在类定义外提供.处于命名空间作用域的静态数据成员的模板声明可以是类模板的非模板静态数据成员的定义。
```C++
template<class T>
T X<T>::s = 0;
```
- 变量模板也可以有默认模板参数，使用默认模板参数时模板实参列表可以空着，但是不能不写。

更多细节：
- 变量模板可以被特化，也可以被偏特化。
- 如果将静态成员变量定义为了`inline`，那么就不能再做定义（此时声明就是定义）。静态成员变量模板也是一个道理。
- 变量和变量模板都必须有定义。
- `inline`变量声明（即是定义）时做了初始化，就不能再特化。如果特化时做了初始化，会有重复初始化问题，不做初始化则会链接错误（为什么？）。
- `const`静态成员变量可以不需要类外定义，但此时必须进行类内初始化才会将类内声明作为定义，如果没有类内初始化，那么必须类外定义并初始化。
- `constexpr`静态成员变量会默认`inline`，必须类内初始化。不需要在类外再定义（虽然好像写了也是合法的）。
- 注意普通`inline`静态变量在类外定义是会报重定义的。
- 变量模板也是模板，同普通模板一样，不需要`inline`也不会链接时符号重定义。
- 变量模板的全特化就是定义，放在头文件中时需要`inline`。
- 编译期变量模板非常常用，C++17起，标准库中的所有编译期得到布尔值的类型特性，都有一个`_v`后缀的变量模板对应。比如：`std::is_const_v<xxx>`等。实现类似于：
```C++
template<typename T>
inline constexpr bool is_const_v = std::is_const<T>::value;
```

## 模板模板参数

模板模板参数允许我们将类模板本身作为模板参数，语法：
```C++
template<template<typename xxx...> class ArgClassTemplate = defaultClassTemplate>
template-definition;
```
- C++17起，可以使用`typename`代替`class`关键字，表达完全相同的含义。
```C++
template<template<typename xxx...> typename ArgClassTemplate = defaultClassTemplate>
template-definition;
```
- C++11起，除了类模板，还可以使用匹配的别名模板作为模板模板参数的实参。
- 因为模板模板参数声明中的模板参数没有被使用，习惯上是不写出名称。
- 模板模板参数中也是可以有默认模板参数。
- 模板模板参数是类模板或者别名模板的一个占位符。但是在语法上没有函数模板或者变量模板的占位符。
- C++17之前，模板模板参数实参的默认参数不被考虑，必须要和模板模板形参完全匹配。C++17后，则会得到考虑。

## 总结

- 依赖于模板参数的嵌套从属名称需要使用`typename`。
- 获取依赖于模板参数的基类成员，需要使用`this->`。
- 嵌套类和成员函数也可以是模板。
- 模板版本的构造、拷贝构造和赋值运算符不会覆盖隐式生成的版本。
- 使用空的列表初始化或者显式调用默认构造以在模板内部进行零初始化，以统一内建类型和自定义类型。
- 某些时候可以为数组、字符串字面量提供模板特化。
- 传递数组或者字符串字面量到函数模板，如果是值传递则会退化。
- 可以定义变量模板。
- 可以使用类模板作为模板参数，称之为模板模板参数。
