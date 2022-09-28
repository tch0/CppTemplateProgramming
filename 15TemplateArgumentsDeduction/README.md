<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十五章：模板实参推导](#%E7%AC%AC%E5%8D%81%E4%BA%94%E7%AB%A0%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82%E6%8E%A8%E5%AF%BC)
  - [推导过程](#%E6%8E%A8%E5%AF%BC%E8%BF%87%E7%A8%8B)
  - [推导上下文](#%E6%8E%A8%E5%AF%BC%E4%B8%8A%E4%B8%8B%E6%96%87)
  - [特殊推导情况](#%E7%89%B9%E6%AE%8A%E6%8E%A8%E5%AF%BC%E6%83%85%E5%86%B5)
  - [初始化列表](#%E5%88%9D%E5%A7%8B%E5%8C%96%E5%88%97%E8%A1%A8)
  - [形参包](#%E5%BD%A2%E5%8F%82%E5%8C%85)
  - [右值引用](#%E5%8F%B3%E5%80%BC%E5%BC%95%E7%94%A8)
  - [SFINAE](#sfinae)
  - [推导的限制](#%E6%8E%A8%E5%AF%BC%E7%9A%84%E9%99%90%E5%88%B6)
  - [显式函数模板实参](#%E6%98%BE%E5%BC%8F%E5%87%BD%E6%95%B0%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82)
  - [从初始化器和表达式中推导](#%E4%BB%8E%E5%88%9D%E5%A7%8B%E5%8C%96%E5%99%A8%E5%92%8C%E8%A1%A8%E8%BE%BE%E5%BC%8F%E4%B8%AD%E6%8E%A8%E5%AF%BC)
  - [别名模板](#%E5%88%AB%E5%90%8D%E6%A8%A1%E6%9D%BF)
  - [类模板实参推导](#%E7%B1%BB%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82%E6%8E%A8%E5%AF%BC)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十五章：模板实参推导

C++中的函数模板参数推导机制可以简化函数模板的调用。通常情况下，很多推导规则都是符合直觉的，建立对模板实参推导的稳固理解有利于避免出人意料的情况。模板实参推导最开始引入是为了简化函数模板调用，不过现在也用在一些其他的地方，比如从初始化器推导变量类型。

## 推导过程

- 函数模板会根据每个函数实参去推导对应的一个或多个模板形参。
- 每个实参-形参对的推导都会独立分析，如果最终得到的结论不一致，那么推导过程会失败。
- 即使参数推导成功，但函数体或者返回值不合法的话，依然会编译失败。
- 参数值传递时结果类型会**退化**，引用传递则不会。
- 引用传递不会退化这一点很多时候可能会出人意料，比如字符串字面量会被推导为字符数组。
- 回顾一下退化：
    - 引用类型退化为其引用的类型。
    - 数组退化为指针。
    - 函数退化为函数指针。
    - 去掉顶层CV限定。

## 推导上下文

参数化的类型通常来说可不仅仅是将类型`T`替换为给定的实参类型这么简单：
- 可能还有指针、引用、数组、函数指针、成员指针、参数化类型等复合类型，以及各种CV限定参与其中。
- 复杂类型都是由基础的结构递归组合而成，大多数的类型都可以这样进行匹配，称之为推导上下文（deduction context）。
- 然而，有一些结构不能用于推导上下文：
    - 限定类型名称，比如`Q<T>::X`类型是不能用于推导模板参数`T`的。
    - 不仅仅是非类型参数的非类型表达式，比类型`S<I+1>`不能用于推导`I`。

## 特殊推导情况

有一些情况中，不会从函数调用的实参和函数模板的形参中获取到实参和形参的序对`(A, P)`，这时`A`和`P`会比较特殊：
- 第一个场景，取函数模板地址。
```C++
template<typename T>
void f(T, T);
void (*pf)(char, char) = &f;
```
- 这种情况下，`P`是`void(T, T)`然后`A`是`void(char, char)`，推导成功后`T`被替换为`char`，然后`pf`的值是`f<char>`的地址。
- 一些其他情况：
    - 决定重载函数模板的偏序时。
    - 匹配一个显式特化到函数模板时。
    - 匹配一个显式实例化到函数模板时。
    - 匹配一个友元函数模板特化到模板时。
    - 匹配一个placement版本的`operator delete/delete[]`到对应的placement版本`operator new/new[]`时。
- 更多关于模板实参推导与类模板偏特化的话题会在16章讨论。
- 还有一个特殊情况就是转换函数模板，这种情况下`(A, P)`将在尝试进行类型转换时获取，`A`是源类型，`P`是目标类型。
- 另外`auto`占位符类型也会需要一些特殊处理。

## 初始化列表

- 当函数调用实参传递一个初始化列表时，不能推导`(A, P)`，因为没有`A`。
- 但是如果`P`退化之后，等价于`std::initializer_list<P'>`并且`P'`是一个可推导的模式的话，那么会检查初始化列表中所有参数类型，一致的话就会推导成功，不一致则会失败。
```C++
template<typename T>
void f(T, p);
template<typename T>
void g(std::initializer_list<T>);
int main
{
    f({1, 2, 3}); // ERROR
    g({1, 2, 3}); // OK: T is deduced to int
    g({1, 2, '3'}); // ERROR
}
```
- 参数为数组时也可以这样推导。

## 形参包

实参和形参对应推导时，每个实参-形参对的推导都会独立分析，但是对于参数包就不是这样了。此时多个实参`P`对应于一个参数包`A`：
- 例子：
```C++
template<typename First, typename... Rest>
void f(First first, Rest... rest);

void g(int i, double j, int* k)
{
    f(i, j, k); // Rest is deduced to {double, int*}
}
```
- 然后模板参数包中会被依次推导为一个类型序列。
- 除了函数参数列表，模板出现在模板实参列表中也可以推导。
- 参数包只在函数参数列表和模板实参列表中才是推导上下文，否则就不是。

字面量运算符模板：
- [字面量运算符](https://zh.cppreference.com/w/cpp/language/user_literal)模板有自己独特的决定实参的方式。
```C++
template<char...> int operator""_B7();
int a = 121_B7;
```
- 当推导`121_B7`时，模板参数被推导为`'1', '2', '1'`。
- 仅对数值字面量有效。

## 右值引用

引用折叠：
- 程序员不可以直接定义指向引用的引用。但是通过模板实参替换模板形参、类型别名、或者`decltype`结构，这种情形是可以存在的。
- 其结果类型由一种叫做引用折叠（reference collapseing）的规则决定。
- 折叠时只有最里层的CV限定会被保留，任何上层的都会忽略。
- 引用折叠规则：
```
inner outter result
&    +  &   -> &
&    +  &&  -> &
&&   +  &   -> &
&&   +  &&  -> &&
```
- 例子：
```C++
using RCI = const int &;
volatile RCI&& r = 42; // r has type const int&, outter volatile is discarded
using RRI = int&&;
const RRI&& rr = 42; // rr has type int&&, outter const is discarded
```

转发引用：
- 当函数模板的参数是转发引用时，会使用一种特殊的推导规则。
- 这时候，模板实参推导不仅仅考虑函数调用实参的类型，还会考虑其值类别。
    - 如果调用实参是一个左值，那么其类型会被推导为实参类型的左值引用，然后引用折叠规则会确保这个函数参数类型是左值引用类型。
    - 否则的话，推导出的函数参数类型就是实参类型的的右值引用。
```C++
template<typename T>
void f(T&& p);

void g()
{
    int i;
    const int j = 0;
    f(i); // T is int&, paramter p has type int&
    f(j); // T is const int&, parameter p has type const int&
    f(10); // T is int, parameter p has type int&&
}
```
- 一定注意传入右值时模板参数会被简单推导为其类型，而非右值引用，参数类型也不需要引用折叠。
- 将模板参数推导为引用类型可能会有一些限制，比如定义变量必须初始化，返回值如果返回引用可能会有意想不到的行为（比如返回一个临时变量引用造成悬挂引用）。内部使用该模板类型参数时为了确保移除引用可以使用`std::remove_reference`。
- 注意右值引用本身是一个左值，所以遇到转发引用会被推导为左值引用，这是需要注意的。

完美转发：
- 转发引用的特殊推导规则加上引用折叠就可以实现完美转发。
- 完美转发到另一函数时，使用`static_cast<T&&>`亦可以完成转发，但通常我们都是用`std::forward<T>`以更好的表达意图。
- 可变模板参数的完美转发：右值引用模式的参数包通常与完美转发结合起来，同时展开两个参数包：`std::forward<Args>(args)...`。
- 返回值如果也需要推导的话，使用`auto`会自动退化，如果需要完美转发返回值需要使用`auto`配合`decltype`尾置返回值，或者使用更简单的`decltype(auto)`。

推导的意外：
- 某些时候可能使用模板类型参数的右值引用但是却只想表达右值引用而非转发引用的含义。那么可以使用`std::remove_reference_t<T>&&`作为参数类型。
- 或者使用SFINAE使函数仅对右值有效：`std::enable_if<!std::is_lvalue_reference_t<T>>`。

## SFINAE

函数类型推导很多时候都会和SFINAE结合起来，在某一个重载上推导失败时根据SFINAE原则，忽略这个重载，而不是报错。

**即时上下文**：
- 在为了进行类型推导而做的函数模板替换中，在实例化过程中发生的下列事情**不是**发生在函数模板替换的即时上下文（immediate context）中：
    - 类模板定义。
    - 函数模板定义。
    - 变量模板的初始化器。
    - 默认参数。
    - 默认成员初始化器。
    - 异常声明。
    - 另外所有由替换过程引起的特殊成员函数（比如转换构造函数、拷贝构造之类）的隐式定义也不在即时上下文中。
- 所有其他的东西都是即时上下文的一部分。
- 所以在上述不在函数替换的即时上下文中发生的事情实例化时如果触发了错误，那么就是一个真正的错误（硬错误，hard error，对应的我们称即时上下文中满足SFINAE原则时发生的实例化错误为软错误，soft error），而不仅仅是SFINAE忽略这个函数声明那么简单。
- 上面说的太拗口了，简单总结就是：即时上下文基本上就是你所看见的模板声明本身，所有在其之外实例化失败的东西就是硬错误而非SFINAE。也就是只有即时上下文中的模板替换错误才会触发SFINAE。
- 例子1：类模板定义。
```C++
template<typename T>
struct Array
{
    using iterator = T*;
};

template<typename T>
void f(typename Array<T>::iterator first, typename Array<T>::iterator last); // 1

template<typename T>
void f(T*, T*); // 2

int main(int argc, char const *argv[])
{
    f<int&>(nullptr, nullptr); // will trigger a hard error, when instantiate the first version of f
    return 0;
}
```
- 例子2：函数模板。
```C++
template<typename T> auto f(T p) { return p->m; }
int f(...);
template<typename T> auto g(T p) -> delctype(f(p));
int main()
{
    g(42); // instantiation of f fails, trigger a hard error
    return 0;
}
```
- 19章会详细介绍如何使用SFINAE友好的trait避免即时上下文问题。

## 推导的限制

使用模板实参推导很方便，但也会有一些限制：
- 不能推导类模板参数，C++17起可以推导。
- 默认函数调用实参只有在不传入对应参数时才会实例化，即使这个默认实参是模板参数无关的，也不会用于模板实参推导。
- 和函数默认实参类似，异常说明也仅在需要时才会实例化，同样不参与模板实参推导。

## 显式函数模板实参

- 当一个参数不能推导时，必须显式指定模板实参。当然即使可以推导，也可以指定模板实参。
- 指定模板实参后对应的函数参数不再用于推导，并且可以进行类型转换。
- 可以显式指定一部分参数，推导剩余参数，显式指定的参数是从左往右匹配的。也就是说应该将不能推导的参数放前面，能够推导的放后面。
- 使用推导决定模板参数时，指定一个空模板实参列表也是有用的，用于指定调用模板版本，一定不会调用非模板版本。
- 当函数模板用于友元函数时，如果要指定是使用一个已经存在的模板作为友元，那么可以在函数名后加上空模板实参列表`<>`。而不是在没有已知声明的情况下作为第一次声明。
- 如果函数模板有多个匹配，那么就不能取地址，如果经过SFINAE筛选后只剩下一个，那么是可以取地址的。

## 从初始化器和表达式中推导

C++允许变量从其初始化器推导（`auto decltype(auto)`），同样也提供了机制获取命名对象或者表达式的类型（`decltype`）。

**`auto`类型标识符：**
- `auto decltype(auto)`都叫做占位符类型（placeholder type），可以用于命名空间作用域、局部作用域从初始化器推导变量类型。
- 使用`auto& auto&&`创建推导类型的引用。
- 使用`auto&&`推导时遵循模板中万能引用的推导规则，如果是左值会推导为左值引用，然后经过引用折叠后最终变量类型为左值引用。这个技巧常用在不知道返回值类型的泛型代码中，用于接收任意类型的返回值。
- 比如可以在范围for中使用可以确保没有对象会被拷贝，不过一般来说范围for中最常用还是`auto&`和`const auto&`就是了（因为容器中一般不会保存右值，所以一般引用折叠后依旧是左值引用）：
```C++
for (auto&& x : c)
{
}
```
- `auto`可以和`const`、指针、成员指针等类型组合成复合类型，但是`auto`必须是其中的底层类型。不能作为模板实参，或者非底层类型的情况下声明符的一部分（主要为了防止滥用）。
```C++
template<typename T> struct X { const T m; };
const auto N = 10u; // auto is unsigned
auto* p = &N; // auto is const unsigned
const auto X<int>::*pm = &X<int>::m; // auto is int
X<auto> xa = X<int>(); // ERROR
const int auto::*pm2 = &X<int>::m; // ERROR
```
- C++11前，`auto`的含义是自动变量，也就是非静态局部变量就是`auto`变量，这种用法是多余的且在实践中非常罕见。C++11起引入了如今的新含义，废弃了以前的含义。
- 推导返回值：
    - C++14起可以将函数返回值声明为`auto`，让编译器自动推导。
    - lambda中也有类似的推导机制，不写返回值类型就是让编译器自动推导，C++11起就可以推导了，C++14起lambda的推导才被解释为和`auto`同一个机制。
    ```C++
    auto f1() { return 42; }
    auto f2() -> auto { return 42; }
    ```
    - 上面两种方式一个含义，前者指明用`auto`推导返回值，后者则是`auto`作为占位符，实际返回值类型由尾置返回值类型决定，而尾置返回值是`auto`，所以自动推导。
    - 实践中甚至更青睐后一种，因为和lambda统一起来了，并且可以书写不同的尾置返回值类型，常在尾置返回值中使用`decltype`。
    - lambda中的机制是一样的，如果没有写返回值类型，就是自动使用`auto`推导，也可以使用尾置返回值，下面两种方式同样等价：
    ```C++
    auto g1 = []() { return 42; }
    auto g2 = []() -> auto { return 42; }
    ```
    - `auto`声明返回值同样可以用于定义声明分离的情况，但这种应用限制很大，只能用于定义在任何函数使用点都可见的情况，因为一定要确定返回值类型才能生成代码。
    ```C++
    auto f();
    int f(); // forward declaration like this is invalid.
    auto f() { return 42; }
    ```
- 推导非类型参数：
    - C++17起，非类型模板参数类型也可以使用`auto`占位然后通过推导得到了。
    ```C++
    template<auto V> struct S;
    S<42>* ps; // V is deduced to 42, and has type is int
    ```
    - 注意非类型模板参数的限制依旧存在，比如不能使用浮点数。
    - 在其中要使用这个非类型模板参数类型时可以用`decltype(V)`。
    - 非类型模板参数包同样可以使用。并且可以被推导为不一样的类型，而不像普通非类型模板参数包那样只能是同一个类型。
    - 如果想要强制得到一个同类型的参数包，可以使用`template<auto V1, decltype(V1)... VRest>`。

**使用`decltype`表达表达式的类型：**
- `decltype`可以推导表达式的类型，`decltype`是根据现有表达式直接得到类型。而`auto`是根据初始化器去推导一个新的变量的类型，有一定区别。
- `decltype`有一些微妙的点，需要特别注意，对于`decltype(e)`：
- 如果`e`是一个实体的名称（变量、函数、数据成员等）或者一个类成员，那么会得到这实体的声明类型。
    - 例子：其中`y1`的类型会取决于`+`运算符的行为，而`y2`类型一定和`x`一致。
    ```C++
    auto x = ...;
    auto y1 = x+1;
    decltype(x) y2 = x+1;
    ```
- 否则，如果`e`是一个表达式，那么`decltype(e)`会根据表达式类型和值类别得到最终类型：
    - 如果`e`是`T`类型的左值（lvalue），那么得到`T&`。
    - 如果`e`是`T`类型的亡值（xvalue），那么得到`T&&`。
    - 如果`e`是`T`类型的纯右值（prvalue），那么得到`T`。
- 如果想要将单独的实体名称作为表达式推导，那么需要在其外加一个`()`，如`decltye((e))`。这是C++极少数的加括号影响表达式行为的地方之一。
- `decltype`同样可以用于推导返回值类型，会保留表达式的类型与值类别。类似于完美转发的效果。
```C++
xxx f();
decltype(f()) g()
{
    return f();
}
```
- `decltype`可以用来弥补`auto`推导不足以满足要求的情况。
- 当用`auto`的方式来写`decltype`时，需要在`decltype`中重复一次表达式，C++14引入了`decltype(auto)`来简化这个问题。
- `decltype`中的表达式是不求值表达式，不会求值或完全实例化调用到的函数模板、变量模板等。
- 注意`decltype`的结果不会退化，而`auto`会自动退化。

**`decltype(auto)`：**
- 同样是一个占位符类型，可以用于变量类型、返回值类型、模板形参等。
- 然而，不同于`auto`是使用模板实参推导来决定表达式类型。`decltype(auto)`是直接在对应的表达式上应用`decltype`得到结果。
- 从实践上来说，本质区别还是`auto`（无引用修饰时）会自动退化，而`decltype(auto)`不会退化。
```C++
int i = 42;
const int & ri = i;
auto x = ref; // x has type int
decltype(auto) y = ref; // y has type const auto&
```
- 在对于引用类型需要保留或者传递引用类型，而对于值类型则值传递的场景就必须使用`decltype(auto)`。而在任何情况都传递值或者都传递引用的场景就可以使用`auto auto& auto&&`。
- 前面介绍过`auto&&`可以用于完美转发返回值，`decltype(auto)`也可以，但场景是有区别的：
    - 前者通常用于函数内部接受另一个函数的返回值然后完美转发，避免拷贝发生（左值用左值引用接收，右值使用右值引用接收）。
    - 而`decltype(auto)`通常用于完美转发`return`表达式返回的值，用在函数声明中的返回值类型（返回值类型就传递值，返回引用就传递引用，避免引用转换为值，值转换为引用）。
- 不同于`auto`，`decltype(auto)`推导时不能再使用`decltype(auto)`构建复合类型，因为`decltype(auto)`就是最终类型：
```C++
decltype(auto)* p = nullptr; // invalid
const decltype(auto) NN = N*N; // invalid
```
- 注意`decltype(auto)`的初始化器（或者函数返回值）中的额外的外层`()`可能会影响行为，因为就是将`decltype`运用在右侧表达式或者名称上：这一点是`decltype`一样，比较微妙，需要特别注意。
```C++
int x{};
decltype(auto) z = x; // z has type int
decltype(auto) r = (x); // r has type int&
```
- C++17起，`decltype(auto)`同样可以用于推导非类型模板参数的类型。`()`的影响在这里会很重要，最终推导结果是一个常量作为模板参数，还是一个引用作为模板参数是有很大区别的。通常来说比较少这么用。
```C++
template<decltype(auto) Val> struct S;
constexpr int c = 42;
S<c> sc; // has type S<42>, c must be constexpr
S<(v)> sv; // has type S<(int&)v>, the template parameter has type int&, totally differ from sc.
S<v> sv2; // ERROR, v is not a compile-time constant.
```

**`auto`推导的特殊情况：**
- auto推导存在一些特殊情况：
- 第一种特殊情况就是从初始化列表推导，因为模板类型参数不能从初始化列表推导，但在有`std::initializer_list`版本参数的情况下是可以推导的。使用`auto`做一层中间转发就可以让函数模板推导初始化列表了。
    ```C++
    template<typename T> void f(T);
    f({1, 2, 3}); // ERROR, deduction failed
    auto vals = {1, 2, 3}; // vals has type std::initializer_list<int>
    f(vals); // success

    auto val {2}; // OK: val has type int in C++17
    auto values {1, 2, 3}; // ERROR in C++17
    ```
    - 如果是使用初始化列表拷贝初始化，会推导得到`std::initializer_list`。如果是直接初始化，那么列表中只能有一个值，并且会推导为该值的类型，多个值则会直接失败。
    - 值得注意的是：使用`auto`推导返回值但是却返回初始化列表是非法的。因为初始化列表指向的底层数组对象会在离开函数后失效，然后导致悬垂引用（因为初始化列表中保存常量，拷贝操作是浅拷贝）。虽然语法上看起来没有问题，但标准考虑到了这一点，禁用掉了这种情况。
- 第二种特殊情况是多个变量声明共享同一个`auto`的情况。
    - 此时会独立从每一个初始化器推导每一变量的类型，并且推导出来的`auto`类型相同才会成功，否则就是非良构的。
    - 其中也可以定义指针，和普通类型声明中一样。
    - 当使用`auto`推导函数返回值，但是有多个返回值语句时，和这里是一样的：独立推导且目标类型一致才会成功。
    ```C++
    // error: inconsistent deduction for auto return type: 'double' and then 'int'
    auto f(bool b)
    {
        if (b) {
            return 42.0;
        }
        else {
            return 1;
        }
    }
    ```
    - 如果函数是递归的，返回的表达式中存在对自己的调用，那么将无法推导。除非前面已经有其他能够确定返回值类型的返回语句，此时将忽略这个递归的返回值语句。
- 下一个特殊情况：推导返回值时，如果没有返回语句，或者有`return;`返回语句，那么返回值将被推导为`void`，非常符合直觉。但是如果返回值模式不能匹配`void`，比如`auto* auto&`那么推导失败。
- 最后一个特例发生在使用`decltype(auto)`推导返回值与SFINAE结合的时候：
```C++
template<typename T, typename U>
auto addA(T t, U u) -> decltype(t+u)
{
    return t + u;
}
void addA(...);

template<typename T, typename U>
auto addB(T t, U u) -> decltype(auto)
{
    return t + u;
}
void addB(...);

struct X {};
using AddResA = decltype(addA(X(), X())); // AddResA is void
using AddResB = decltype(addB(X(), X())); // ERROR, deducation of type of t + u is not in immediate context, no SFINAE.
```
- 在使用`decltype(auto)`代替`decltype(expression)`时，后者在即时上下文中，有SFINAE机制，并且只需要替换声明。但前者推导的表达式在函数体中，**不在即时上下文中**，需要实例化整个函数才能得到返回值类型，所以如果实例化失败就会触发**硬错误**。
- 通常我们会认为`decltype(auto)`仅仅是`decltype(expression)`的一个快捷使用方式，但事实上会有那么一些和SFINAE机制互动上的差别，使用`decltype(auto)`时必须特别注意此类问题。

[**结构化绑定（structured binding）：**](https://zh.cppreference.com/w/cpp/language/structured_binding)
- 看个例子：
```C++
struct MaybeInt { bool valid; int value; };
MaybeInt g();
const auto&& [b, N] = g();
```
- 语法上来说，一个结构化绑定必须总是声明为`auto`类型并使用`const volatile`以及`& &&`修饰（但不能是指针），然后跟一个方括号其中包含至少一个标识符，最后跟一个初始化器。
- 有三种初始化器可以用来初始化一个结构化绑定：
    - 第一种是类类型，其中所有的非静态数据成员都是公有的，要么所有成员直接位于该类中，或者全部位于一个不产生歧义的公有基类中，类中也不能有匿名联合。这种情况下，结构化绑定中的标识符数量必须与成员数量一致，每个成员被按顺序对应绑定到对应名称。如果成员是位域，那么不能取地址。
    - 第二种是数组，数组大小必须与括起来的名称数量一致。在直接用`auto`不使用引用的情况下，数组会被拷贝。（这是C++为数不多内建数组可以直接拷贝的语法，另外两个是lambda的值捕获数组，以及合成构造函数中）。
    - 第三种是类tuple的类型（tuple-like classes），可以将他们的元素使用模板`get<>()`分解到每个变量。元组长度必须和变量数量一致。首先会尝试使用成员版本的`get<i>()`接口，如果没有则会使用全局版本的`get<i>(e)`。标准库中满足的类型有`std::tuple std::pair std::array`。可以对自定义类型为`std::tuple_size std::tuple_element`实现特化，然后添加成员或者全局版本的`get<>()`（同一个命名空间，使用ADL）就可以满足这个协议以使用结构化绑定。
- 结构化绑定是一种解包（decomposition）方式，使用结构化绑定可以更方便的返回多个值。本质是将其包装为对象，并方便地解包，而不是真的返回多个值。

[**泛型lambda：**](https://zh.cppreference.com/w/cpp/language/lambda#:~:text=%E8%AF%A5%20lambda%20%E6%98%AF-,%E6%B3%9B%E5%9E%8B%20lambda,-%E3%80%82)
- 编写lambda时，有时参数和返回值类型写起来会非常冗长。C++14引入了泛型lambda，可以将参数类型声明为`auto`，然后通过推导得到类型以简化lambda的书写。
- 在lambda中，有`auto`参数的lambda称之为泛型lambda，每个`auto`参数会对应于一个隐式的模板参数。这个lambda可以被任何参数调用，调用时的参数类型推导就和模板参数类型推导一个道理。
- lambda对象是隐式生成的闭包类型（closure type）的对象，lambda对象也称之为闭包对象。
- 对于泛型lambda，其闭包类型中的函数调用运算符是一个成员函数模板。
- 当lambda被调用时，其函数调用运算符模板被实例化。

## 别名模板

- 别名模板对推导是透明的，无论任何时候，别名模板在给了模板实参之后，永远都是先使用模板实参去替换其定义中的形参，得到最终模板之后才进行推导。别名模板对最终的推导无任何影响。
- 注意别名模板不能特化。

## 类模板实参推导

[类模板实参推导](https://zh.cppreference.com/w/cpp/language/class_template_argument_deduction)（Class Template Argument Deduction, **CTAD**，C++17起）是通过类模板的构造函数或者推导指引做到的：
- 注意要么所有类模板实参都由推导或者默认参数得到，如果不能推导出所有实参，那么就必须显式指定所有类模板实参。

推导指引：
- C++17同时引入了类模板推导指引，可以指导类模板在传入某个参数列表时如何进行推导。
- 语法：
```C++
template<TemplateParameterList> ClassTemplateName(ConstructorParameterList) -> ClassTemplateName<TargetArgumentList>;
```
- 特点：
    - 必须使用尾置返回值类型。
    - 开头并没有`auto`。
    - 推导指引的名称必须是类模板的未修饰名称，声明于同一个作用域的前面。
    - 目标类型（guided type）必须是一个模板id它的模板名称是推导的类模板名称。
    - 可以使用`explicit`修饰。
- 使用类模板实参推导：
```C++
template<typename T>
class S
{
public:
    S(T) {}
};
template<typename T> S(T) -> S<T>; // deduction guide

// using of CTAD
S x(12); // S<int>
S* p = &x; // ERROR: not permitted
```
- 类模板实参推导时格式是：`ClassPlaceholderType var = xxx;`，类模板名称也叫做占位符类类型，然后必须跟变量，然后必须初始化。
- 多个对象同时使用类模板参数推导声明时，必须是同一个类型，和`auto`类似。
```C++
S s1(1), s2(2.0); // ERROR
```
- 通常推导指引都需要有一个对应的构造函数，但这个要求不是必须的。可以对聚合类声明推导指引（注意对聚合类必须进行聚合初始化）。在没有构造函数的情况下，如果不声明推导指引，那么实例化对象时必须指定模板实参。
```C++
template<typename T>
struct X
{
    T val;
};
template<typename T> X(T) -> X<T>;

// aggregate class
X y1{1};
X y2 = {2};
X y3(3); // ERROR
X y4 = 4; // ERROR
```

隐式推导指引（implicit deduction guides）：
- 就是根据构造函数生成的默认推导指引，比较符合直觉，但某些地方会比较微妙，特别涉及初始化列表时，细节不赘述。
- 通常来说要使用类模板实参推导的话，请为所有想要推导的构造声明推导指引。

其他微妙的细节：
- 为了让注入类名能够正常工作，保持向前兼容，如果名称是注入类名，那么会禁用CTAD，类模板名称表示的是当前实例化。
- 当构造函数涉及到转发引用时，可以看到第二个推导指引看起来就像是转发引用。构造函数则不会有这个问题，因为模板参数来自类模板，但是推导指引就不一样了。如果传入非`const`左值，按照推导指引后者更加匹配，根据万能引用含义，`T`会被推导为左值引用，最终推导出类型`X<T&>`，这会导致错误（这里明显是想匹配前者，以得到类型`X<T>`）。所以C++标准委员会决定禁用推导指引中的转发引用含义，如果函数参数右值引用的底层类型是来自类模板的话，以此堵上了这个漏洞。
```C++
template<typename T>
struct X {
    X(const T&);
    X(T&&);
};
template<typename T> X(const T&) -> X<T>;
template<typename T> X(T&&) -> X<T>;
```
- `explicit`关键字：推导指引可以声明为`explicit`，对直接初始化无影响，对于拷贝初始化则会忽略这个推导指引，从而选择其他推导指引。注意下面的代码，仅仅推导指引加了`explicit`，但是构造函数没有加，所以构造函数还是该调哪个调那个，能影响的只有推导得到的最终类型。
```C++
template<typename T, typename U>
struct X {
    X(const T&) // *1
    {
        std::cout << "X(const T&)" << std::endl;
    }
    X(T&&) // *2
    {
        std::cout << "X(T&&)" << std::endl;
    }
};
template<typename T> X(const T&) -> X<T, T&>; // #1
template<typename T> explicit X(T&&) -> X<T, T>; // #2

int main(int argc, char const *argv[])
{
    X x1 = 1; // prefer #1 and call *2
    static_assert(std::is_same_v<decltype(x1), X<int, int&>>);
    X x2{2}; // prefer #2 and call *2
    static_assert(std::is_same_v<decltype(x2), X<int, int>>);
    return 0;
}
```
- 拷贝构造与初始化列表：长度为1初始化列表构造推导结果和长度更长的结果不一样。不过还是比较符合直觉的，只能说少这样用，显式给模板参数就完了，谁能记住这么多细节啊！
```C++
template<typenme... Ts> struct Tuple {
    Tuple(Ts...);
    Tuple(const Tuple<Ts...>&);
};
template<typename... Ts> Tuple(Ts...) -> Tuple<Ts...>; // #1
template<typename... Ts> Tuple(const Tuple<Ts...>&) -> Tuple<Ts...>; // #2

auto x = tuple{1, 2}; // #1
auto a = x; // #2
auto b(x);  // #2
Tuple c{x, x}; // #1, d has type Tuple<Tuple<int, int>, Tuple<int, int>>
Tuple d{x}; // #2, d has type Tuple<int, int>
```
- 最后需要明确**推导指引唯一的作用就是用来推导**，而**不是用来调用**的，传递参数时值传递还是引用传递是无关紧要的，推导指引并不是必须和构造函数一一对应的（只是常这样做保证每种合法的参数都能正确推导）。经过推导指引确定类模板类型后，对构造函数的调用并无任何影响。
```C++
template<typename T>
struct X {
    X(const T&);
    X(T&&);
};
template<typename T> X(T) -> X<T>; // no matter rvalue or lvalue, all X<T>
```
## 后记

- 函数模板的模板参数推导是最原始的C++设计中就有的内容。
- SFAINAE实际上在C++98就引入了，但是那是能做的事情还非常少，直到C++11才支持了更多情况。
- `auto`和`decltype`原本计划在C++03引入，不过最终是在C++11引入的。
- C++17开始可以使用`auto`声明非类型模板参数。
- C++17引入类模板实参推导、结构化绑定。