<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十章：类型属性上的重载](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E7%AB%A0%E7%B1%BB%E5%9E%8B%E5%B1%9E%E6%80%A7%E4%B8%8A%E7%9A%84%E9%87%8D%E8%BD%BD)
  - [算法特化](#%E7%AE%97%E6%B3%95%E7%89%B9%E5%8C%96)
  - [标签分发](#%E6%A0%87%E7%AD%BE%E5%88%86%E5%8F%91)
  - [启用与禁用模板](#%E5%90%AF%E7%94%A8%E4%B8%8E%E7%A6%81%E7%94%A8%E6%A8%A1%E6%9D%BF)
  - [类模板特化](#%E7%B1%BB%E6%A8%A1%E6%9D%BF%E7%89%B9%E5%8C%96)
  - [实例化安全的模板](#%E5%AE%9E%E4%BE%8B%E5%8C%96%E5%AE%89%E5%85%A8%E7%9A%84%E6%A8%A1%E6%9D%BF)
  - [标准库中的实践](#%E6%A0%87%E5%87%86%E5%BA%93%E4%B8%AD%E7%9A%84%E5%AE%9E%E8%B7%B5)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十章：类型属性上的重载

同名的函数或者函数模板可以根据函数参数列表重载。一个很自然的想法是函数模板根据类型的属性重载：
```C++
template<typename Number> void f(Number); // only for Number
template<typename Container> void f(Container); // only for Container
```
- 但是C++是不能根据类型属性重载的，上述两个声明会被视为等同的。
- 虽然不能直接这样做，但是现实中，有很多技巧可以实现函数模板的按属性重载。

## 算法特化

为函数模板进行重载的一个常见动机是为某些特殊类型提供特化版本。
- 例子：
```C++
template<typename T>
void swap(T& x, T& y)
{
    T tmp = x;
    x = y;
    y = tmp;
}
template<typename T>
void swap(Array<T>& x, Array<T>& y)
{
    swap(x.ptr, y.ptr);
    swap(x.len, y.len);
}
```
- 为特殊的类型提供特化可能可以提升算法效率。
- 这种为泛型算法提供特化的优化手段叫做算法特化（algorithm specialization）。
- 这种特化只作用于泛型算法可以作用的一个子集，特化版本根据特定类型或者类型的特定属性来做选择。
- 实现时需要确保的是：特化版本一定要比泛型版本更加特化（根据C++偏序规则）。
- 不是所有的特殊算法都能通过C++偏序规则来直接实现。

## 标签分发

另一种实现算法特化的方式是标签分发（tag dispatching），主要用于直接重载函数模板无法区分有歧义的情况。手段是给函数模板添加一个参数，根据这个参数类型来重载：
- 例子：
```C++
template<typename Iterator, typename Distance>
void advanceIterImpl(Iterator& x, Distance n, std::iput_iterator_tag)
{
    while (n > 0)
    {
        ++x;
        --n;
    }
}
template<typename Iterator, typename Distance>
void advanceIterImpl(Iterator& x, Distance n, std::random_access_iterator_tag)
{
    x += n;
}
```
- 调用时则需要获取迭代器属性`std::iterator_traits<Iterator>::iterator_category`传入作为第三个参数。
- 当函数模板相关类型提供了这样一个标签时，就可以使用标签分发。
- 这样的技巧通常需要一层间接层次作为实现层，从接口层调用实现层时利用函数重载实现分发。

## 启用与禁用模板

我们需要基于模板参数的属性选择模板重载的手段，但是仅依赖函数模板重载的偏序或者重载决议并不能实现更高级的算法特化。
- 所以标准库提供了`std::enable_if`使我们可以禁用或者启用模板。
- `std::enable_if`的实现：
```C++
template<bool B, typename T = void>
struct enable_if {};
 
template<class T>
struct enable_if<true, T> { using type = T; };
```
- `std::enable_if`结合SFINAE就可以在特定条件下启用或者禁用模板了，前面有说过，不赘述。
- 注意多个重载如果形式完全相同，那么条件必须要是**互斥**（mutually exclusive）的，否则同时满足多个重载会有歧义。
- 标签分发与`enable_if`优缺：
    - 相比起标签分发，使用`enable_if`需要每次添加新重载时都回去检查甚至修改每个重载，保持他们的条件是互斥的。
    - 使用标签分发通常用于标签本身互斥或者有继承层次的情况，`enable_if`相比起来条件组合可以更加灵活。
- `enable_if`用在什么地方：
    - 通常会放到返回值中，但是某些函数比如构造函数、转换运算符是没有返回值类型的。并且放返回值会让函数变得难以阅读。
    - 大部分时候还是会放在默认模板参数中，用起来比较统一。但是如果我们想加一个重载，使用同样形式他们就会变成相同的声明，所以这种时候就需要再加一个无意义的带有默认实参的模板形参加以区分。
    ```C++
    template<typename A, typename = std::enable_if<xxx>>
    void f();
    template<typename A, typename = std::enable_if<yyy>, typename = int/*extra necessary dummy parameter to distinguish*/>
    void f();
    ```

编译期`if`：
- 在C++17之后，重载函数模板的函数参数列表相同时，`enable_if`可以完美转换为`if constexpr`语法。
- 现代C++中，这样写是最方便的。
- 这种改写只有在泛型组件的条件能够在函数体内完全表达出来时才能工作。
- 下列情况依旧需要继续使用`enable_if`：
    - 不同接口。
    - 类模板不同参数需要不同类定义。
    - 对于特定的模板实参列表不应该存在合法的实例化（可以一定程度使用`static_assert`解决，但因为不能将其从候选集合中排除，所以不能用于SFINAE的场景）。

概念：
- 前面的每种方法都能工作，但是他们或多或少都有些笨拙。
- 所以在支持C++20的环境中，最好的实践肯定是使用概念。
- 概念可以完全替代`enable_if`，并且相比`enable_if`，概念之间可以有包含关系，提供了一个自然的偏序，不是必须要互斥。
- 概念还可以被附加到一个模板化实体，但本身并不是模板的东西上（比如类模板的普通成员函数）。
- C++20中，概念配合`if constexpr`基本可以消除标签分发、`enable_if`这种古典技巧的使用。更安全和方便地编写模板代码。

## 类模板特化

函数模板为特定模板实参型提供特殊实现的机制是重载，而在类模板中，这个机制就是类模板偏特化。
- 要禁用或者启用特定的函数模板偏特化，同样可以使用`enable_if`。
- 引入做法通常是在主模板中添加一个带默认模板实参`void`的（未命名）模板形参。在偏特化中这个模板形参使用`enable_if`搭配相应的互斥的条件来填充。
- 这个新的模板实参作为`enable_if`的锚，不需要做其他任何事情。
- 和函数重载不一样的是，主模板不需要添加一个互斥的条件，因为所有偏特化的优先级都高于主模板。但是偏特化之间需要互斥。

类模板中的标签分发：
- 同样将主模板其中一个模板参数作为标签，实例化时将标签类型作为模板实参传入（或者利用类模板实参推导将标签类型推导出来）。
- 但需要注意一点的是，将标签作为类型传入就必须完美匹配，具有派生关系都是行不通的。
- 所以无法完美匹配时，可能还需要一个类似于`BestMatchInSet<InputTagType, TagsInSet...>`这样的辅助特征。要实现这个特征则可以借助函数重载来做，细节这里不赘述。
- 用起来还是略麻烦。

## 实例化安全的模板

`enable_if`技巧的本质是根据模板实参的某种特定条件启用特定函数模板重载或者类模板偏特化。
- 在条件不满足时直接不尝试进行实例化，直接报错或者忽略这个实现，就叫做实例化安全（instantiation-safe）。
- 虽然实例化也会进行报错，但是报错来源是不一样的，实例化的报错可能很长且令人难以理解，而不满足条件则会报告出不满足哪一个条件。
- 如果还有其他满足条件的重载，如果不是实例化安全的代码，可能压根就编不过。而实例化安全的代码在SFINAE机制下选择正确的版本。

## 标准库中的实践

- 标准库中提供了多种迭代器种类，使用`std::iterator_traits`可以提取他们。
- C++标准库中许多算法针对特定迭代器种类提供了不同实现，比如`std::distance std::advance`等。
- 标准库算法中许多算法也针对特定类型做了优化，比如`std::copy`针对平凡拷贝构造的类型做了特化，使用`std::memecpy() std::memmove()`来实现，`std::fill`针对平凡析构的类型做了特化，使用`std::memset()`来实现。
- 标准中所说的某个函数在某些情况下不参与重载决议就是使用`std::enable_if`实现的。

## 后记

发展历史，略。
