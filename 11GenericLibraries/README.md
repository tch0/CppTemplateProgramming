<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十一章：泛型库](#%E7%AC%AC%E5%8D%81%E4%B8%80%E7%AB%A0%E6%B3%9B%E5%9E%8B%E5%BA%93)
  - [可调用对象](#%E5%8F%AF%E8%B0%83%E7%94%A8%E5%AF%B9%E8%B1%A1)
  - [实现泛型库的其他实用组件](#%E5%AE%9E%E7%8E%B0%E6%B3%9B%E5%9E%8B%E5%BA%93%E7%9A%84%E5%85%B6%E4%BB%96%E5%AE%9E%E7%94%A8%E7%BB%84%E4%BB%B6)
  - [完美转发临时对象](#%E5%AE%8C%E7%BE%8E%E8%BD%AC%E5%8F%91%E4%B8%B4%E6%97%B6%E5%AF%B9%E8%B1%A1)
  - [引用作为模板参数](#%E5%BC%95%E7%94%A8%E4%BD%9C%E4%B8%BA%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0)
  - [延迟求值](#%E5%BB%B6%E8%BF%9F%E6%B1%82%E5%80%BC)
  - [编写泛型库时需要考虑的问题](#%E7%BC%96%E5%86%99%E6%B3%9B%E5%9E%8B%E5%BA%93%E6%97%B6%E9%9C%80%E8%A6%81%E8%80%83%E8%99%91%E7%9A%84%E9%97%AE%E9%A2%98)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十一章：泛型库

本章介绍一些在编写针对尚未实例化的未知类型的泛型组件时需要考虑的一般性问题。

## 可调用对象

支持函数对象：
- 许多库包括标准库都会有需要支持回调函数的场景。
- 通常有多种方式支持回调函数：
    - 函数指针。
    - 函数对象：lambda或者重载了`operator()`的对象。
    - 能够转换为函数指针或者函数引用的类类型。
- 这些类型都叫做**函数对象**类型，这些类型的对象都叫做函数对象（function object）。
- 这些对象被统称为可调用对象（callable object）。

支持函数对象：
- 可调用对象通常与模板结合使用，通过自动推导可调用对象类型以支持任意类型的可调用对象：
```C++
template<typename Iter, typename Callable>
void foreach(Iter first, Iter last, Callable op)
{
    for (; first != last; ++first)
    {
        op(*first);
    }
}
```
- 值传递函数时会自动退化为函数指针，`const`修饰对函数类型没有意义，会被自动忽略。函数的引用类型在C++中比较少使用，但确实是合法的。
- 传递函数对象时，是调用的它的`operator()`，通常`operator()`应该定义为`const`，除非这个调用会更改内部状态且这就是预期行为。
- 对象可以隐式转换为函数指针或者引用也是可能的，这种使用方式叫做代理调用函数（surrogate call function），相对来说并不常见。
- lambda表达式是匿名函数对象，是一个重载了`operator()`的匿名类的实例。
- 使用`[]`开头的也就是捕获列表为空的lambda表达式提供了一个转换为函数指针的转换运算符。

处理成员函数和额外参数：
- 一种可能的前面没有使用的函数是成员函数。
- 这是因为调用非静态成员函数需要一个对象或者指针以`obj.memfn(...) ptr->memfn(...)`的方式调用，和`functor(...)`的调用方式不匹配。
- 这时可以使用C++17引入的`std::invoke(op, ...)`：
    - 如果可调用对象是一个成员函数指针，那么会将第一个额外参数作为this对象，用剩余参数调用这个成员函数。
    - 如果不是，那么就用所有额外参数调用这个函数对象。
    - 甚至可以传入一个数据成员指针，然后调用时会返回该对象的该数据成员的值。
- 要统一函数指针也可以使用`std::mem_fn`从成员函数指针生成函数对象。
- 重新实现`foreach`以支持成员函数指针：
```C++
template<typename Iter, typename Callable, typename... Args>
void foreach(Iter first, Iter last, Callable op, const Args&... args)
{
    for (; first != last; ++first)
    {
        std::invoke(op, args..., *first);
    }
}
```

包装函数调用：
- 如果想要包装函数调用，在其中进行一些操作，比如记录日志之类的。那么需要谨慎地处理参数和返回值：
```C++
template<typename Callable, typename... Args>
decltype(auto) call(Callable&& op, Args&&... args)
{
    return std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
}
```
- 返回值使用`decltype(auto)`自动推导返回值类型保证不会退化，注意不应使用`auto`因为会自动退化。
- 如果中间要做一些事情就不能直接返回了，可能需要保存中间结果：
```C++
template<typename Callable, typename... Args>
decltype(auto) call(Callable&& op, Args&&... args)
{
    decltype(auto) ret{std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...)};
    // do something
    return ret;
}
```
- 注意不能使用`auto&&`。
- 但是这样是不能用于返回值是`void`的情况的，因为`void`是不完全类型。
- 有两个解决方案：
- 第一个：通过一个局部类对象的析构函数来做。
```C++
template<typename Callable, typename... Args>
decltype(auto) call(Callable&& op, Args&&... args)
{
    struct dosomething {
        ~dosomething()
        {
            // do something
        }
    } dummy;
    return std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
}
```
- 第二个：分情况讨论，返回值是`void`时单独处理。
```C++
template<typename Callable, typename... Args>
decltype(auto) call(Callable&& op, Args&&... args)
{
    if constexpr (std::is_same_v<std::invoke_reuslt_t<Callable, Args...>, void>)
    {
        std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
        // do something
        return;
    }
    else
    {
        decltype(auto) ret{std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...)};
        // do something
        return ret;
    }
}
```
- `std::invoke_reuslt`自C++17引入。

## 实现泛型库的其他实用组件

`std::invoke`是其中一个有用的组件，标准库中还有很多重要组件。

类型特性：
- 标准库提供了一系列[类型特性](https://zh.cppreference.com/w/cpp/header/type_traits)用来对类型进行求值或者修改。
- 这些类型特性具有通用的表示泛型代码需要支持或者满足的各种约束的能力。
- 这些能力包括：
    - 判断类型分类：如是否是整型、浮点型、指针、各种引用、函数等。
    - 判断复合类型分类：基础类型、算术类型之类。
    - 判断是否具有某个类型性质：CV限定、是否平凡、聚合、是否是有符号无符号等。
    - 判断一个类型是否支持某个操作：平凡构造、由某个参数列表构造、可由某个类型赋值等。
    - 性质查询：类型对齐要求、数组维数、数组某个维度大小。
    - 类型关系：两个类型是否相同、是否是基类派生类关系、是否可转换、布局是否兼容、可调用对象是否可以以某组参数列表调用等。
    - CV限定修饰：添加移除顶层CV限定。
    - 引用限定：添加移除引用。
    - 指针限定：添加移除指针。
    - 符号修饰：有符号无符号数相互转换。
    - 数组维度操作：移除一个或所有维度。
    - 杂项变换：公共类型、退化、编译期类型选、`enable_if`等。
- 头文件`<type_traits>`。
- 这些特性常用在SFINAE、编译期`if`等场景中。
- 这些特性很可能跟你预想的行为不一样：使用之前先看文档，明确行为之后再用，不要想当然。特别是这些特性使用时对输入类型都有一定要求，不满足要求还不一定会报错。
- [附录D](../AppendixD/)中会有细节描述。

`std::addressof()`函数模板：
- 用于获取对象的地址，即是在重载了`operator &`时也能够正确获取到地址（比如智能指针）。
- 在泛型代码中取地址就可以使用`std::addresssof()`代替`&`运算符。

`std::declval<>()`函数模板：
- 作为一个占位符，用在需要一个某类型对象引用但不需要求值（不求值语境也就是`decltype()`中）的地方。
- 也就是在不尝试构造对象的情况下假定拥有一个该类型对象。
- 因为返回类型是右值引用，所以通常要配合`std::decay`使用。
- 例子：`std::decay_t<decltype(condition ? std::declval<T1>() : std::declval<T2>())>`就是满足条件就是类型T1，不满足就是类型T2的意思。虽然这个场景`std::conditional`也能做到就是了。

## 完美转发临时对象

通过万能引用类型参数加上`std::forward<>()`可以完美转发参数：
```C++
template<typename T>
void f(T&& t)
{
    g(std::forward<T>(t));
}
```
如果要完美转发临时对象可以使用`auto&&`与`decltype`配合：
```C++
template<typename T>
void f(T&& t)
{
    auto&& val = get(x);
    set(std::forward<decltype(val)>(val));
}
```
- 这样做可以用来避免拷贝。

## 引用作为模板参数

引用也可以作为模板参数，很多时候这会从根本上改变模板的行为。通过自动类型推导，推导出来的通常是值类型。要使用引用类型模板参数，通常要显式指定模板参数。
- 某些时候设计模板时就没有考虑引用作为模板参数的情况，那么使用引用作为模板参数可能就会导致非预期行为或者报错。
- `std::tie`是引用作为模板参数的例子。
- 使用引用作为模板参数需要注意的点：
    - 默认初始化不再合法。
    - 隐式默认构造函数不再有效。
- 使用引用作为非类型模板参数（是合法的）可能会很危险。
- 代码中明确这么用可能会比较少，但是C++17起，模板参数也可以使用`auto decltype(auto)`进行推导了，如果使用`decltype(auto)`是很容易出现将模板实参推导成引用的情况的。通常来说应该使用`auto`。
- 某些设施为了禁止引用类型作为模板参数可能会有约束或者静态断言提供保证，编写模板时需要注意这一点。
- 当前我唯一遇到过的使用引用作为模板参数的情况只有`std::pair/std::tuple`，保存引用时可以很方便做到解包。
```C++
std::pair<int, int> f();
int main()
{
    int a, b;
    std::tie(a, b) = f(); // std::tie will return a std::pair<int&, int&>
    // equivalent to:
    // auto p = f();
    // a = p.first;
    // b = p.second;
}
```

## 延迟求值

在编写模板是，还有一个很重要的问题是：**是否能够处理不完全类型？**
- 也就是实例化时能否使用不完全类型来实例化，比如在不同类之间存在相互依赖时、一个类依赖于一个使用了自己作为模板参数的模板时这是很重要的。
- 比如：
```C++
struct Node
{
    std::string value;
    Cont<Node> next;
};
```
- 这个定义中`Cont`必须要能够接受不完全类型作为模板参数才能这样做。
- 使用不完全类型有许多限制，比如只能使用指针、引用，不能使用任何成员。
- 使用了某些要求完全类型的类型特性也可能导致问题，比如：
```C++
template<typename T>
class Cont
{
private:
    T* elems;
public:
    typename std::conditional_t<std::is_move_constructible_v<T>, T&&, T&> foo();
}
```
- 因为`std::is_move_constructible`要求完全类型，所以这里不可行。
- 有一个手段解决这个问题：将成员函数定义为模板函数，将`T`作为默认模板实参，调用时还是一样，但将对完全类型的需求**延迟**（defer）到了`foo`成员函数模板实例化时：
```C++
template<typename T>
class Cont
{
private:
    T* elems;
public:
    template<typename D = T>
    typename std::conditional_t<std::is_move_constructible_v<D>, T&&, T&> foo();
}
```

## 编写泛型库时需要考虑的问题

回顾所有编写泛型库时需要考虑的问题：
- 使用万能引用在函数模板中进行完美转发，如果这些值不依赖于模板参数，那么使用`auto&&`配合万能引用转发临时变量。
- 函数模板中使用万能引用时，要考虑好模板参数被推导为引用的处理。
- 当在模板中对依赖于模板参数的对象取地址时，使用`std::addressof`以避免模板参数类型重载了`operator &`的影响。
- 对于函数模板，确保他们相比预定义的拷贝构造、赋值运算符不会匹配得更好。如果是，需要添加约束。
- 当模板参数类型可能是字符串字面量时，考虑使用`std::decay`。
- 当有参数按照引用传递作为输出或者输入输出参数时，考虑传递`const`对象导致模板实参被推导为`const`的情况。
- 考虑引用作为模板参数时的副作用与是否处理。尤其是，确保返回值类型是合理的。
- 考虑模板对不完全类型的支持。比如在递归数据结构中。
- 为所有数组类型重载或者特化，而不仅仅是`T[SZ]`，还有引用、未知边界的不完全类型。

## 总结

- 模板允许传递函数、函数指针、函数对象、lambda作为可调用对象。
- 定义函数对象类型时，将`operator()`定义为`const`。
- 使用`std::invoke`，可以统一普通函数对象和成员函数指针的调用。
- 使用`decltype(auto)`自动推导返回值类型以完美转发返回值（因`auto`会自动退化）。
- 使用类型特性可以检查类型属性、对类型做变换等。
- 使用`std::addressof`在模板中取一个依赖于模板参数的类型对象的地址。
- 在不求值表达式中使用`std::declval()`创建一个特定类型的值。
- 使用`auto&&`完美转发函数模板中不依赖于模板参数的临时对象。
- 注意处理模板参数是引用时的副作用，或者禁用掉。
- 可以使用模板延迟对使用到不完全类型的编译期表达式的求值以支持不完全类型。
