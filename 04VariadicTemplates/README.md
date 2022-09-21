<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第四章：可变参数模板](#%E7%AC%AC%E5%9B%9B%E7%AB%A0%E5%8F%AF%E5%8F%98%E5%8F%82%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [可变参数模板](#%E5%8F%AF%E5%8F%98%E5%8F%82%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [折叠表达式](#%E6%8A%98%E5%8F%A0%E8%A1%A8%E8%BE%BE%E5%BC%8F)
  - [可变参数模板应用](#%E5%8F%AF%E5%8F%98%E5%8F%82%E6%95%B0%E6%A8%A1%E6%9D%BF%E5%BA%94%E7%94%A8)
  - [可变类模板参数与可变参数表达式](#%E5%8F%AF%E5%8F%98%E7%B1%BB%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0%E4%B8%8E%E5%8F%AF%E5%8F%98%E5%8F%82%E6%95%B0%E8%A1%A8%E8%BE%BE%E5%BC%8F)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第四章：可变参数模板

## 可变参数模板

例子：
```C++
void print()
{
}
template<typename T, typename... Args>
void print(T firstArg, Args... args)
{
    std::cout << firstArg << std::endl;
    print(args...);
}
```
- `Args`是一个**模板参数包**（template parameter pack），`args`是一个**函数参数包**（function parameter pack）。
- 在参数包后添加`...`将其展开。
- 这里通过一个模板递归，顺序输出任意数量的参数。递归过程中，每套模板参数都会生成一个函数实例。
- 非模板函数`void print()`重载作为模板递归的出口。

重载可变与不可变参数模板：
- 可以重载可变参数模板与不可变参数模板，如果他们仅仅是尾部的参数包有区别（即一个有，一个没有）。并且两个重载都是匹配的，这时会优先匹配没有参数包的那一个（早期的编译器可能会报二义）。
- 通常可变参数模板会与模板递归配合使用，但不是一定。
- 模板递归的出口可以是一个没有参数列表的普通函数，也可以是一个参数列表和可变参数模板比仅少了一个参数包的函数模板重载。

`sizeof...`运算符：
- 得到参数包的大小，这是一个编译期常量。
- 操作数可以是模板参数包或者函数参数包。
- 通过`sizeof...`配合C++17引入的编译期if（即`if constexpr`），递归终点不再必须是另一个重载了。后续详细讨论。

## [折叠表达式](https://zh.cppreference.com/w/cpp/language/fold)

参数包展开：
- 我们可以通过参数包加上`...`展开这个参数包，`Args...`就等价于`Args1, ..., ArgsN`，模板参数包展开成一个模板参数列表，函数参数包则展开成一个函数参数列表。其实还有非类型模板参数包，其行为类似于函数参数包。
- 此外，能够应用在普通参数上的规则也可以用在参数包上，参数包中的所有参数都会按照同一个模式展开：
- `Args&&... args`和普通引用折叠一样。`Args const&... args`等同理。
- `std::forward<Args>(args)...`会完美转发参数包中所有参数。

折叠表达式：
- 在C++17后，可以更加灵活地展开参数包。我们可以直接对参数包做一些计算：
- 这样的式子叫做折叠表达式（Fold Expression，行为与函数式编程中的典型的`fold foldleft foldright`等函数类似）：

|折叠表达式|求值细节
|:-|:-
|`(... op pack)`|`(((pack1 op pack2) op pack3) ... op packN)`
|`(pack op ...)`| `(pack1 op ... (packN-1 op packN))`
|`(init op ... op pack)`|`(((init op pack1) op pack2) ... packN)`
|`(pack op ... init)`|`(pack1 op ... (pakcN op init))`

- `...`放参数包左边就是从左向右折叠（即`foldleft`），`...`放右边就是从右向左折叠（即`foldright`）。
- 其中的`op`可以是几乎任何二元运算符，甚至`.*`和`->*`。
- 对于大部分二元运算符来说，如果参数包是空的，那么整个折叠表达式是非法的。
- 对于少数运算符参数包为空也是合法的，比如`&&`结果是`true`，`||`结果是`false`，`,`结果是`void()`。
- 注意折叠表达式中的括号是必不可少的。

有了折叠表达式，可以将输出所有参数的操作简化为：
```C++
template<typename... Args>
void print(Args... args)
{
    (std::cout << ... << args) << std::endl; // (init op ... op pack)
}
```
- 但是这样就不能输出空格了，要在中间输出空格，必须将对象包装一下，然后为其实现`<<`运算符：
```C++
template<typename T>
class AddSpace
{
friend std::ostream& operator<<(std::ostream& os, AddSpace<T> s)
{
    return os << s.ref << " ";
}
private:
    T const& ref;
public:
    AddSpace(T const& r) : ref(r) {}
};

template<typename... Args>
void print(Args... args)
{
    (std::cout << ... << AddSpace(args)) << std::endl; // (init op ... op pack)
}
```
- 其中`AddSpace(args)`使用了类模板参数推导，等价于`AddSpace<Args>(args)`。
- 当然这看起来有点太麻烦了，一个更灵活的实现方式是：利用`&&`运算符顺序求值的特性：
```C++
template<typename... Args>
void print(Args... args)
{
    auto printSpace = [](std::ostream& os, const auto& val) -> bool {
        os << val << " ";
        return true;
    };
    (... && printSpace(std::cout, args));
    std::cout << std::endl;
}
```
- 当然可以确保求值顺序的还有逗号运算符，不需要中途中断的话使用逗号运算符比`&&`还更好一些，相比起来没有额外开销。

注意折叠表达式和普通参数包展开的区别：
- 折叠表达式是一个表达式。
- 而参数包展开是成为一个模板参数列表或者函数参数列表（虽然用逗号连接各个参数，但这并不是逗号运算符，和op是逗号的折叠表达式不是一个东西）。

细节：
- 支持的运算符：`+ - * / % ^ & | = < > << >> += -= *= /= %= ^= &= |= <<= >>= == != <= >= && || , .* ->*`。
- 二元折叠中的两个运算符必须相同。
- 初值表达式或者其中的参数包表达式如果运算符优先级低于折叠运算符，需要加括号。

## 可变参数模板应用

一个可变参数模板的典型应用是：配合`std::forward`完美转发参数列表。
```C++
template<typename... Args>
void f(Args&&... args)
{
    otherFunc(std::forward<Args>(args)...);
}
```
- 容器类的原地构造基本都是这么实现的，以支持不同参数列表。

## 可变类模板参数与可变参数表达式

除了以上的地方之外，可变参数模板的参数包还可以出现在表达式、类模板、using声明、推导指引等地方。后续章节详述。

可变参数表达式（其实CppReference中没有这样一个名词）：
- 除了单纯的转发参数包，还可以对他们进行计算，或者叫做将所有参数运用到同一个模式。
- 注意这不是折叠表达式，折叠表达式是得到一个值，而可变参数表达式是得到一个列表，无论是值列表，还是类型列表，只能用在他们能够使用的地方。
- 例子：
    - 比如可变参数表达式`args + args...`，对于参数包`1, std::string("hello"), 2.0`就代表：`1 + 1, std::string("hello") + std::string("hello"), 2.0 + 2.0`。
    - `args + 1 ...`/`(args + 1)...`表示对所有参数+1后再传递。不能用`args + 1...`会被解析为小数。
    - 注意`...`不能直接跟在一个数值字面量之后，会被解析为小数点。

编译期表达式也可以包含参数包：
- `(std::is_same<T1, TN>::value && ...)`，其中`TN`是模板参数包。（这个例子是折叠表达式）。

可变参数索引：
- 一个常见用法：按下标打印一个容器中的多个元素。
```C++
template<typename Container, typename... Idx>
void printElems(const Container& c, Idx... idx)
{
    print(c[idx] ...); // also variadic expression
}
```

参数包展开细节：
- [形参包](https://zh.cppreference.com/w/cpp/language/parameter_pack)
- 参数包展开的场所：
    - 函数参数包：
        - 函数实参列表。
        - 有括号初始化器，比如`new`运算符中，本质上是传递给构造函数的实参列表。
        - 花括号包围的初始化器。
        - `lambda`捕获列表中，捕获一个展开后的参数包，捕获后即可在lambda中使用。
        ```C++
        auto f = [args...]() { f(args...); };
        ```
    - 模板参数包：
        - 模板实参列表，作为一个类型列表（同样可在应用某个模式之后展开）。
        - 函数形参列表，作为一个类型列表（同样可在应用某个模式之后展开）。
        - 模板形参列表，将模板参数包也就是一个类型列表展开到模板形参列表中得到一个非类型模板参数的列表。（那么参数包中所有类型都必须是能够作为非类型模板参数的类型）。
        - 抛出声明，即函数末尾的`throw(X...)`。抛出声明通常来说已不再使用。C++17起。
        - `using`声明中，声明所有模板参数包中的类型、函数等名称。（比如用在从一个模板形参包派生时）。C++17起。
    - 非类型模板形参包：类似于函数形参包。
    - `sizeof...`：也被归类为参数包的展开，函数参数包和模板参数包均可。此时参数包不加`...   `。
    - 属性列表中也可以使用，`[[attributes...]]`。
    - 折叠表达式中：展开后得到一个表达式。C++17起。
- 如果两个形参包在同一模式中展开，那么他们同时展开并且长度必须相同。
    - 例子：对于`std::tuple<std::pair<Args1, Args2>...>`，如果`Args1`是`A1, A2`而`Args2`是`B1, B2`，那么展开后得到`std::tuple<std::pair<A1, B1>, std::pair<A2, B2>>`。
    - 典型情况是一个来自类模板一个来自成员函数模板，或者一个模板参数包一个函数参数包。
- 如果包展开嵌套于另一个包展开中，先展开内层再展开外层：
    - 例子：对于`f(h(args1...) + args2...)`，如果`args1`是`a, b, c`而`args2`是`d, e`，那么展开后得到`f(h(a, b, c) + d, h(a, b, c) + e)`。
    - 甚至上面例子中内外两个参数包可以是同一个。
- 很多时候会同时展开模板参数包和函数参数包：典型例子就是完美转发`std::forward<Args>(args)...`。

可变参数类模板：
- 类模板也可以具有可变模板参数，典型例子是`std::tuple`，`std::variant`。后续将会讨论其实现。
```C++
template <std::size_t...>
struct Indices {};

template<typename T, std::size_t... Idx>
void printByIndex(T t, Indices<Idx...>)
{
    print(std::get<Idx>(t)...);
}
```
- 这是通往模板元编程的第一步，后续会详细讨论模板元编程。

可变参数推导指引：
- 推导指引也可以是可变参数的。
- `std::array`的推导指引：
```C++
template <class T, class... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;
```

可变参数基类和`using`：
- 基类也可以是可变模板参数，通常用于从多个类派生。
- 可以在其中`using`使用可变参数基类中的符号。
```C++
template<typename... Bases>
struct Overloader : Bases...
{
    using Bases::operator()...;
};
```
- 使用`Overloader`从多个类派生便可以将多个类的`operator()`行为组合到一起。

## 总结

- 使用参数包，模板可以定义任意数量任意类型的模板参数。
- 为了处理参数包，需要使用递归，还可能需要搭配非可变模板参数版本的重载。也可以将其包展开后转发给其他函数处理，或者使用折叠表达式。
- 操作符`sizeof...`得到参数包的大小。
- 可变模板参数的典型应用是完美转发任意数量任意类型的参数。
- 通过使用折叠表达式，可以将运算符运用到参数包中的所有参数上。
- 区分**折叠表达式**和**可变参数表达式**：
    - 前者是一个货真价实的表达式，对整个可变参数的序列做折叠操作，得到一个值。必须用括号`()`包起来。
    - 后者实际上并不是一个表达式，参数包按照模式展开后得到的是一个在每个参数上应用模式后的类型列表或者实参列表。
