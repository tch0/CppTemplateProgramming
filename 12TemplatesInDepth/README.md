<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十二章：深入模板基础](#%E7%AC%AC%E5%8D%81%E4%BA%8C%E7%AB%A0%E6%B7%B1%E5%85%A5%E6%A8%A1%E6%9D%BF%E5%9F%BA%E7%A1%80)
  - [参数化声明](#%E5%8F%82%E6%95%B0%E5%8C%96%E5%A3%B0%E6%98%8E)
  - [模板形参](#%E6%A8%A1%E6%9D%BF%E5%BD%A2%E5%8F%82)
  - [模板实参](#%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82)
  - [可变参数模板](#%E5%8F%AF%E5%8F%98%E5%8F%82%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [友元](#%E5%8F%8B%E5%85%83)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十二章：深入模板基础

深入第一部分介绍的一些概念的琐碎细节，模板声明、模板形参的限制、模板实参的限制等。

## 参数化声明

- C++有四种模板：类模板、函数模板、变量模板、别名模板。
- 他们可以出现在命名空间作用域或者类作用域，出现在类作用域中时是：嵌套类模板、成员函数模板、静态成员变量模板、成员别名模板。
```C++
// function template
template<typename T>
void foo()
{
    std::cout << "template<typename T> void foo()" << std::endl;
}

// variable template
template<typename T>
int bar = 1;

// alias template
template<typename T>
using FooInt = Foo<int, T>;

// template declearations in class scope
class Buz
{
public:
    // nested class template
    template<typename T1, typename T2>
    class Foo {};

    // memeber function template
    template<typename T>
    void foo()
    {
        std::cout << "template<typename T> void Buz::foo()" << std::endl;
    }

    // static member variable template
    template<typename T>
    inline static int bar = 1;

    // member alias template
    template<typename T>
    using FooInt = Foo<int, T>;
};
```
- C++17起变量模板（包括全局变量模板、静态成员变量模板）可以`inline`。这也就意味着他们的定义可以出现在不同编译单元中了，因为模板本来就可以定义在不同编译单元，所以这一特性是已经有的。但对于静态成员变量模板来说，`inline`可以省略它的定义，确实是有用的。
- 其中嵌套类模板、成员函数模板、静态成员变量模板都可以类外定义。如果他们本是类模板的成员，那么会需要多个`template<...>`子句。第一个是类模板的，第二个是成员模板的。更多层也是可行的，最开始的是最外层的类模板子句。
- 构造函数模板会禁用默认构造的合成，如果要使用隐式生成的版本，需要显式声明为`=default`。

`union`也可以是模板，他们被视为一种类模板：
```C++
template<typename T>
union AllocChunk
{
    T obj;
    unsigned char bytes[sizeof(T)];
};
```

默认调用实参：
- 函数模板可以有默认调用实参。
- 函数模板默认实参经常依赖于模板参数：
```C++
template<typename T>
void fill(Arrar<T>& arr, const T& = T{});
```
- 如果调用时传递了第二个参数，那么默认实参不会被实例化，以防止`T`不能默认构造导致的错误。

类模板的非模板成员：
- 类模板除了模板成员也可以有非模板成员，他们也是参数化的，所有模板参数都来自于类模板。
- 他们的类外定义仅需要类模板的参数化子句（parameterized clause）。
- 他们不是直接的模板，而是依赖于类模板的，统称为模板化实体（templated entity）。

虚成员函数：
- 成员函数模板不能定义为虚函数，因为他们是参数化的，还没有被实例化就不知道究竟有多少个函数，而虚函数的实现通常都是使用一个固定大小的虚表，每个条目对应一个虚函数，虚函数的数量必须编译时可知。
- 类模板的普通成员函数则可以是虚函数，因为当类模板实例化时他们的数量是固定的。

模板的链接属性：
- 每个模板都必须有一个名称，并且在其作用域中唯一，除了函数模板可以重载之外。特别是类模板不可以和另一个实体共享一个名称（而普通类可以和变量同名）。
- 模板名称可以有链接属性，但不能是以C方式链接。
```C++
extern "C++" template<typename T> void normal(); // this is default and could be omitted
```
- 当然编译器可能会有其他的非标准链接属性，这个不在讨论之列。
- 模板通常都有外部链接属性，仅有的例外是：
    - 命名空间作用域的`static`函数模板，内部链接。
    - 直接或者间接定义在匿名命名空间中的模板，内部链接。
    - 未命名类（匿名类）的成员模板（必须类内定义，因为没有办法类外定义），没有链接属性。
- 当前来说，模板不能定义在函数作用域和局部类作用域。
- 但是泛型lambda可以出现在局部作用域，其关联的闭包类型定义了`operator()`成员模板。
- 函数模板的实例是内部链接属性的。
- 变量模板的实例则是外部链接，即使其类型中含有顶层`const`（而顶层`const`修饰的普通全局变量是内部链接的）。

主模板：
- 模板名称后面没有`<>`的叫做主模板。
- 类模板和变量模板全特化或者偏特化时就是非主模板。

## 模板形参

- 三种模板形参：
    - 类型参数。
    - 非类型参数。
    - 模板模板参数。
- 这三种模板形参都可以用于声明模板参数包。
- 每个模板形参都可以在后续模板形参的声明中出现。
```C++
template<typename T, T Root, template<T> class Buf>
void foo();
```
- 这里`T`只能传入可以作为非类型模板参数的类型，因为后续将其用作了`Root`和模板模板参数的非类型模板参数。
- 模板形参就像一个类型别名一样，在模板内部使用其表示模板参数类型，在内部表示该类型时仅仅使用模板参数名称`T`，而不能是`class T`这种。

非类型模板参数：
- 非类型模板参数必须是编译期或者链接期常量。
- 见[第三章：非类型模板参数](../03NonTypeTemplateParameters)。
- 合法类型：
    - 整型、枚举。
    - 指针：对象和函数指针。
    - 成员指针类型。
    - 左值引用（对象或者函数）。
    - `std::nullptr_t`。
    - 也可以是`auto`或者`decltype(auto)`经由自动类型推导得出。
- 用嵌套从属名称声明非类型模板参数时可以由`typename`开始。
- 函数和数组用于非类型模板形参时会自动退化为指针。`template<int buf[5]>`等价于`template<int* buf>`。
- 顶层CV限定会被忽略，`template<const int N>`等价于`template<int N>`。
- 非引用类型的非类型模板参数总是纯右值，不能被赋值，不能取地址。
- 左值引用类型的非类型模板参数可以被赋值、取地址等。
- 右值引用不能作为非类型模板形参。

模板模板参数：
- 是类模板或者别名模板的占位符。就像类模板一样声明，但不能使用`struct union`关键字。
- C++17起可以使用`typename`代替`class`。这一改变是被别名模板也可以作为模板模板参数的实参这一特性启发的。
- 模板模板参数的形参可以有默认实参，在模板内部使用模板模板参数时如果未指定对应实参则会使用其默认实参。
- 模板模板参数声明中的形参名称在外部的模板声明中是不可见的，仅仅用作声明中的占位符，基本没有任何作用，甚至通常来说都是不写的（即使声明了默认实参）。

模板参数包：
- 模板形参声明前加上`...`即成为可变参数模板。
- 可变类型参数模板可以匹配任意数量的模板类型参数。
- 非类型模板参数和模板模板参数也可以声明参数包，同理。
- 类模板、变量模板、别名模板都只能有一个模板参数包。模板参数包只能放在模板参数列表末尾。
- 函数模板的限制更弱一些：只要模板参数包后的参数有默认模板实参或者能够被推导出，就可以在任意位置声明模板参数包。可以有多个模板参数包，模板参数包后也可以有其他模板参数。
```C++
template<typename... Args, typename Last>
void foo(Last value);

template<typename... Args> class Bar;
template<typename... Args1, typename... Args2>
auto buz(Bar<Args1...>, Bar<Args2...>);
```
- 类模板与变量模板的偏特化可以有多个模板参数包，因为偏特化的选择是类似于函数模板推导的过程。
```C++
template<typename... Args> class TypeList;
template<typename X, typename Y> class Zip;
template<typename... Xs, typename... Ys, typename T>
class Zip<TypeList<Xs...>, TypeList<Ys..., T>>;
```
- 注意模板类型参数包不能用于同一个模板参数列表中后续非类型模板参数的声明：
```C++
template<typename... Ts, Ts... vals> class WhatEver; // ERROR
```
- 但是在不同模板参数列表是可以这样做：
```C++
template<typename... ts>
struct Outter
{
    template<Ts... Values> struct Inner {};
};
```
- 包含参数包的模板被称为可变参数模板。

默认模板实参：
- 除了模板参数包之外所有模板形参都可以有默认实参。
- 所有类别的模板参数都可以有默认模板实参，默认模板实参的类别要和形参一致（比如非类型模板形参的默认实参不能是一个类型。）
- 默认模板实参不能依赖于它对应的形参，因为直到默认模板实参定义完成后该形参名称才被加入到作用域中。比如`template<typename T = Base<T>>`就是非法的。
- 但是可以依赖于前面已经声明了的模板形参。
- 对于类模板、变量模板、别名模板，默认模板实参只能从后往前添加，和函数默认实参类似。
```C++
template<typename T1, typename T2, typename T3 = char> struct Foo;
template<typename T1, typename T2 = char, typename T3> struct Foo; // valid: T3 already has a default template argument, can not change order with previous row
template<typename T1, typename T2, typename T3 = char> struct Foo; // ERROR: default template argument cannot be repeated
```
- 函数模板则不要求一个具有默认实参的模板参数的后续所有模板参数都具有默认实参。因为可以靠推导而来。
- 多个声明中默认模板实参不能重复。并且因为对类模板要从后往前添加，所以类模板、变量模板、别名模板只能在第一次出现模板声明的时候就添加模板实参，当然也可以在多个声明中依次从后往前添加，但是不能重复，不能跳过某一个。
- 一些场景中不能添加默认模板实参：
    - 偏特化。
    - 参数包。
    - 类外定义。
    - 友元模板声明。
- 友元模板声明如果同时是定义并且编译单元其他地方没有声明才可以添加默认实参。

## 模板实参

当模板被实例化时，模板形参被模板实参替换，模板实参被以下几种机制决定：
- 显式传入的模板实参，模板名称加上角括号包围起来的模板实参列表称之为template-id。
- 注入类名：在类模板`X`作用域中，对于模板实参`P1, P2, ...`，名称`X`等价于`X<P1, P2, ...>`。
- 默认模板实参：模板实例化时显式模板参数可以省略，如果有默认实参的话。注意，就算所有模板形参都有默认实参，也必须写出空的角括号`<>`。
- 模板实参推导：函数模板中没有被显式指定的实参可以经由函数实参推导而来。如果所有模板实参都有默认参数或者经由推导而来，那么可以不写角括号。C++17起，类模板参数也可以推导。

函数模板实参：
- 可来自显式指定、默认实参或者推导。
- 如果模板形参没有出现在函数参数列表中，那么该模板实参无法被推导出来。通常这种参数被放在模板形参列表开头，以便剩余的能够被推导出来从而不用显式指定。
- 这样的参数不能放在参数包的后面。
- 因为函数模板能够重载，显式指定所有模板实参可能并不足以确定哪一个重载，某些时候，这个template-id是一个函数集合。此时因为不唯一，也就不能取地址或者转化为函数指针。
```C++
template<typename T> void foo(T) {}
template<typename T>
void bar(T) // 1
{
    std::cout << "version 1" << std::endl;
}
template<typename T>
void bar(T*) // 2
{
    std::cout << "version 2" << std::endl;
}

int main(int argc, char const *argv[])
{
    auto pfoo = &foo<int>;
    // auto pbar = &bar<int>; // ambiguous
    void (*pbar1)(int);
    pbar1 = &bar<int>; // valid, pbar1 is version 1
    pbar1(1);
    void (*pbar2)(int*);
    pbar2 = &bar<int>; // valid, pbar2 is version 2
    pbar2(nullptr);
    return 0;
}
```
- 当然如果一个函数模板`f<int>`只有一个匹配的函数模板是合法的，那么也不会产生二义。这就是前面讨论过的SFINAE原则。

类型实参：
- 任意类型都能够作为类型实参。
- 但是在替换后需要使得模板是合法的，模板中没有不支持的非法操作，否则会报错。

非类型模板实参：
- 非类型模板实参是下列事物之一：
    - 另一个具有正确类型的非类型模板实参。
    - 编译期整型或者枚举常量。
    - 一个外部链接的变量的地址（`&`取地址），匹配指针类型非类型模板参数。C++17放宽了要求，任意产生指针类型的编译期常量表达式均可。
    - 一个成员指针常量，`&C::m`形式，任意类型匹配的非静态成员指针均可。C++17放宽了要求，产生成员指针常量的编译期常量表达式均可。
    - 空指针常量`nullptr`是成员指针类型的非类型模板参数的合法值。
- 对于最常见的整型非类型模板参数，会考虑隐式类型转换。C++11引入了`constexpr`，就是说从编译期常量的类对象转化为整型也会被考虑。
- 当非类型模板参数是指针或者引用时，C++17前，不会考虑用户定义转换和派生类到基类转换，C++17之后好像也不考虑。
- 增加参数的CV限定的隐式转化是合法的。
- 对于非类型模板参数来说，一般的限制的=是编译器或者链接器需要能够在程序建立起来时（运行前）就知道这个非类型模板实参的值。也就是必须是编译期或者链接期常量。运行时才能知道的值则不能（比如局部变量的地址）。
- 有一些常量当前还不能用于非类型模板实参：
    - 浮点数。
    - 字符串字面量。
- 字符串字面量的问题是，两个相同的字符串字面量可能被存储在两个不同的地址。
- 一个比较繁琐的方式是使用一个全局变量保存字符串字面量，这样便能够用于非类型模板实参了。
- 通常来说整数外的非类型模板参数用得比较少。

模板模板实参：
- 可以是完全匹配模板模板形参声明的类模板或者别名模板。
- 模板模板参数也可以有默认模板模板实参。
- C++17之后，模板模板实参的默认参数也会被考虑进来，不需要模板模板实参与模板模板形参的模板形参列表完全匹配，只需要模板模板实参能够按照模板模板形参中的形参列表来使用即可。
- 模板参数包都是同一个种类（kind）的：即要么都是类型参数、非类型参数、模板模板参数。
- 模板参数包实例化时如果混入了不同种类的实参，那么就会失败。
- 虽然模板模板形参是用`class`（C++17起`typename`也可以）来声明的，但是类模板、别名模板、`struct`模板、`union`模板都是可以作为实参的。

实参的等价性：
- 两组模板实参比较，如果一一对应等价，则两组实参等价。
- 类型别名不会对比较造成干扰。
- 整数类型比较时，与其怎样表达的无关（`2*2`和`1+3`等价）。
- 在依赖于模板的环境中，如果模板实参的值不能被明确地建立起来，那么等价判断的结果可能会比较微妙：
```C++
template<in N> struct I {};
template<int M, int N> void f<I<M+N>>; // 1
template<int N, int M> void f<I<N+M>>; // 2
template<int M, int N> void f<I<N+M>>; // 3, ERROR
```
- 版本1和2是等价的同一个函数模板的两次声明，而3和1或者2在形式上则不是等价的。
- 但是当调用`f<1, 2>`时，1和3都同样匹配，他们是功能等价的。但是编译器不会将这两个声明认为是同一个。需要非常小心地处理这种情况。
- 一个模板实例化得到函数永远不会跟一个普通函数等价，即使他们有相同的参数列表和名称（经过名称修饰后名称是不一样的）。
- 这一点引出了两个重要的结论：
    - 一个函数模板生成的成员函数永远不会是另一个虚函数的重写（override）。
    - 一个函数模板生成的构造函数永远不会是一个拷贝或者移动构造函数，赋值运算符同理。

## 可变参数模板

## 友元

## 后记
