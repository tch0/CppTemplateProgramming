<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十三章：模板中的名称](#%E7%AC%AC%E5%8D%81%E4%B8%89%E7%AB%A0%E6%A8%A1%E6%9D%BF%E4%B8%AD%E7%9A%84%E5%90%8D%E7%A7%B0)
  - [名称分类](#%E5%90%8D%E7%A7%B0%E5%88%86%E7%B1%BB)
  - [名称查找](#%E5%90%8D%E7%A7%B0%E6%9F%A5%E6%89%BE)
  - [解析模板](#%E8%A7%A3%E6%9E%90%E6%A8%A1%E6%9D%BF)
  - [继承与类模板](#%E7%BB%A7%E6%89%BF%E4%B8%8E%E7%B1%BB%E6%A8%A1%E6%9D%BF)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十三章：模板中的名称

C++是上下文敏感的语言，一个C++语法结构能够被理解的前提是知道足够的上下文信息。比如`x*y`可以是一个指针声明或者乘法表达式。在模板中，将面临比普通C++代码更多的上下文信息：
- 模板定义的上下文。
- 模板实例化的上下文。
- 模板实例化的实参的上下文。

## 名称分类

C++采用多种分类方式对名称进行分类：
- 讨论分类之前需要先明确两个概念：
    - 限定名称（修饰过的名称，qualified name）：使用作用域运算符`::`或者取成员运算符`. ->`修饰过的名称。这种名称明确了其作用域。
    - 依赖名称（非独立名称，dependent name）：依赖于某个模板参数的名称，比如`std::vector<T>::iterator`。
- 分类：

|分类|解释
|:-|:-
|标识符（Identifier）|一个连续的字符序列，有字母、数字、下划线组成，不能使用数字开头。某些标识符是为实现保留的，不应该使用。另外单下划线和双下划线开头的标识符是为标准库保留的，不应该使用。
|运算符函数id（Operator-function-id）|由关键字`operator`开头的，比如`operator new` `operator []`
|转换函数id（conversion-function-id）|用户定义转换运算符，比如`operator int&`
|字面量运算符id（literal-operator-id）|用户定义字面量运算符，比如`operator ""_km`
|模板id（template-id）|模板名称后跟角括号`<>`包围起来的模板实参列表，比如`std::vector<int>`。一个模板id同时可以是运算符函数id或者字面量运算符id，比如`operator+<X<int>>`
|未限定id（unqualified-id）|一般化的标识符，可以是上述所有类型的标识符，运算符函数id，转换函数id等，甚至可以是一个析构函数名称`~SomeClass`。
|限定id（qualified-id）|一个未限定名称经过类名、枚举、作用域限定，或者仅仅是全局作用域限定，比如`::x Array<T>::y ::N::Z`。
|限定名称（qualified name）|标准中未定义这个概念，这里用来指明经历限定查找（qualified lookup）的名称。具体来说是指限定id以及未限定id经过取成员运算符`. ->`修饰后的名称。
|未限定名称（unqualified name）|不是限定名称的名称，这里用来指定经历未限定查找（unqulified lookup的名称，也是非标准概念。
|名称（name）|限定名称或者未限定名称。
|依赖名称（或者叫非独立名称，dependent name）|以某种方式依赖于模板形参的名称。典型如显式包含一个模板实参的限定名称或者未限定名称是非独立名称。通过`. ->`限定的限定名称如果左边是非独立的类型那么也是非独立名称，比如类模板中的`this->b`。另外仅当参数表达式是非独立类型时且服从参数依赖查找（Argument Dependent Lookup，ADL）的名称是非独立名称。
|独立名称（Nondependent name）|不是非独立名称，那么就是独立名称。

以上术语在C++模板中将会非常频繁使用，前面已经接触过一些了，比如template-id。

## 名称查找

C++的名称查找规则有许多细节，这里仅介绍一些主要的概念。
- 仅需要知道：
    - 一般的情况都是符合直觉的处理。
    - 一些比较病态的情况标准会覆盖说明。
- **限定名称（qualified name）会在其隐含的结构中查找**。如果这个结构是一个类，那么其基类可能也会被查找。而不会直接在它出现的作用域中查找。
- **非限定名称（unqualified name）则会从其出现的作用域一层一层往外查找**。（在成员函数作用域中则是从函数作用域到类作用域，然后基类作用域，然后才是外层作用域）。这种查找称为普通查找（ordinary lookup）。
- 非限定名称的一个例外是**参数依赖查找**（Argument-dependent lookup，ADL，C++98/03中也叫做Koenig lookup，来自这个机制的提名者Andrew Koenig）。
- 例子：
```C++
#include <iostream>
#include <string>
namespace impl
{
class BigNumber
{
    friend bool operator<(const BigNumber& lhs, const BigNumber& rhs)
    {
        return lhs.num < rhs.num;
    }
    friend std::ostream& operator<<(std::ostream& os, const BigNumber& number)
    {
        return os << number.num;
    }
private:
    std::string num;
public:
    BigNumber(const std::string& _num) : num(_num) {}
};
}

template<typename T>
T max(T a, T b)
{
    return a < b ? b : a;
}

using impl::BigNumber;

int main(int argc, char const *argv[])
{
    BigNumber a("100"), b("101");
    std::cout << ::max(a, b) << std::endl;
    return 0;
}
```
- 友元虽然定义在类内，但其实其定义是位于`impl`命名空间内的。这里`::max`调用时用到的`operator<`和最后用于输出的`operator<<`都不在全局作用域。就是通过参数依赖查找找到的。

参数依赖查找：
- ADL主要应用在查找看起来是一个非成员函数或者操作符的非限定名称时。
- 如果普通查找已经找到了以下名称，则不会再运用参数依赖查找：
    - 成员函数名称。
    - 变量名称。
    - 类型名称。
    - 块作用域的函数声明的名称。
- 另外如果调用的函数名称被括号包围起来，也不会进行ADL。
- 除了上述例外之外，ADL会在调用参数关联的命名空间（associated namespaces）和关联类（associated classes）中进行查找。
- 关联命名空间和关联类的集合由以下规则决定：
    - 内置类型，集合为空。
    - 指针和数组类型，是底层类型的关联命名空间和关联类。
    - 枚举类型，是枚举类型声明的命名空间。
    - 对于类成员，其外层的类是关联类。
    - 对于类对象（包括union），关联类是其本身，其外部类（如果是嵌套类的话），和所有直接与间接基类的集合。关联命名空间则是所有这些类声明的命名空间。如果这个类是一个类模板，那么类模板的模板类型参数也是关联类，并且模板模板类型参数本身和其所在命名空间也在关联类集合和关联命名空间集合中。
    - 对于函数类型，这个集合包括函数参数类型和返回值类型所关联的类和命名空间。
    - 对于某个类的成员指针类型，则该类和其所在命名空间都在集合中，并且如果是数据成员指针，那么成员类型也在，如果是成员函数指针，那么其参数类型和返回值类型也会在其中。
- ADL在查找名称时会在所有关联命名空间中查找，就像这个名称被这些命名空间修饰了一样。
- 值得注意的是，ADL查找时会忽略关联命名空间中的`using`指令。
- 例子：
```C++
#include <iostream>

namespace X
{
template<typename T>
void f(T)
{
    std::cout << "template<typename T> void f(T)" << std::endl;
}
} // namespace X


namespace N
{
using namespace X; // ignored when ADL
enum E {e1};
void f(E)
{
    std::cout << "N::f(N::E)" << std::endl;
}
} // namespace N

void f(int)
{
    std::cout << "::f(int)" << std::endl;
}

int main(int argc, char const *argv[])
{
    ::f(N::e1); // qualified name, just ordinary lookup, no ADL
    f(N::e1); // unqualified name: ordinary lookup finds ::f() and ADL find() N::f(), N::f() is preferred
    return 0;
}
```
- 注意非限定名称才执行ADL，后者在执行`f(N::e1)`时找到了`::f`以及`N::f`，两者构成重载，并且后者匹配更佳。如果相同程度匹配，则无法选择从而报错。

友元声明的ADL：
- 友元函数声明可以是函数的第一次声明，这时会假定函数出现在最近的外层命名空间作用域（可能是全局作用域）。
- 但是这样的唯一的一个友元函数声明并不会直接让这个友元函数暴露在外层作用域。
- 例子：
```C++
template<typename T>
class C
{
    friend void f()
    {
        std::cout << "void f()" << std::endl;
    }
    friend void f(const C<T>*)
    {
        std::cout << "void f(const C<T>*)" << std::endl;
    }
};

void g(C<int>* p)
{
    // f(); // f was not declared
    f(p); // valid
}
```
- 这里第二个调用会通过ADL找到友元函数，但是第一个调用无法通过ADL找到，并且只是假定`f()`出现在外层作用域，并不是等价于`f()`就真的出现在外层作用域。
- 并且一个调用如果涉及对友元函数的查找，则会造成类模板的实例化（如果还没有进行实例化的话）。
- 一些时候通过ADL查找友元声明和定义也叫做友元名称注入（friend name injection）。【但这个名称有点误导，因为并没有实际注入到外层作用域，普通查找找`void f()`还是找不到的。】

注入类名：
- 类名本身会被注入到类自身中作为一个未修饰名称。通过修饰名称来访问时则访问不到。
- 类模板同样会注入类名，不同于普通类名称，类模板名称可以添加模板实参，成为注入类模板名称。
- 如果注入类模板名称没有跟参数列表，则代表将类模板形参作为其实参。在偏特化中就是特化实参。在期望模板的上下文中表示模板，在期望类的上下文中则表示类。
- 如果在类模板`C`中使用`::C`则不是注入类名，总是代表类模板，而不会代表类本身。

当前实例化：
- 类模板中的注入类名同时表示类时，表示的就是当前实例化（current instantiation）。
- 在类模板和嵌套类中，一个类名是否是当前实例化可能令人困惑：
    - 当类模板的嵌套类中，嵌套类名称本身和类模板名称确实表示当前实例化。
    - 但是在平行的嵌套类中，另一个嵌套类的名称则不是这样，而是一个未知实例化。

## 解析模板

- 大部分编程语言中最基础的步骤都是词法分析（tokenization/Scanning/lexing）和语法分析（Parsing）。
- 词法分析时，将源码作为字符序列读进来，生成一个记号序列（a sequence of tokens）。
- 比如`int*p = 0;`词法分析的输出结果是：一个关键字`int`，一个符号或者操作符`*`，一个标识符`p`，一个符号/操作符`=`，整数字面量`0`，一个符号或者操作符`;`。
- 然后语法分析器（parser）会根据已知的模式将这个记号序列解析成更高级的语法结构。这里的例子就得到一个初始化声明语句。

非模板代码的上下文敏感性：
- 词法分析相对语法分析会更简单，语法分析才是编译器前端最重要的东西，对比编译原理中有很多严谨的理论指导了该怎么进行语法分析。
- 因为C++是上下文敏感的（事实上几乎所有编程语言都是上下文敏感的），所以在词法分析和语法分析时，还需要一个符号表（symbol table）的配合。
- 当一个声明被解析完成后，它就进入了符号表，当词法分析器找到一个标识符时，会去符号表中查找这个标识符是什么类型。
- 比如：当编译器看到`x*`是，去符号表中如果找到了`x`是一个类型，那么将会将其理解为一个声明，而如果`x`是一个非类型的变量，那么唯一有效的语法结构就是乘法表达式了。
- 另一个例子：`X<1>(0)`如果`X`是一个模板，那么`1`会被解析为模板参数，`(0)`会被理解为参数列表，而如果`X`不是一个模板，那么这个式子将被理解为`(X<1)>0`。
- 当然以上这些上下文敏感的原因，一部分是因为选择了同一个符号用作两种含义。
- 再比如`List<List<int>> a;`在C++11之前，要求两个`>>`之间必须有空格，因为`>>`也可以被理解为右移运算符。由于词法分析的贪心原则（maximum munch principle，即优先将多个符号构成的字符序列尽可能解析为更长的token）。C++11起标准对这个规则做了hack，在模板中将不再优先解析为`>>`。
- 还有一个预处理阶段将`<:`替换为`[`的微妙问题，同样修改了标准，对预处理器或者词法分析器做了hack。一般来说遇不到，仅做了解。

非独立类型名称（dependent names of types）：
- 模板中的名称的一个问题是：不能被有效分类。
- 举一个具体的例子：一个模板使用了另一个模板，它也不能通过查看该模板的定义来获取必要的信息，因为模板还未实例化，对主模板有效对其中一个特化可能就是无效的。
```C++
template<typename T>
struct Trap
{
    enum {x};
};
template<typename T>
struct Victim
{
    int y;
    void proof() {
        Trap<T>::x * y; // a declaration or multiplication?
    }
};
template<>
struct Trap<void> {
    using x = int;
};
```
- 可以看到因为模板实参未决定导致在看到模板定义时很多东西都是信息不充分的。就像不知道`Trap<T>::x`是一个类型还是一个变量。
- 很明显，`Trap<T>`就是一个非独立类型（dependent type），因为这个类型依赖于模板形参`T`。而且`Trap<T>`是一个未知特化（unknown specialization），所以无法知道`Trap<T>::x`是一个类型还是一个变量。
- 标准为了解决这个问题，规定了任何这种名称都会被默认假定为非类型，如果它是一个类型，需要显式在前面添加`typename`关键字。
- 如果使用了`typename`修饰，但实例化时发现并不是一个类型，就会报出错误。
- 在这里不能将`typename`替换为`class`，`typename`一开始引入就是干这个事情，后面才扩展了用途。
- 在一个名称用作嵌套非独立类型名称时，所有需要使用`typename`的场景：
    - 是限定名称，但并不是仅跟在`::`后。
    - 不是详细类型说明符（elaborated-type-sepcifier，一个由`class struct union enum`开头的类型）的一部分。
    - 未出现在基类列表中和构造函数初始化列表中，因为这两种情况只能是类型，所以直接假定为类型，而不需要加`typename`。
    - 依赖于一个模板形参。
    - 是一个未知实例化的成员，也就是其名称被一个未知实例化（用`::`）所修饰。
- 注意的是在不满足前两个条件时一定不允许。第三个条件中也不允许使用`typename`，但最后两个条件如果满足则必须使用`typename`，不满足则`typename`是可选的。
- 最后一条件是比较难于判断的，对于一个嵌套从属类型名称，不管他是当前实例化还是未知实例化，保险起见，实践建议是都加上`typename`一定不会错，也不需要费尽心思去判断。
    - 例子：在类模板`X<T>`中，`C`是一个嵌套类型，那么在其定义中`typename X::C typename X<T>::C typename S<int>::C`的`typename`就是可选的，写上也不会错。但是用到了另一个类模板的嵌套类型名称`typename S<T>::SomeType`就必须要写。
- 最后C++20放宽了限制，某些场景下可以不加了。但还是那句话，比较难判断，加上总是好的。

非独立模板名称（dependent names of templates）：
- 和非独立类型名称很类似的一个问题是依赖于模板参数的模板，即非独立模板名称。
- 通常来说，一个模板后跟着的`<`应该被视为角括号，否则就被视为小于运算符。
- 与非独立类型名称类似，在模板中，标准规定一个非独立名称会被假定为不是模板，如果是模板的话需要显式使用`template`关键字。
- 一个依赖于模板参数的成员函数模板、嵌套类模板、嵌套别名模板、静态变量模板都需要在`:: -> .`后模板id前加上`template`关键字。更准确地说，是未知实例化的话才需要。当前实例化或者确切类型的实例化则不需要，但像`typename`也可以加。总之在类似场景下，加上总是更清晰且不易出错的。
- 在类模板外，已经实例化的话是不需要的。

`using`声明中的非独立名称（dependent names in using declarations）：
- `using`声明可以从两个位置将名称带到当前作用域来：命名空间作用域、类作用域。
- 前者比较简单，和这里讨论的问题无关。
- 后者在模板中主要用在将基类名称引入派生类中。这个名称可以是重载的类名称，此时所有重载都被引入派生类。引入后会加入到派生类对应访问层级下，比如可将私有基类中的成员引入到派生类`public`下，这样对派生类的用户将也变得可见。
- 在类模板中引入基类模板的类型时，同样需要使用`typename`关键字。`using typename Base<T>::name;`。
- 但在引入基类模板的模板时，却不提供`using Base<T>::template name;`这种语法。但是对于类型的话可以使用别名模板来代替：`template<T> using name = typename Base<T>::template name;`算是一种曲折的解决方案。但对于函数模板却没有这样一种合适的机制，比较遗憾。

ADL和显式模板参数：
- 当函数模板通过显式模板参数调用，又期望通过ADL查找时，会出现一个问题：函数模板在调用点不可见，就只能通过ADL查找，但是当看到函数模板调用的模板形参列表时才会将前面的名称作为函数模板来解析，但是只有将前面的名称作为函数模板时才会将后续的模板形参列表当做模板形参列表，而不是小于号开始的奇怪表达式。
```C++
namespace N {
    class X {
        ...
    };
    template<typename T> void select(X*);
}
void g(N::X* xp)
{
    select<X>(xp); // ERROR: no ADL!
}
```
- 这就造成了先有鸡还是先有蛋的死递归，所以程序不能处理。最终编译器会在不知道名称是模板的情况下当做小于号处理。
- 要解决这个问题，可以在调用点做一次模板声明，指明这是一个函数模板。然后使用ADL就能找到定义了。

非独立表达式（dependent expressions）：
- 就像名称一样，表达式也可以依赖于模板参数，然后在不同实例化中表现出不同的行为。
- 比如：选择不同的重载、产生不同的类型或者常量。
- 不依赖模板参数的表达式则会在所有实例化中保持相同的行为。
- 非独立表达式可以以多种方式依赖于模板参数：
    - 类型依赖表达式（type-dependent expression），即表达式类型在不同实例化中不同。有类型依赖子表达式的表达式一般都是类型依赖的。
    - 不是所有依赖于模板参数的表达式都是类型依赖的，比如也有可能依赖于一个非类型模板参数，然后返回非类型模板参数自身的表达式就是一个值依赖表达式（value-dependent expression）。值依赖表达式不一定是类型依赖表达式。
    - 所有涉及到模板参数类型的都叫做实例化依赖表达式（instantiation-dependent expression）。实例化依赖表达式有可能在实例化时报出一个错误。
- 他们之间的包含关系：
    - 所有表达式包含实例化依赖表达式。
    - 实例化依赖表达式包含值依赖表达式。
    - 值依赖表达式包含类型依赖表达式。
- 知道这些好像对实践并无多大指导作用。这些概念都是C++标准中的用来描述语义的，在实例化时会有影响。

## 继承与类模板

类模板也可以继承或者被继承，这没有什么特殊的。但有一个场景比较微妙，就是基类依赖于模板参数的情况，即有一个非独立基类（dependent base class）。

独立基类：
- 换言之，使用基类名称就是使用独立名称。
- 只是要注意有一个反直觉的点：来自于基类的未修饰名称优先于模板参数。通常来说要避免这种名称冲突。

非独立基类：
- 独立基类有一个问题就是相关名称不能在实例化时才查找（不能延迟查找）。
- 独立名称遇到时就会查找。
- 而非独立名称要在实例化时才会查找，包括来自非独立基类的名称。
- 所以C++规定独立名称不会在非独立基类中查找（但是独立名称依然一遇到就会查找）。
- 那么如果派生类用到了非独立基类的名称（但是看起来却一个独立名称，一遇到就查找过了，而本意其实是想使用非独立基类中的非独立名称）怎么办呢？答案就是让这个独立名称变成一个非独立名称（依赖名称）。
- 标准做法有三个，前面已经介绍过了：
    - 使用`this->`修饰这个独立名称让其变为非独立名称。
    - 使用基类来限定名称`Base<T>::name`。但是这样会丧失多态的可能，如果是虚函数的话就必须使用前者。
    - 使用`using`引入基类名称，已经介绍过。
- 在非独立基类配合多继承，且有来自多个基类的同一个函数实现时，情况可能变得非常复杂，方法2和3可以作为解决，不过需要自行选择需要哪一个实现。

## 后记

相关发展历史就略过了，没有太大必要了解。
