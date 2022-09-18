<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十章：基本模板术语](#%E7%AC%AC%E5%8D%81%E7%AB%A0%E5%9F%BA%E6%9C%AC%E6%A8%A1%E6%9D%BF%E6%9C%AF%E8%AF%AD)
  - [类模板还是模板类？](#%E7%B1%BB%E6%A8%A1%E6%9D%BF%E8%BF%98%E6%98%AF%E6%A8%A1%E6%9D%BF%E7%B1%BB)
  - [替换、实例化、特化](#%E6%9B%BF%E6%8D%A2%E5%AE%9E%E4%BE%8B%E5%8C%96%E7%89%B9%E5%8C%96)
  - [声明与定义](#%E5%A3%B0%E6%98%8E%E4%B8%8E%E5%AE%9A%E4%B9%89)
  - [一个定义原则（One Definition Rule）](#%E4%B8%80%E4%B8%AA%E5%AE%9A%E4%B9%89%E5%8E%9F%E5%88%99one-definition-rule)
  - [模板实参与模板形参](#%E6%A8%A1%E6%9D%BF%E5%AE%9E%E5%8F%82%E4%B8%8E%E6%A8%A1%E6%9D%BF%E5%BD%A2%E5%8F%82)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十章：基本模板术语

## 类模板还是模板类？

C++中将类、结构、联合（union）称为**类类型**（class type），类这个词的含义则是用`class`或者`struct`定义出来的类类型，注意类这个词不包括联合。

和模板结合时：
- **类模板**（class template）是说这个类是一个模板，是对参数化的一簇类的描述。
- 而**模板类**（template class）这个词通常用于以下场合：
    - 类模板的同义词。
    - 表示使用模板实例化得到的类。
    - 表示一个模板名称、`<>`以及模板参数组合得到的一个名称。
    - 第二个和第三个含义的区别比较微妙，也没有那么大必要区分。
- 无论是C++社区中还是C++标准中，这些术语的含义并没有那么统一。
- 这本书中不使用模板类这个名词，而是使用类模板。（类模板并不是类，而是一个生成类的模板，用模板类来表示类模板在我看来是不严谨的，当然我很多时候也会用模板类表示类模板实例化得到的类）。
- 同理的其他名词：
    - 函数模板（function template）。
    - 成员模板（member template）。
    - 成员函数模板（member function template）。
    - 变量模板（variable template）。

## 替换、实例化、特化

替换（Substitution）：
- 这是使用模板实参去替换模板形参的过程，很多时候这种替换可能只是暂定的，还需要检查替换后模板是否合法。
- 替换是重载决议或者模板实例化的一个中间步骤。
- 通常来说在提到SFINAE时才需要注意这个过程。

实例化（Instantiation）：
- 为一个普通类创建一个实例的过程称之为实例化（对象实例化、变量实例化）。
- 用在模板中时，称为**模板实例化**（template instantiation），是指通过使用模板实参替换模板形参为模板别名、函数模板、类模板、成员函数模板、变量模板创建出一个实际的定义的过程。
- 值得注意的是，当前来说，通过使用模板实参替换模板形参来创建一个模板声明的过程并没有公认的术语，有人称之为部分实例化，也称之为声明的实例化，或者叫做不完全实例化（比如对于类模板就会生成一个不完全类）。

特化（Sepcialization）：
- 通过模板实例化或者不完全实例化得到的实体称之为模板的一个**特化**。
- 在C++中除了实例化还有其他生成特化的方式，也即是为已有模板显式地声明一种特殊模板实参，这种特化以`tmplate<>`作为开头：
```C++
template<typename T1, typename T2>
class MyClass {
    ...
};
template<>
class MyClass<int, bool> {
    ...
};
```
- 严格来说，这种特化叫做**显式特化**（更准确地说，显式全特化），而由实例化生成的则称为生成特化或者实例特化（instantiated or generated specialization）。
- 如果特化之后依然含有模板参数，则称之为**偏特化**（partial specialization，通常也广泛称之为部分特化）。
```C++
template<typename T1, typename T2>
class MyClass {
    ...
};
template<typename T>
class MyClass<int, T> {
    ...
};
```
- 解释：
    - 显式特化可以表示生成的那个实体，或者表示这个过程，或者表示用于生成实体的那个声明。
    - 与之对应的实例化生成的特化，实例化才是那个过程，特化是生成的结果也即是生成的实体。
    - 而偏特化则没有了生成的结果，就仅仅表示这个过程和这个声明。
- 当我们说显式特化或者偏特化时，对应的那个更一般的模板被称之为**主模板**（primary template）。

## 声明与定义

声明（declaration）和定义（definition）这两个词在C++中有着非常准确的定义：
- **声明**：一个在C++作用域中引入或者重新引入名称的C++结构（a C++ construct that introduces or reintroduces a name into a C++ scope）。比如：
```C++
class C;
void f(int p);
extern int v;
```
- **定义**：当一个声明附加上能够知道它的结构的所有信息后，它将变成定义。
    - 对于变量：提供初始化或者没有`extern`声明符会让变量声明成为定义，这时其内存必须被分配。
    - 对于类型定义：必须提供一个`{}`包围的定义。
    - 对于函数定义：提供函数体或者`=default =delte`声明。
- 定义通常来说也是一种声明，而声明则不一定是定义。

完全类型（Complete type）与不完全类型（Incomplete type）：
- 完全类型和不完全类型是一组类似于定义和声明的概念。
- 不完全类型：
    - 一个只被声明但是没有定义的类。
    - 一个不确定大小的数组类型。
    - 一个不完全类型的数组类型。
    - `void`
    - 一个底层类型或者枚举值未定义的枚举类型。
    - 所有上述类型的`const/volatile`修饰版本。
- 例子：
```C++
class C;            // C is incomplete type
C const* cp;        // cp is a pointer to incomplete type
extern C elem[10];  // elems has an incomplete type
extern int arr[];   // arr has an incomplete type

class C {};         // C now is a complete type
int arr[10];        // arr now has a complete type
```

## 一个定义原则（One Definition Rule）

C++中对同一个实体的重新声明有很多限制，总体体现为**一个定义原则**（One Definition Rule，ODR）。在不同的场景中这个原则会有不同的体现，[附录A](../AppendixA/)会详细阐述。知道基本原则已经足够：
- 普通（即非模板）非`inline`函数、成员函数、非`inline`全局变量、非`inline`静态数据成员在整个程序中只应该定义一次。（C++17引入`inline`变量）
- 类类型（包括结构和联合）、模板（包括偏特化，但是不包括全特化）、内联函数、内联变量至多只能在每个编译单元定义一次，并且所有这些定义必须完全相同。

可链接实体（linkable eneity）：函数、成员函数、全局变量、静态数据成员，包括通过模板实例化得到的所有这些东西。

## 模板实参与模板形参

- 模板形参是在关键字`template`后在模板声明和定义中的名称。
- 模板实参是模板实例化时用来替换模板形参的类型或者编译期表达式。

## 总结

- 使用模板时，我们使用术语类模板、函数模板、变量模板来描述。
- 模板实例化是使用模板实参替换模板形参的过程，得到的实体叫做特化。
- 类型可以是完全类型或者不完全类型。
- 根据一个定义原则，非内联函数、成员函数、非内联全局变量、非内联静态成员变量在整个程序中只能定义一次。
