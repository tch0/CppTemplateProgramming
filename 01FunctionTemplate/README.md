<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第一章 函数模板](#%E7%AC%AC%E4%B8%80%E7%AB%A0-%E5%87%BD%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [概览](#%E6%A6%82%E8%A7%88)
  - [模板参数推导](#%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0%E6%8E%A8%E5%AF%BC)
  - [多个模板参数](#%E5%A4%9A%E4%B8%AA%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0)
  - [默认模板实参](#%E9%BB%98%E8%AE%A4%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82)
  - [重载函数模板](#%E9%87%8D%E8%BD%BD%E5%87%BD%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [一些问题](#%E4%B8%80%E4%BA%9B%E9%97%AE%E9%A2%98)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第一章 函数模板

## 概览

两阶段检查：
- 编译器见到函数模板定义时会进行第一次语法检查（某些编译器可能不会把所有能做的都做到，而是留到实例化才检查）：
    - 语法错误检查，比如缺失分号。
    - 检查使用到的模板参数无关的名称。
    - 检查模板参数无关的静态断言。
- 实例化时才进行最主要的第二次语法检查：
    - 所有东西，特别是依赖于模板参数的东西都会被检查。

## 模板参数推导

当模板参数被用在函数参数中时，编译器会根据调用时传入的实参对模板类型参数进行推导，从而无需传入模板类型实参，将函数模板像普通函数一样使用：
- 在模板类型参数推导期间，隐式类型转换不会被执行。
- 两个使用相同模板参数类型的函数参数必须完全一致，而不能有转换。
- 当将函数参数定义为值传递时，只能够做一些特定的转换（也就是只会做类型退化）：
    - 忽略顶层const和volatile。
    - 引用类型转换为其引用的类型。
    - 数组退化为指针。
    - 两个同样的使用了模板类型参数的函数参数，传入的实参退化后的类型则必须完全匹配。
- 如果两个相同的使用了模板类型参数的函数参数，调用时传入的实参类型不同，可以的解决方法是：
    - 对其中一个做显式类型转换，使其类型一致。
    - 显式指定函数模板参数以阻止编译器进行模板类型参数推导。
    - 将函数参数类型指定为不同的模板类型参数。
- 注意模板参数类型推导不会应用在默认参数上。为了能够使用默认参数，应该同时为模板类型形参声明一个默认类型实参以避免推导。
    ```C++
    template<typename T = int>
    void func(T = 1)
    {

    }
    func(); // equivalent to func<int>(1);
    ```

## 多个模板参数

函数模板有两套不同的参数：
- 模板参数，声明在尖括号中，可以有多个。
- 调用参数，声明在小括号中，同样可以有多个。

如果使用了多个模板参数，但返回值为其中一个，在某些例子中可能会有问题，如：
```C++
template<typename T1, typename T2>
T1 max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
- 当调用的参数类型不同时，可能会引入不必要的转换，导致同一个意图，但是参数交换一下就会得到不一样的结果。
- 要解决这个问题可以：
    - 声明第三个模板参数作为返回值类型。
    - 让编译器推导返回值类型。
    - 声明返回值类型为两个类型的公共类型（common type）。

声明返回值类型：
- 最好的做法是：
```C++
template<typename RT, typename T1, typename T2>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
int a = ::max<double>(1, 2);
```
- 为模板类型参数RT传入模板类型实参，剩余两个类型由推导得出，和默认参数很类似，由推导得出的类型应该写在最后面。调用时必须至少指明至最后一个不能经由推导得出的模板参数。
- 当然就这个例子而言，最简单的还是只声明一个类型参数，返回值同样类型，当传入不同类型参数时，显式指定类型参数，对调用实参做转换即可。

推导返回值类型：
- 普通方法是没有办法推导函数返回值类型的。
- 但自C++14起，将返回值声明为`auto`可以让编译器自行推导。
- 将返回值声明为`auto`但是不加尾置返回值声明的意思就是让编译器通过函数中返回语句推导返回值类型。
- 因此，函数中的多个返回语句必须匹配（返回类型推导出来是一致的）。
```C++
template<typename T1, typename T2>
auto max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
- C++14以前，自C++11起，我们也可以使用尾置返回值加上`decltype`来做到类似的推导。
```C++
template<typename T1, typename T2>
auto max(T1 a, T2 b) -> decltype(b < a ? a : b)
{
    return b < a ? a : b;
}
```
- 但是`decltype`存在一个问题，当传入的参数是引用时会推导出引用类型。可以使用`std::decay`解决。
```C++
template<typename T1, typename T2>
auto max(T1 a, T2 b) -> std::decay_t<decltype(b < a ? a : b)>
{
    return b < a ? a : b;
}
```
- `auto`类型的初始化会自动**退化**，所以不需要手动写`std::decay`。返回的过程就类似于初始化，所以也会自动退化：
```C++
int a = 1;
int& ra = a;
auto b = ra; // b is an int, not int&
```

顺便一提，类型退化（[decay](https://zh.cppreference.com/w/cpp/types/decay)）做的事情：
- 首先去掉引用。
- 如果是数组，退化为指针。
- 如果是函数，退化为函数指针。
- 剩余情况，退化后会去掉顶层CV限定。
- `std::decay`的典型实现：
```C++
template< class T >
struct decay {
private:
    typedef typename std::remove_reference<T>::type U;
public:
    typedef typename std::conditional< 
        std::is_array<U>::value,
        typename std::remove_extent<U>::type*,
        typename std::conditional< 
            std::is_function<U>::value,
            typename std::add_pointer<U>::type,
            typename std::remove_cv<U>::type
        >::type
    >::type type;
};
```
- 退化也属于一种隐式转换：数组到指针、函数到指针、左值到右值。退化是隐式类型转换的子集，某些语法中仅支持退化（比如这里的`auto`还有模板参数推导）而不支持普遍的隐式类型转换。

返回公共类型：
- C++11起，可以使用`std::common_type<>::type`选择两个类型中更通用/泛化的那个类型（即公共类型）。
- 所以上述例子也可以这样写：
```C++
#include <type_traits>
template<typename T1, typename T2>
std::common_type_t<T1, T2d> max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
- 注意C++14支持变量模板后才可以使用`std::common_type_t<>`代替`std::common_type<>::type`。
- `std::common_type<>`实现比较复杂，后续（第26章）讨论，传入`int`和`double`会得到`double`。
- `std::common_type<>`的结果同样会退化。

## 默认模板实参

上面的例子同样可以用默认模板实参表达：
- 使用`decltype`：
```C++
template<typename T1, typename T2, typename RT = std::decay_t<decltype(true ? T1() : T2())>>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
- 也可以使用`std::common_type<>`：
```C++
template<typename T1, typename T2, typename RT = std::common_type_t<T1, T2>>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
- 在未显式指定返回值类型时则会使用默认模板实参。

当然总结起来，最简单的方法当然是使用`auto`让编译器去推导。

## 重载函数模板

函数模板可以与普通函数重载：
- 当普通函数和函数模板同样程度匹配函数调用时，优先选择普通函数版本。
- 如果函数模板可以实例化出一个更好的匹配，那么优先选择函数模板版本。
- 如果显式指定了一个模板实参列表（即使为空），那么仅会选择函数模板版本。
- 如果两个重载的函数模板同样好地匹配的一个函数调用，那么将会产生二义性。
- 两个重载的模板，如果其中一个是类型`T`，另一个是`T*`，那么对于指针，第二个将会更好地匹配。
- 注意在使用一个函数或者函数模板时该函数/函数模板声明一定要可见，重载一定是当前可见的版本中最好匹配的那一个。即使后面有一个更好匹配的版本。


## 一些问题

传值还是引用：
- 通常情况下，定义普通函数时，我们会更多地使用引用传递。
- 但是在定义函数模板时，很多时候值传递会更好，原因如下：
    - 语法更简单。
    - 编译器更好优化。
    - 移动语义使得拷贝开销更小。
    - 很多时候甚至不存在拷贝或者移动（复制消除）。
- 在函数模板中，一些额外的因素值得被考虑：
    - 模板可以用于简单的或者复杂的类型，对于简单类型来说，传值可能更高效。对于复杂类型更好的选择可能对简单类型适得其反。
    - 即是在值传递时，我们依然可以通过`std::ref`和`std::cref`来传递引用。
    - 传递字符串字面量或者数组时，使用引用传递可能还会造成问题。
- 可以看到本书中定义函数模板时会更多地使用值传递，但是不是所有情况都是如此。

使用inline：
- 不像普通函数必须定义为inline才能将定义写在头文件中，函数模板不需要inline也可以定义在头文件，并被包含在多个不同的编译单元中。
- 唯一的例外的针对某一个类型的函数模板全特化，这已经是针对具体的类型。这时就需要使用inline才能定义在头文件中。
- 内联函数和模板的目的不一样，不过他们都能够使函数定义在头文件中。

使用constexpr：
- C++11之后，我们可以使用`constexpr`进行某些编译期计算。

## 总结

- 函数模板为不同的模板参数定义了一簇函数。
- 当传递的函数参数依赖于模板参数时，编译器可以对模板类型参数做推导。将函数模板实例化到对应的模板参数。
- 可以显式指定函数模板参数。
- 可以为模板形参定义默认模板实参，这些实参可以引用前面已经定义的模板参数，并且默认实参不一定要放在最后。
- 可以重载函数模板与普通函数。
- 可以将函数模板与其他函数模板进行重载。应当确保仅有一个函数模板精确匹配每一个函数调用。
- 重载了多个函数模板时，最好显式指定模板参数。
- 在调用函数模板时，确保编译器能够看到所有的重载版本。
