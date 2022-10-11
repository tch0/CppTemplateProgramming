<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十一章：模板与继承](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E4%B8%80%E7%AB%A0%E6%A8%A1%E6%9D%BF%E4%B8%8E%E7%BB%A7%E6%89%BF)
  - [空基类优化（EBCO）](#%E7%A9%BA%E5%9F%BA%E7%B1%BB%E4%BC%98%E5%8C%96ebco)
  - [奇异模板递归（CRTP）](#%E5%A5%87%E5%BC%82%E6%A8%A1%E6%9D%BF%E9%80%92%E5%BD%92crtp)
  - [混入（Mixin）](#%E6%B7%B7%E5%85%A5mixin)
  - [命名模板实参](#%E5%91%BD%E5%90%8D%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十一章：模板与继承

## 空基类优化（EBCO）

C++中经常出现类型是空的情况，也就是那种只有成员类型和静态数据成员、没有虚函数和非静态数据成员的类型。那么在运行时也就不需要任何存储空间。
- 在实现中，这样的类虽然理论上不需要任何存储空间，但是考虑到一个对象必须要有地址，要体现在内存中，所以空类的对象在编译器实现时普遍都会给一个字节的大小。
```C++
#include <iostream>

class A {};

int main(int argc, char const *argv[])
{
    std::cout << sizeof(A) << std::endl; // 1
    return 0;
}
```
- 部分编译器实现时甚至因为对齐要求会得到更大的长度（比如典型值4）。

布局原则：
- C++设计者有各种各样的原因需要给空对象一个非零的大小。
    - 比如，为了保持类对象数组操作的统一：取元素，求两元素距离等操作如果元素大小为0将会出现问题。
- 尽管C++中不能出现零大小的类型，但是C++标准却指定了当空类型作为基类时，这个空基类子对象可以没有大小。
- 称之为**空基类优化**（empty base class optimization），由编译器选择是否实现（一般都会）。
```C++
class A {};
class B : public A {};
class C : public B {};

int main(int argc, char const *argv[])
{
    std::cout << sizeof(A) << std::endl; // 1
    std::cout << sizeof(B) << std::endl; // 1
    std::cout << sizeof(C) << std::endl; // 1
    return 0;
}
```
- 考虑如下情况：
```C++
class D : public A, public B {}; // has size 2
```
- C++标准规定两个相同类型的不同基类子对象不能具有同一个地址（因为是非虚继承，所以不是同一个基类子对象，为了区分这两个基类子对象，他们需要有不同地址），所以`D`大小是2。
- 在实践中，很多类型可能会派生自同一个空基类，这一定程度上阻碍了空基类优化，这种时候也许需要虚继承。
- 但ECBO在模板编程中依然非常重要，因为模板库普遍会使用从空基类继承别名或者值的做法（典型如`std::true_type std::false_type`）。

成员作为基类：
- 空类型作为基类可以有空基类优化，但是作为成员则不行（因为会导致成员指针出现问题）。
- 但是我们可以将空类型成员作为基类，为了避免接口污染并维持`has-a`关系，需要使用`protected`或者`private`继承。
- 私有继承和保护继承作为HAS-A关系的类型组织手段，在实践中都可以被组合代替，所以一般可能很少会见到。空基类优化是使用保护继承和私有继承的一个重要原因（甚至可以说除此之外就没有其他特别的原因了）。
- libstdc++中的`std::vector`就使用了这个技巧，派生了`std::allocator`（这样派生本身看起来都是有点吊诡的就是了）。

## 奇异模板递归（CRTP）

奇异模板递归（Curiously Recurring Template Pattern）：
- 简单来说，就是将派生类作为基类模板参数的类模板编写模式：
```C++
template<typename Derived>
class CuriousBase
{
    ...
};
class Curious : public CuriousBase<Curious>
{
    ...
};
```
- 派生类也是类模板时的形式：
```C++
template<typename Derived>
class CuriousBase
{
    ...
};
template<typename T>
class CuriousTemplate : public CuriousBase<CuriousTemplate<T>>
{
    ...
};
```
- 通过将派生类作为模板参数传入基类模板，基类就可以根据派生类在不需要虚函数的情况下定制自己的行为（比如调用派生类的方法）。
- 一个简单应用：统计当前存在多少特定类型的对象。
```C++
template<typename CountedType>
class ObjectCounter
{
private:
    inline static std::size_t count = 0;
protected:
    ObjectCounter() { ++count; }
    ObjectCounter(const ObjectCounter&) { ++count; }
    ObjectCounter(ObjectCounter&&) { ++count; }
    ~ObjectCounter() { --count; }
public:
    static std::size_t live() { return count; }
};
```

**Barton-Nackman技巧**：
- 1994年John J.Barton和Lee R.Nackman提出了一种称之为受限模板扩展（restricted template expansion）的技术（即是Barton-Nackman技巧）。
- 为了说明这个东西，定义一个类模板以及一个运算符：
```C++
template<typename T>
class Array
{
public:
    ...
};
template<typename T>
bool operator==(const Array<T>& a, const Array<T>& b)
{
    ...
}
```
- 但在上古C++中函数模板不能重载时，这样写有一个问题，在这个作用域中就不能定义其他`operator==`了。
- Barton和Nackman通过将`operator==`定义为一个普通友元函数解决了这个问题：
```C++
template<typename T>
class Array
{
    friend bool operator==(const Array<T>& a, const Array<T>& b)
    {
        ...
    }
public:
    ...
};
```
- 此时`operator==`是类模板的一个普通友元函数，而非函数模板。当类模板被实例化时，作为实例化的副作用，这个非模板友元函数被注入到外层作用域。因为它是非模板函数，所以可以重载。
- 在不支持函数模板重载时，通过这个技巧解决了不能重载的问题。称之为Barton-Nackman技巧，也称之为**受限模板扩展**（如果是函数模板，能直接应用于所有类型，则是不受限模板扩展）。
- 在它发明的时候，友元可以在类模板实例化时在其外层封闭作用域内可见，称之为**友元名称注入**（friend name injection）。
- 1994年之后，友元函数定义的名称查找规则发生了改变。现在是通过参数依赖查找（ADL）来查找友元了。这意味着至少需要有一个函数调用的参数类型将这个友元函数作为友元。如果参数类型仅仅是能够转换为定义了友元的类型，那么通过ADL是查找不到这个友元的（而直接注入外层作用域是能够查找到的）。
```C++
#include <iostream>

class S {};

template<typename T>
class Wrapper
{
private:
    T object;
public:
    Wrapper(T obj) : object(obj) {}
    friend void foo(const Wrapper<T>&)
    {
        std::cout << "foo" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    S s;
    Wrapper<S> w(s);
    foo(w);
    // foo(s); // ERROR: can not find foo through ADL
    return 0;
}
```
- 所以定义友元时是定义函数模板的特化为友元，还是定义依赖类模板参数的普通函数为友元，是有一定区别的。前者因为在外层作用域可见，所以允许转换。后者因为必须经过ADL，经过转换则查找不到。
- 友元在CRTP中也很有用。

运算符实现：
- 在现实情况中，如果定义了一个运算符，很有可能要定义相关的其他运算符，比如定义了`==`，很有可能还要定义`!=`，定义了`<`，那么定义`<= > >=`也是合理的。
- `<utility>`头文件中`std::rel_ops`命名空间中定义了许多运算符，比如用`==`实现`!=`，用`<`实现`<= > >=`。不过C++20因为让位给三路比较`<=>`所以弃用了。
- CRTP以及Barton-Nackman技巧结合可以很方便地定义依赖于其他运算符的运算符，而不需要每次都重复编写：
```C++
template<typename Derived>
class EqualityCompare
{
    friend bool operator!=(const Derived& x1, const Derived& x2)
    {
        return !(x1 == x2);
    }
};

class X : public EqualityCompare<X>
{
    friend bool operator==(const X& x1, const X& x2)
    {
        return true;
    }
};
```
- CRTP和Barton-Nackman技巧结合是库作者非常喜爱的一种写法。

**Facades**：
- 在上面所说的运算符实现技巧上更进一步，可以使用CRTP基类定义大部分或者全部的公有接口，然后由派生类来实现剩余的小部分接口，称之为门面模式（**facade pattern**，或者叫表面模式？）
- 这种方式在定义需要满足一些现存接口要求的新类型时非常有用，比如数值类型、迭代器类型、容器类型等。
- 这里用迭代器作为例子：使用这种模式将大幅降低满足特定要求的迭代器的实现复杂度。
- 实现，迭代器的接口：
```C++
template<typename Derived, typename Value, typename Category, typename Reference = Value&, typename Distance = std::ptrdiff_t>
class IteratorFacade
{
public:
    using value_type = std::remove_reference_t<Value>;
    using reference = Reference;
    using pointer = Value*;
    using difference_type = Distance;
    using iterator_category = Category;
    // input iterator interface
    reference operator*() const;
    pointer operator->() const;
    Derived& operator++();
    Derived operator++(int);
    ...
    // bidirectional iterator interface
    Derived& operator--();
    Derived operator--(int);
    // random access iterator interface
    reference operator[](difference_type n) const;
    Derived& operator+=(difference_type n);
    ...
    friend bool operator==(IteratorFacade& lhs, IteratorFacade& rhs);
    friend difference_type operator-(const IteratorFacade& lhs, const IteratorFacade& rhs);
    friend bool operator<(const IteratorFacade& lhs, const IteratorFacade& rhs);
};
```
- 然后将这些操作委托给一套核心接口：`dereference increment equals`（输入迭代器） `decrement`（双向迭代器） `advace measureDistance`（随机访问迭代器），上述所有接口都可以通过这套核心接口实现，然后在派生类中实现这些接口即可，细节略。
- 实现时只需要提供对应层级的接口即可，如果调用了更高层级，则会实例化失败。
- 使用这一套骨架来定义迭代器适配器也是很方便的。

## 混入（Mixin）

混入（Mixin）：
- 首先考虑一个场景，定义一个多边形类，这个类是一系列点的序列：
```C++
class Point
{
public:
    double x, y;
    Point(double _x = 0.0, double _y = 0.0) : x(_x), y(_y) {}
};

class Polygon
{
private:
    std::vector<Point> points;
public:
    ...
};
```
- 现在考虑我们想定制点的属性，比如颜色、标签之类，可以将`Polygon`改成类模板，并且要求模板参数从`Point`派生以遵守`Polygon`使用的接口。
```C++
template<std::derived_from<Point> T>
class Polygon
{
private:
    std::vector<T> points;
public:
    ...
};
```
- 但是这种做法有一些缺点：
    - `Point`定义必须暴露给用户。
    - 如果`Point`定义发生修改，那么所有派生类定义都需要修改。
- 如果使用混入（Mixin），则可以实现同样功能的同时避免以上的缺点：将用户提供的功能定义为类，并作为`Point`的基类，而不是由用户来派生`Point`。
```C++
template<typename... Mixins>
class Point : public Mixins...
{
public:
    double x, y;
    Point(double _x = 0.0, double _y = 0.0) : x(_x), y(_y) {}
};

template<typename... Mixins>
class Polygon
{
private:
    std::vector<Point<Mixins...>> points;
public:
    ...
};
```
- 将要混入`Point`类的类型作为`Polygon`的模板参数即可做到。同时即避免了`Point`接口暴露，`Point`修改也不会影响到用户代码。

奇异模板混入（Curious Mixins）：
- 将奇异模板递归与混入结合起来。
- 就上面的例子，`Point`是派生类，同时将所有的混入类定义为类模板，并以`Point`作为模板参数：
```C++
template<template<typename>... Mixins>
class Point : public Mixins<Point>...
{
public:
    double x, y;
    Point(double _x = 0.0, double _y = 0.0) : x(_x), y(_y) {}
};
```
- 现在混入的类型就能够使用派生类的操作了，非常方便。（但需要注意的是使用的同时加重了耦合）。

参数化的虚拟性（parameterized virtuality）:
- 混入同时也允许我们间接地参数化派生类的其他属性，比如成员函数的虚拟性（是否为虚的）。
- 例子：
```C++
#include <iostream>
#include <memory>

class NonVirtual {};
class Virtual {
public:
    virtual void foo()
    {
        std::cout << "Virtual::foo()" << std::endl;
    }
};

template<typename... Mixins>
class Base : public Mixins...
{
public:
    void foo()
    {
        std::cout << "Base::foo()" << std::endl;
    }
};

template<typename... Mixins>
class Derived : public Base<Mixins...>
{
public:
    void foo()
    {
        std::cout << "Derived::foo()" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    std::shared_ptr<Base<NonVirtual>> p1 = std::make_shared<Derived<NonVirtual>>();
    p1->foo(); // Base::foo()
    std::shared_ptr<Base<Virtual>> p2 = std::make_shared<Derived<Virtual>>();
    p2->foo(); // Derived::foo()
    return 0;
}
```
- 这个技巧可以用于设计可以同时用于具体类或者使用继承扩展的类的类模板。
- 当然现实编码时更好的做法可能还是将用于具体类和用于可继承类的两个类模板分开设计，实际使用场景存疑。

## 命名模板实参

类模板设计时经常会有很多模板参数，并且很多都会有默认模板实参，指定一个模板实参就需要指定它前面的所有模板参数，即使他们已经有了默认实参，一个很自然的想法的是使用命名模板实参。
- 当前的C++语法并不直接提供支持，所以需要自己实现。
- 细节略，我感觉没什么卵用，用到了虚继承以及一个很复杂的派生层次，仅仅为了实现命名模板实参这一个功能。

## 后记

发展史，略。
