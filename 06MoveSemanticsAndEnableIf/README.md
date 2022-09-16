<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第六章：移动语义与enable_if<>](#%E7%AC%AC%E5%85%AD%E7%AB%A0%E7%A7%BB%E5%8A%A8%E8%AF%AD%E4%B9%89%E4%B8%8Eenable_if)
  - [完美转发](#%E5%AE%8C%E7%BE%8E%E8%BD%AC%E5%8F%91)
  - [特殊的成员函数模板](#%E7%89%B9%E6%AE%8A%E7%9A%84%E6%88%90%E5%91%98%E5%87%BD%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [使用enable_if<>禁用模板](#%E4%BD%BF%E7%94%A8enable_if%E7%A6%81%E7%94%A8%E6%A8%A1%E6%9D%BF)
  - [使用enable_if<>](#%E4%BD%BF%E7%94%A8enable_if)
  - [使用concept简化enable_if表达式](#%E4%BD%BF%E7%94%A8concept%E7%AE%80%E5%8C%96enable_if%E8%A1%A8%E8%BE%BE%E5%BC%8F)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第六章：移动语义与enable_if<>

C++11引入的最著名的特性莫过于移动语义，通过移动大幅优化了拷贝和赋值大型对象的开销。这同时影响了泛型代码的设计，所以C++11引入了一系列规则来支持泛型代码中的移动语义。

## 完美转发

为了转发传入函数的参数的基本属性：
- 可修改对象转发之后依然可修改。
- 常量对象转发之后依然是只读的。
- 可移动对象转发之后依然是可移动的。

所以现在我们需要对这三种情况进行编码：
```C++
void f(X&);
void f(const X&);
void f(X&&);
// forward to f
void g(X& val)
{
    f(val);
}
void g(const X& val)
{
    f(val);
}
void g(X&& va)
{
    f(std::move(val));
}
```
- 移动语义时转发的代码有一点不同，需要使用`std::move()`。因为移动语义不会自动传递，右值引用本身是一个左值。
- 为了在泛型代码中将这三种情况组合起来，就必须处理移动语义的问题。
- C++11引入了完美转发`std::forward<T>(val)`，这个标准库模板函数针对左值和右值分别做了特化，统一了这两种情况，libstdc++实现：
```C++
template<typename _Tp> _GLIBCXX_NODISCARD constexpr _Tp&& forward(typename std::remove_reference<_Tp>::type& __t) noexcept
{
    return static_cast<_Tp&&>(__t);
}
template<typename _Tp> constexpr _Tp& forward(typename std::remove_reference<_Tp>::type&& __t) noexcept
{
    static_assert(!std::is_lvalue_reference<_Tp>::value,
        "std::forward must not be used to convert an rvalue to an lvalue");
    return static_cast<_Tp&&>(__t);
}
```
- 但在本质上他们做一样的事情，并且都是用了万能引用。

万能引用：
- 当函数模板的参数是模板参数的右值引用时，含义不再是右值引用，而是万能引用（universal reference，或者叫转发引用，forwarding reference，C++17的新叫法）。
- 万能引用会根据传入参数的值类别推导出参数类型，如果传入左值会推导为左值引用（CV限定不变），如果传入右值则推导为右值引用。
- 然后经过一个特殊的**引用折叠**过程得到最终推导出的类型。
```
&& + && -> &&
&& + & -> &
& + && -> &
& + & -> &
```
- 所以使用万能引用能够接收左值和右值并且能够传递其值类别。
- `std::forward`的实现仅仅是做转换而已，本质上其实就是单纯利用引用折叠保持参数的值类别。
- 和可变参数模板结合可以完美转发任意类型任意数量的参数，前面介绍过。
- 注意参数类型一定要是模板类型，而不能仅仅是与模板参数相关。比如模板参数是`T`，而函数参数类型是`T::iterator&&`，那么这仅仅是一个普通的右值引用。
- 重写上面的代码：
```C++
void f(X&);
void f(const X&);
void f(X&&);
// forward to f
template<typename T>
void g(T&& val)
{
    f(std::forward<T>(val));
}
```

## 特殊的成员函数模板

考虑一个场景，我们有一个类：
```C++
class Person
{
private:
    std::string name;
public:
    Person(const std::string& _name) : name(_name) // 1
    {
        std::cout << "Person(const std::string& _name)" << std::endl;
    }
    Person(std::string&& _name) : name(std::move(_name)) // 2
    {
        std::cout << "Person(std::string&& _name)" << std::endl;
    }
    Person(const Person& other) : name(other.name) // 3
    {
        std::cout << "Person(const Person& other)" << std::endl;
    }
    Person(Person&& other) : name(std::move(other.name)) // 4
    {
        std::cout << "Person(Person&& other)" << std::endl;
    }
};
```
- 其中拷贝构造和移动构造是没有办法模板化的，因为模板不会抑制隐式生成版本的合成，最终还是会调用隐式生成版本。但版本1和2是完全可以统一起来的。
```C++
class Person
{
private:
    std::string name;
public:
    template<typename T>
    Person(T&& _name) : name(std::forward<T>(_name)) // 1
    {
        std::cout << "template<typename T> Person(T&& _name)" << std::endl;
    }
    Person(const Person& other) : name(other.name) // 2
    {
        std::cout << "Person(const Person& other)" << std::endl;
    }
    Person(Person&& other) : name(std::move(other.name)) // 3
    {
        std::cout << "Person(Person&& other)" << std::endl;
    }
};
```
- 更改之后出现了一个致命问题：当使用非const的`Person`类型对象拷贝构造另一个对象时，版本1比版本2更加匹配，导致编译错误。
- 所以此时需要一种能够在某种情况下为某些类型或者某些条件禁用某个函数模板重载参与重载决议的能力。而提供这种能力的就是`enable_if`。

## 使用enable_if<>禁用模板

libstdc++中的实现是这样的：
```C++
template<bool, typename _Tp = void>
struct enable_if { };
// Partial specialization for true.
template<typename _Tp>
struct enable_if<true, _Tp>
{ typedef _Tp type; };
```
- 也就是在第一个模板参数为`true`时定义内部类型为第二个模板参数，默认是`void`，为`false`那就没有定义。
- 然后他还有一个别名模板：
```C++
template<bool _Cond, typename _Tp = void>
using enable_if_t = typename enable_if<_Cond, _Tp>::type;
```
- 当值为`false`因为SFINAE（Subtitution failure if not a error，替换失败并非错误）的机制，该模板是非法的，不会参数重载决议。而为`true`时则替换为对应类型，参与重载决议。
- 将一个编译期表达式作为条件填入`std::enable_if`的第一个模板参数即可实现特定条件下模板才有效的机制。
- 最典型的使用方式是在模板形参末尾添加一个不命名的默认参数：
```C++
template<typename T, typename = std::enable_if_t<condition>>
void foo();
```
- 可以将特定条件定义为别名模板以简化和复用。

## 使用enable_if<>

使用`enable_if`解决上面提到的例子：
- 增加一个条件，模板参数是`std::string`或者能够转换为`std::string`才参与重载决议。
```C++
template<typename T,
    typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
Person2(T&& _name) : name(std::forward<T>(_name)) // 1
{
    std::cout << "template<typename T> Person2(T&& _name)" << std::endl;
}
```
- `std::is_convertiblb<T1, T2>`是前者能够隐式转换为后者的特性。
- 如果要能够显式转换也满足要求则需要使用`std::is_constructible<T1, T2...>`，含义是使用参数列表`T2...`能够构造前者。

将拷贝构造统一到模板版本：
- 有一个非常取巧的技巧可以将使用拷贝控制成员统一到模板版本中。
- 就是将拷贝构造声明为`const volatile Type&`并声明为`=delete`。
- 这样所有非`const volatile`的该类型对象都会优先匹配万能引用版本（而一般情况我们不会使用`volatile`）。
- 例子，如果模板参数是整数类型则禁用拷贝：
```C++
template<typename T>
class C
{
public:
    C(const volatile C&) = delete;
    template<typename U, typename = std::enable_if_t<!std::is_integral<U>::value>>
    C(const C<U>&)
    {
        ...
    }
};
```
- 这只是一个技巧，通常情况下还是不要这么用的好，直接显式定义出来是最好的。

## 使用concept简化enable_if表达式

使用`enable_if`是能够达到目的的，但本质上这是一种对模板的滥用，这样做可能会让模板变得难以理解。更好的方式是在C++语言中引入一种机制来规定函数模板对于模板参数的要求，不满足直接忽略这个模板。这就是C++20引入的[概念](https://zh.cppreference.com/w/cpp/language/constraints)（concepts）所做的事情：
- 这是C++20引入的核心语言特性。
- 类模板、函数模板、以及非模板函数（通常是类模板成员）可以关联到约束（constraint），它指定了对模板实参的一些要求，这些要求用于选择最恰当的函数重载和模板特化。
- 这种要求的具名集合称为概念（concept），每个概念都是一个谓词，编译期求值，并在自己被用作约束时成为模板接口的一部分。
- 概念和约束内容比较多，在[附录E](../AppendixE)中详述。

## 总结

- 通过万能引用/转发引用配合`std::forward`可以完美转发参数。
- 当使用完美转发成员函数模板，他们可能比预定义的特殊成员函数（构造、拷贝移动控制成员）更加匹配，这时就需要使用`enable_if`或者概念以在某些情况下禁用模板。
- 通过使用`enable_if`，可以在某个编译期表达式为`false`时禁用函数模板。
- 可以通过将拷贝控制成员声明为`const volatile&`并且声明为`delete`来实现特殊成员函数均调用模板的效果。
- 概念为函数模板提供了比`enable_if`更加符合直觉的语法。
