<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十八章：调试模板](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E5%85%AB%E7%AB%A0%E8%B0%83%E8%AF%95%E6%A8%A1%E6%9D%BF)
  - [浅式实例化](#%E6%B5%85%E5%BC%8F%E5%AE%9E%E4%BE%8B%E5%8C%96)
  - [静态断言](#%E9%9D%99%E6%80%81%E6%96%AD%E8%A8%80)
  - [原型](#%E5%8E%9F%E5%9E%8B)
  - [跟踪程序](#%E8%B7%9F%E8%B8%AA%E7%A8%8B%E5%BA%8F)
  - [Oracles](#oracles)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十八章：调试模板

调试模板时总是会遇到两类相反的问题：
- 如果确保编写出的模板对于所有满足特定条件的模板实参都能正常工作？
- 当模板未正常工作时如何找到模板实参未满足的那个条件？

在思考所有之前，区分约束的种类是有必要的：
- 语法约束（syntactic constraints）：如果违反则会报出编译错误的约束。
- 语义约束（semantic constraints）：违反了之后不会报出编译错误的约束。
- 举个例子：对于模板参数需要能够比较的约束，要求具有一个`operator<`是语法约束，而要求这个类型在其作用域上是严格弱序的则是语义约束。
- 本章主要讨论语法约束，语义约束本质上并没有好的方法来检测与证实。

## 浅式实例化

当模板实例化发生错误时，问题经常发生在一条很长的实例化链条上，通常都会报出非常冗长的错误信息。

浅式实例化是指在实例化的顶层插入不会参与执行的但是同底层满足相同约束的代码，提前触发实例化错误。
- 典型手段是：局部类（local class）。
- 很显然这增加了代码编写的复杂度，并且引入了无用的代码。
- 所以一个很典型的想法是将这些约束收集起来放到某种库中。C++20中某种程度上可以使用`<concept>`概念库做这件事情了。
- 可以使用静态断言代替。

## 静态断言

C++继承自C语言的`assert()`宏可以用以运行时断言，如果断言失败程序会中断在这个点以便程序员修复。

而C++11引入的`static_assert`关键字，同样的目的但是是在编译期断言，断言一个编译期表达式为真，如果为假，则会编译失败。
- 静态断言可以用于浅式实例化可以用的所有地方，通过定义一个特征或者概念，使用静态断言即可做到和浅式实例化一样的效果。
- C++20之后，配合概念，对于比较简单的情况可以直接对`requires`表达式做静态断言，都不需要定义为概念。

## 原型

在确定一个模板是否能够针对任何满足了约束的模板实参进行实例化时，可以定义一个原型（Archetype）进行测试。
- 比如一个函数`find`需要模板参数类型能够使用`operator==`比较，并且比较结果能够转换`bool`。
- 那么可以定义如下原型，进行实例化尝试：
```C++
class EqualityComparableArchetype {};
class ConvertibleToBoolArchetype
{
public:
    operator bool() const;
};
ConvertibleToBoolArchetype operator==(const EqualityComparableArchetype&, const EqualityComparableArchetype&);
```
- 如果能够实例化通过，说明模板实现正确。不能实例化，则说明模板中有其他多余的约束。
- 注意原型中也不应该添加多余功能。

## 跟踪程序

略，就是针对特定对象编写的跟踪程序，比如跟踪创建了多少对象、销毁了多少、复制了多少等。

## Oracles

略。

## 后记

略。