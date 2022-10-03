<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十八章：模板的多态性](#%E7%AC%AC%E5%8D%81%E5%85%AB%E7%AB%A0%E6%A8%A1%E6%9D%BF%E7%9A%84%E5%A4%9A%E6%80%81%E6%80%A7)
  - [动态多态](#%E5%8A%A8%E6%80%81%E5%A4%9A%E6%80%81)
  - [静态多态](#%E9%9D%99%E6%80%81%E5%A4%9A%E6%80%81)
  - [动态与静态多态对比](#%E5%8A%A8%E6%80%81%E4%B8%8E%E9%9D%99%E6%80%81%E5%A4%9A%E6%80%81%E5%AF%B9%E6%AF%94)
  - [概念(Concepts)](#%E6%A6%82%E5%BF%B5concepts)
  - [设计模式的新形式](#%E8%AE%BE%E8%AE%A1%E6%A8%A1%E5%BC%8F%E7%9A%84%E6%96%B0%E5%BD%A2%E5%BC%8F)
  - [泛型编程](#%E6%B3%9B%E5%9E%8B%E7%BC%96%E7%A8%8B)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

本章是第三部分的开始，主要关注于利用模板进行的程序设计技术，而非具体的语法细节。第三部分内容：
- 高级多态分发。
- 使用traits的泛型编程。
- 处理重载与继承。
- 元编程。
- 异质数据结构与算法。
- 表达式模板。
- 另外再介绍一些模板调试技巧。

# 第十八章：模板的多态性

多态（Polymorphism）是将一种泛型表示结合到多种不同的特定行为的能力。多态也是面向对象编程的基石，C++中通过类的继承和虚函数实现多态。因为这种多态是在运行时处理的，所以也称为动态多态（dynamic polymorphism），但在C++中讨论多态大多都是指动态多态。除此之外，也有在编译期处理的多态，称之为静态多态（static polymorphism）。

## 动态多态

就是继承和虚函数那一套东西，不赘述。有一个值得一提的好处是，使用动态多态就能够处理异质集合（Heterogeneous collections）了。

## 静态多态

静态多态，就是字面意思，同一套泛型代码可以对多套类型参数有效，只需要泛型代码中用到的东西类型参数都支持就行，比如对于函数只需要有同样的函数名和参数列表即可，不需要派生自同一个基类并重写虚函数。这即是静态多态，含义非常简单。实例化后多态类型参数的实例是互不相关的，具体调用哪一个是在实例化时也就是编译期就决定了。

动态多态是不能简单处理异质集合的，编译期决定了类型就只能使用该类型实例化集合类，而不能保存其他类型的对象。好处是可以存储对象，而不需要指针来实现动态多态了，而且获得了性能提升与类型安全。

## 动态与静态多态对比

术语：
- 动态多态：通过继承实现的动态多态是有界（bounded）的和动态的（dynamic）。
    - 有界是指这些类型能够使用的行为已经预先由公共基类决定了。
    - 动态是指接口的绑定在运行时动态地完成。
- 使用模板实现的静态多态是无界的（unbounded）和静态的（static）。
    - 无界指这些类型的多态行为的接口并不会被预先决定，所以也叫做**非侵入式**。
    - 静态则是指接口的绑定发生在编译期。
- 更准确地说，C++中的静态多态应该叫做无界静态多态，动态多态应该叫做有界动态多态。某些语言中还提供其他组合，比如Smalltalk提供无界动态多态。当然在C++中静态多态和动态多态并不会产生歧义。

优点与缺点：
- 动态多态：
    - 异质数据结构可以轻松处理。
    - 生成的二进制更小（而模板则会为每组类型生成实例）。
    - 代码可以被完全编译，不需要发布实现代码（而模板则必须公开实现）。
- 静态多态：
    - 类型的公有特性不需要被表达为一个公共基类。
    - 生成的代码可能更快，没有虚调用的间接层次。
- 通常来说，我们认为静态多态更加类型安全，而动态多态的分发都在运行时，可能会出现问题（比如保存了一个非法类型的指针）。
- 但是使用动态多态的心智负担会小一点，因为动态多态的接口都在基类中一眼可见，而静态多态则可能需要看文档与实现才能知道一个模板需要模板参数提供哪些东西有哪些约束。

组合这两种多态：22章将会介绍CRTP（Curiously recurring template pattern）的使用。

## 概念(Concepts)

静态多态中，模板参数需要满足一定要求，这些要求并不会被一套公共的接口所定义，而是由模板的有效表达式决定，如果不满足约束，可能会抛出难以理解的错误。
- 为了解决这一点，C++20引入了概念对模板的约束进行规范的说明。
- 概念代表了一个为了成功实例化模板，模板参数所需要满足的约束集合。
- 概念可以被理解为一种静态多态的接口。
- 举个例子：
```C++
template<typename T>
concept GeoObj = requires(T x)
{
    { x.draw() } -> void;
    { x.center_of_graavity() } -> Coord;
};
```
- 这个concept `GeoObj`表示：需要无参数的成员函数`draw`，其返回值是`void`，以及无参数成员函数`center_of_graavity`返回`Coord`类型。
- 使用时：
```C++
template<typename T>
requires GeoObj<T>
void myDraw(const T& obj)
{
    obj.draw();
}
```
- 这种解决方式对要参与到静态多态中的类型来说是非侵入式（non-invasive）的。
- 更多细节见[附录E](../AppendixE)。

## 设计模式的新形式

C++模板的静态多态能力给经典设计模式提供了新的实现方式。
- 比如桥接模式，将接口实现委托给实现类，实现类可变并 继承于同一接口类。有了静态多态之后将实现作为模板参数就不需要继承自同一接口类了，不过有个问题就是不能运行时更改了。

## 泛型编程

泛型编程没有一个公认的概念，大体上是说寻找高效算法、数据结构和其他软件概念的抽象表示以及他们的系统组织的计算机科学子学科。
- 在C++中，泛型编程也被定义为模板编程。
- 某种程度上来说，使用模板编程都可以称为泛型编程，但是很多人认为，还有一点很重要：模板应该被设计在一个框架中，并且允许相当程度的灵活组合。
- 目前来说，C++泛型编程最著名的实践是STL。
- STL中，提供了大量泛型算法，操作迭代器，迭代器在任意泛型容器中提供，可以相当灵活地组合起来。这些东西都很基础，不赘述。

## 后记

略。