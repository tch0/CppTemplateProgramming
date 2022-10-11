<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十二章：桥接静态和动态多态](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E4%BA%8C%E7%AB%A0%E6%A1%A5%E6%8E%A5%E9%9D%99%E6%80%81%E5%92%8C%E5%8A%A8%E6%80%81%E5%A4%9A%E6%80%81)
  - [函数对象、函数指针和std::function<>](#%E5%87%BD%E6%95%B0%E5%AF%B9%E8%B1%A1%E5%87%BD%E6%95%B0%E6%8C%87%E9%92%88%E5%92%8Cstdfunction)
  - [广义函数指针](#%E5%B9%BF%E4%B9%89%E5%87%BD%E6%95%B0%E6%8C%87%E9%92%88)
  - [桥接接口（bridge interface）](#%E6%A1%A5%E6%8E%A5%E6%8E%A5%E5%8F%A3bridge-interface)
  - [类型擦除（type erasure）](#%E7%B1%BB%E5%9E%8B%E6%93%A6%E9%99%A4type-erasure)
  - [可选桥接（optional bridging）](#%E5%8F%AF%E9%80%89%E6%A1%A5%E6%8E%A5optional-bridging)
  - [性能考虑](#%E6%80%A7%E8%83%BD%E8%80%83%E8%99%91)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十二章：桥接静态和动态多态

[第十八章](../18PolymorphicOfTemplates)介绍了通过模板实现的静态多态与通过继承和虚函数实现的动态多态。
- 两种多态都提供了有力的抽象，也都有权衡和取舍：
    - 静态多态提供与非多态代码同样的性能，但是运行时使用的类型必须在编译期已知。
    - 动态多态的类型可以在运行时才知道，但是灵活性更低必须从基类派生，并且有虚调用的额外运行时消耗。
- 本章描述如何桥接动态多态和静态多态。提供一些两个模型的好处：更小的二进制尺寸和动态多态的编译期特性、静态多态的接口灵活性。
- 作为例子将会实现一个简化的`std::function<>`。

## 函数对象、函数指针和std::function<>

函数对象可以用来定制模板的行为。
- 以一个可调用对象作为模板参数可以接收多种可调用对象：lambda、函数指针、实现了`operator()`的函数对象、或者能够转化为函数指针或者引用的对象。
- 每一种可调用对象在模板中的使用都会产生一种模板实例，有可能会增大二进制尺寸。
- 为了限制二进制尺寸的膨胀，可以使用非模板代码，比如选择函数指针作为可调用对象类型，但是这样就不能接受其他类型的可调用对象了，比如lambda。
- 为了达到泛用性与避免二进制尺寸膨胀的目的，可以使用标准库提供的`std::function`。
- `std::function`提供了静态多态的特性——可以工作在无界集合上：函数指针、lambda、任意实现了`operator()`的函数对象。也提供了动态多态的特性，到运行时才知道具体是什么类型。
- 其使用了一种技术叫做**类型擦除（type erasure）**，通过类型擦除桥接了静态多态和动态多态。

## 广义函数指针

`std::function`是一种广义函数指针（generalized form of C++ function pointer），提供以下几个操作：
- 可以在调用者不知道其中的函数的情况下调用其中的函数。
- 可以被拷贝、移动、赋值。
- 可以被其他（兼容签名的）函数赋值。
- 拥有一个空状态（null state），这种状态下没有任何函数绑定到它。

这里实现一个`FunctionPtr`提供`std::function`的功能，接口是这个样子的：
```C++
template<typename Signature>
class FunctionPtr;

template<typename R, typename... Args>
class FunctionPtr<R(Args...)>
{
private:
    FunctionBridge<R, Args...>* bridge;
public:
    // constructors
    FunctionPtr() : bridge(nullptr) {}
    FunctionPtr(const FunctionPtr& other);
    FunctionPtr(FunctionPtr& other) : FunctionPtr(static_cast<const FunctionPtr&>(other)) {}
    FunctionPtr(FunctionPtr&& other) : bridge(other.bridge)
    {
        other.bridge = nullptr;
    }
    template<typename F> FunctionPtr(F&& f);
    // assignment
    FunctionPtr& operator=(const FunctionPtr& other)
    {
        FunctionPtr tmp(other);
        swap(*this, tmp);
        return *this;
    }
    FunctionPtr& operator=(FunctionPtr&& other)
    {
        delete bridge;
        bridge = other.bridge;
        other.bridge = nullptr;
        return *this;
    }
    template<typename F>
    FunctionPtr& operator=(F&& f)
    {
        FunctionPtr tmp(std::forward<F>(f));
        swap(*this, tmp);
        return *this;
    }
    // destructor
    ~FunctionPtr()
    {
        delete bridge;
    }
    friend void swap(FunctionPtr& fp1, FunctionPtr& fp2)
    {
        std::swap(fp1.bridge, fp2.bridge);
    }
    explicit operator bool() const
    {
        return bridge != nullptr;
    }
    R operator()(Args... args) const;
};
```
- 其中仅包含了一个非静态成员：`bridge`对象，负责内部函数对象的存储和操纵。
- 未实现部分接下来描述。

## 桥接接口（bridge interface）

`FunctionBridge`负责持有与操纵底层的函数对象。实现为一个抽象类模板，负责实现`FunctionPtr`的动态多态：
```C++
template<typename R, typename... Args>
class FunctionBridge
{
public:
    virtual ~FunctionBridge() {}
    virtual FunctionBridge* clone() const = 0;
    virtual R invoke(Args... args) const = 0;
};
```
- 其提供了必要接口：析构、拷贝、调用底层函数对象。

## 类型擦除（type erasure）

接下来实现`FunctionBridge`的派生类：
- 他们负责提供实现。
- 为了保证支持所有类型的派生类的无界（unbounded）集合，我们也需要无界数量的派生类——一个将底层函数对象类型参数话的派生类模板。
```C++
template<typename Functor, typename R, typename... Args>
class SpecificFunctionBridge : public FunctionBridge<R, Args...>
{
private:
    Functor functor;
public:
    template<typename FunctorFwd>
    SpecificFunctionBridge(FunctorFwd&& _functor) : functor(std::forward<FunctorFwd>(_functor)) {}
    virtual SpecificFunctionBridge* clone() const override
    {
        return new SpecificFunctionBridge(functor);
    }
    virtual R invoke(Args... args) const override
    {
        return functor(std::forward<Args>(args)...);
    }
};
```
- 然后补全`FunctionPtr`通过任意类型函数对象构造的逻辑，底层对象的类型就是退化之后的传入对象类型，以确保该对象能够被存储（去掉引用、CV限定，函数变为函数指针）。
```C++
template<typename R, typename... Args>
template<typename F>
Function<R, Args...>::FunctionPtr(F&& f) : bridge(nullptr)
{
    using Functor = std::decay_t<F>;
    using Bridge = SpecificFunctionBridge<Functor, R, Args...>;
    bridge = new Bridge(std::forward<F>(f));
}
```
- 每一次`SpecificFunctionBridge`对象被赋给`bridge`的时候，类型信息因为派生类到基类的转换丢失了。这就是**类型擦除**，桥接静态多态和动态多态的最常用的技术。
- 现在为止，我们的`FunctionPtr`基本可以替代标准库的`std::function<>`了。除了最后的`operator==`未实现。
- 完整代码见：[P517.FunctionPtr.cpp](P517.FunctionPtr.cpp)

## 可选桥接（optional bridging）

接下来实现判等操作：
- 首先需要给`FunctionBridge`添加接口：
```C++
virtual bool equals(const FunctionBridge* fb) const = 0;
```
- 并在`SpecificFunctionBridge`中实现。
```C++
virtual bool equals(const FunctionBridge<R, Args...>* fb) const override
{
    if (auto specFb = dynamic_cast<const SpecificFunctionBridge*>(fb))
    {
        return functor == specFb->functor;
    }
    return false; // functor with different types are never equal
}
```
- 然后实现`FunctionPtr`的`operator==`即可。
```C++
friend bool operator==(const FunctionPtr& f1, const FunctionPtr& f2)
{
    if (!f1 || !f2)
    {
        return !f1 && !f2; // both are empty
    }
    return f1.bridge->equals(f2.bridge);
}
```
- 这种实现方式有一个缺陷，对于定义了`operator()`的函数对象，可能没有定义`operator==`，这时会实例化失败。就算没有用到`FunctionPtr`的`operator==`，这是由于类型擦除的副作用导致的，因为实例化派生类时会实例化所有的虚函数。
- 我们需要SFINAE来解决：在底层函数对象不支持`operator==`时，不调用`operator==`，转而在运行时抛出异常（不适用也就不会抛出异常）。
```C++
// exception of no equality comparison operator
class NotEuqalityComparable : public std::exception {};

// try to compare Functor with SFINAE
template<typename T, bool = std::equality_comparable<T>>
struct TryEquals
{
    static bool equals(const T& x1, const T& x2)
    {
        return x1 == x2;
    }
};
template<typename T>
struct TryEquals<T, false>
{
    static bool equals(const T& x1, const T& x2)
    {
        throw NotEuqalityComparable();
    }
};
template<typename Functor, typename R, typename... Args>
bool SpecificFunctionBridge<Functor, R, Args...>::equals(const FunctionBridge<R, Args...>* fb) const
{
    if (auto specFb = dynamic_cast<const SpecificFunctionBridge*>(fb))
    {
        return TryEquals<Functor>::equals(functor, specFb->functor);
    }
    return false; // functor with different types are never equal
}
```

## 性能考虑

- 类型擦除同时具有静态多态和动态多态的好处，但不是所有好处。
- 使用类型擦除本质上还是要用虚函数和继承，所以说性能更接近动态多态。会比静态多态差一些，这个性能影响是否显著依赖于具体程序。
- 同理，某些静态多态的传统优势也可能没有了，比如某些内联调用可能不会再内联（因为变成虚调用了）。

## 后记

- Kevlin Henney在C++中引入`any`类型时推广了类型擦除技术，C++17中引入了`std::any`。
- Boost中的`Boost.TypeErase`库提供了对于类型擦除对象的模板元编程技术。
