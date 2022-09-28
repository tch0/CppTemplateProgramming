<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十六章：特化与重载](#%E7%AC%AC%E5%8D%81%E5%85%AD%E7%AB%A0%E7%89%B9%E5%8C%96%E4%B8%8E%E9%87%8D%E8%BD%BD)
  - [当泛型代码不能很好解决问题时](#%E5%BD%93%E6%B3%9B%E5%9E%8B%E4%BB%A3%E7%A0%81%E4%B8%8D%E8%83%BD%E5%BE%88%E5%A5%BD%E8%A7%A3%E5%86%B3%E9%97%AE%E9%A2%98%E6%97%B6)
  - [重载函数模板](#%E9%87%8D%E8%BD%BD%E5%87%BD%E6%95%B0%E6%A8%A1%E6%9D%BF)
  - [显式特化](#%E6%98%BE%E5%BC%8F%E7%89%B9%E5%8C%96)
  - [类模板偏特化](#%E7%B1%BB%E6%A8%A1%E6%9D%BF%E5%81%8F%E7%89%B9%E5%8C%96)
  - [变量模板偏特化](#%E5%8F%98%E9%87%8F%E6%A8%A1%E6%9D%BF%E5%81%8F%E7%89%B9%E5%8C%96)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十六章：特化与重载

前面已经介绍了如何将一个泛型的模板定义通过实例化扩展为一簇函数、类、变量。但泛化和专用是天然对立的，泛化必然要考虑所有类型的情况，必然不可能做到极致性能，泛化的代码对于特定类型替换可能不是最优的，甚至不是良构的。面对这种情况，C++提供了对特定模板参数进行特化（specialization）的手段。除此之外，函数模板重载也提供了另一种可以针对特定模板参数选择不同实现的机制。本章主要讨论这两个话题。

## 当泛型代码不能很好解决问题时

典型例子：`std::swap`的通用实现使用拷贝操作（现在其实已经是移动了，假定是早期版本使用拷贝），然后对于特别大的类型时间和空间开销都会很大，这时候就需要针对特定类型实现不同的逻辑以优化。实现手段可以是特化`std::swap`也可以是在命名空间作用域重载`swap`（通过ADL查找，通常都是这种实现手段，因为函数模板特化必须全特化还要放在全局作用域，而重载可以参数化放同一命名空间）。使用时标准用法是局部作用域`using std::swap;`后使`std::swap`与通过ADL得到的`swap`构成重载（或者直接引入`std::swap`特化），然后直接调用`swap`，无论哪种情况都能调用到最佳实现（前提是对应的声明在此处可见）：
```C++
{
    using std::swap;
    swap(a, b);
}
```

透明化定制（transparent customization）：对于函数模板，重载机制提供了透明化定制的可能，同一个使用方式，重载解析时选择不同重载以达到针对特定类型定制的目的。

语义透明度（semantic transparency）：函数重载方式实现的透明化定制的透明度取决于一系列的实现细节。

## 重载函数模板

函数模板可以重载，不同重载甚至使用相同模板参数实例化，或者拥有相同参数列表，只要函数签名不同（模板实参列表和函数参数列表不同时相同即可），最终生成结果就不是同一个。
```C++
template<typename T>
int f(T); // #1
template<typename T>
int f(T*); // #2

f<int>(1); // #1
f<int*>(&a); // #1
f<int>(&a); // #2
```

签名（signatures）：
- 在一个程序中，只要两个函数有不同的函数签名就可以共存。函数签名使用如下信息定义：
    - 函数的未修饰名称、或者生成的函数名称。
    - 该名称所在的类或者命名空间，如果这个名称是内部链接的，那么还有它所在的编译单元。
    - 成员函数的话，还要加上`const volatile & &&`修饰符。
    - 函数参数类型，如果是函数模板，那么是替换之前的类型。
    - 如果函数是从函数模板生成的，那么还有返回值类型。
    - 如果函数从函数模板生成，那么还有模板形参和模板实参。
- 这意味着原则上下列函数模板的和他们的实例可以同时存在：
```C++
template<typename T1, typename T2>
void f1(T1, T2);
template<typename T1, typename T2>
void f1(T2, T1);
template<typename T>
long f2(T);
template<typename T>
char f2(T);
```
- 因为他们还没有实例化，可能不能在同一个作用域下实例化，会产生歧义的实例在同一编译单元是不能共存的。
- 在同一个编译单元下会产生歧义，但是签名不同的函数模板，在不同编译单元中也是可以共存的。比如上面的两个`f1<char, char>`如果是分属于两个模板的两个实例，并且分别实例化于两个编译单元就能够共存（函数签名是不同的）。

重载函数模板的**偏序**（partial ordering）：
- 例子：
```C++
template<typename T>
int f(T) // #1
{
    return 1;
}
template<typename T>
int f(T*) // #2
{
    return 2;
}

int main(int argc, char const *argv[])
{
    std::cout << f(0) << std::endl; // #1
    std::cout << f(nullptr) << std::endl; // #1
    std::cout << f((int*)nullptr) << std::endl; // #2
    return 0;
}
```
- 第一个和第二个都是只有第一个重载匹配，没有问题。
- 但第三个调用`f((int*)nullptr)`按道理上来说应该是两者同样匹配，第一个重载将`T`推导为`int*`，第二个重载将`T`推导为`int`。
- 但是这里将会有一个特殊的重载决议：同样程度匹配的函数模板重载，谁更加特化谁就更匹配。在这里版本2被认为更加特化，所以匹配了第二个`f<int>(int*)`。
- 这里的不同函数模板重载是有偏序的，具体规则如下。

正式排序规则：
- 上面的例子是符合直觉的，但是我们需要一个正式的规则来说明到底怎么样才叫做**更加特化**。
- 首先需要注意的是，这个排序规则是偏序（partial ordering）的，而非全序。也就是存在两个重载，每个都不比另一个更特化，他们两之间无法判断谁更加特化。这时会产生歧义，编译器报出错误。
- 假定两个同名函数模板，对于给定的一个函数调用都是可行的且同等程序匹配（由普通重载决议规则决定），那么重载决议结果由以下额外规则决定：
    - 首先调用时被默认函数实参覆盖的函数形参、和可变参数形参 不参与下列的比较。
    - 我们合成两个实参列表（或者对于转换函数模板，一个返回类型），通过以下类型来替换每个模板形参：
        - 使用一个合成的类型（invented type）替换每个模板形参。
        - 使用一个合成的类模板替换每个模板模板形参。
        - 使用一个恰当类型的值替换每个非类型模板参数。
        - （在这个上下文中合成出来的类型、模板和值不同于任何其他程序员使用的或者编译器在其他上下文合成出来的类型、模板和值）。
    - 如果第二个合成模板实参列表推导为第一个合成的模板实参列表能够推导成功且完全匹配（exact match），并且反过来不成立，那么第一个模板就比第二个更加特化。反之，如果第一个合成模板实参列表推导为第二个合成模板参数列表能够推导成功且完全匹配，并且反过来不成立，那么第二个模板就比第一个更加特化。否则，两个模板重载之间没有序关系。
- 回顾上一个例子：
```C++
template<typename T>
int f(T) // #1
{
    return 1;
}
template<typename T>
int f(T*) // #2
{
    return 2;
}
```
- 描述这两个合成的实参列表为`(A1) (A2*)`显然后者可以由前者推导得到，将`(A1)`推导为`(A2*)`只需要将`A1`替换为`A2*`，反过来明显不成立，所以第二个版本更特化。
- 通常来讲结果都是符合直觉的，按照直觉思考和编写重载通常不会有太大问题，有问题时再仔细思考一下这个规则即可。

模板与非模板：
- 函数模板可以与非模板函数重载。
- 如果其他所有条件都一致，同样程度匹配时，会优先选择非模板版本。
- 很多时候重载决议时可能会选择意想不到的版本：比如对于构造函数，拷贝接受使用`const`左值参数、移动构造使用右值引用参数以及一个模板版本万能引用参数的构造，如果传入一个左值，那么就会优先选择模板万能引用版本。要禁用这种情况，前面已经介绍过，利用SFINAE原则使用`std::enable_if`解决即可。

可变参数函数模板：
- 当函数模板具有函数参数包时，有几个比较有趣的场景。
- 在相同程度匹配时，有不含函数参数包的版本和含参数包版本，优先选择不含参数包版本。
- 如果多个含有函数参数包的版本同等程度匹配，同样可以使用前面介绍的排序规则进行排序。

## 显式特化

类模板和变量模板不能重载，所以需要使用另外一种机制——显式特化。当我们谈到显式特化，通常是指全特化，不再含有任何模板参数。类模板、函数模板和变量模板都可以全特化（别名模板不能进行任何形式地特化）。

除了全特化，为模板定制实现但是同时保留一定参数化能力也可以进行偏特化。但其实偏特化和全特化都是显式的，所以我们通常不使用显式特化这个词，而是使用术语全特化和偏特化。显式特化为已有的未特化的（泛型）实例提供了一种额外的定义。

类模板全特化；
- 以`template<>`作为开始，在模板名称后跟全特化的模板实参。
- 全特化的实参需要和类模板形参对应，其中有默认模板实参的模板形参是否添加到全特化中则是可选的。
- 全特化不需要有定义，只有声明也是可以的。前向声明对于有相互依赖的场景是有用的。
- 一个类模板全特化定义更类似于一个普通类定义，而非类模板定义。
- 因为类模板全特化是一个确切的类定义，所以只需要在全特化的类定义前声明`template<>`，而因为其成员函数、静态成员都不是模板化的，所以类外定义时不需要也不能加`template<>`，就像普通类的类外成员定义一样。如果是模板成员，也只需要属于成员的一层模板形参声明。
- 类模板全特化是在对一组特定模板实参的情况下对类模板泛型实现的替换。在一个程序中同时有该组实参的泛型特化与全特化是非法的（所以不能同时全特化与显式实例化）。
- 但是如果全特化是定义在源文件中，那么很可能会出现上面所述的情况。所以通常需要将类模板全特化定义在头文件中，保证在实例化每一个类模板实例时所有全特化都是可见的。其中如果在头文件中类外实现类模板全特化的成员，就需要`inline`。

函数模板全特化：
- 函数模板全特化和类模板全特化很像，不过其中还有重载和模板实参推导参与进来。
- 函数模板全特化声明可以忽略默认模板实参。
```C++
template<typename T = int>
void f(T);
// specialization of T = int
template<>
void f<>(int); // equal to f<int>(int)
```
- 函数模板全特化中不能为函数形参指定默认实参，但依然可以使用主模板的默认实参。这是因为函数模板全特化提供了一个替换实现，但不是一个新的声明，可以理解为只有主模板声明是可见的。
```C++
template<typename T = int>
void g(T a = 10)
{
    std::cout << "g: " << a << std::endl;
}
template<>
void g<int>(int a)
{
    std::cout << "g<int>: " << a << std::endl;
}
int main(int argc, char const *argv[])
{
    g(); // ouput: g<int>: 10
    return 0;
}
```
- 函数模板全特化不是模板，同样只能在整个程序中出现一次，要么在头文件中`inline`实现（通常是这样做的），要么头文件声明，源文件中实现一次。注意函数模板全特化的声明其实也不能算是一个严格意义的声明，而是一个对全特化定义的声明。
- 函数模板全特化的模板实参如果可以经过推导而来，可以不写出模板实参列表。
```C++
template<typename T = int>
void f(T);
// specialization of T = int
template<>
void f(int); // equal to f<int>(int)
```

变量模板全特化：
- 变量模板同样可以全特化，同理如果定义在头文件需要`inline`。
- 并且变量模板全特化的变量类型不需要和主模板进行同参数列表实例化时的类型相匹配。

成员模板全特化：
- 类模板的成员模板、以及普通静态数据成员、普通或静态成员函数都可以全特化：
```C++
template<typename T>
class X
{
public:
    // member function template
    template<typename U>
    void tf()
    {
        std::cout << "X<T>::tf<U>()" << std::endl;
    }
    // static member function template
    template<typename U>
    static void stf()
    {
        std::cout << "X<T>::stf<U>()" << std::endl;
    }
    // static data member template
    template<typename U>
    inline static std::string ststr = "X<T>::ststr<U>()";
    
    // ordinary member function
    void f()
    {
        std::cout << "X<T>::f()" << std::endl;
    }
    // ordinary static data member
    static std::string sstr; // = "X<T>::sstr";
    // ordinary static member function
    static void sf()
    {
        std::cout << "X<T>::sf()" << std::endl;
    }
};
template<typename T>
inline std::string X<T>::sstr = "X<T>::sstr"; // defining as inline inside class will cause reinitialization error

// full specialization of member template
template<>
template<>
inline void X<int>::tf<int>()
{
    std::cout << "X<int>::tf<int>()" << std::endl;
}
template<>
template<>
inline void X<int>::stf<int>()
{
    std::cout << "X<int>::stf<int>()" << std::endl;
}
template<>
template<>
inline std::string X<int>::ststr<int> = "X<int>::ststr<int>";

// full specialization of ordinary member
template<>
inline void X<int>::f()
{
    std::cout << "X<int>::f()" << std::endl;
}
template<>
inline std::string X<int>::sstr = "X<int>::sstr";
template<>
inline void X<int>::sf()
{
    std::cout << "X<int>::sf()" << std::endl;
}
```
- 注意，仅对某个类模板参数特化了某个成员的话，就不能再对这套模板参数全特化整个类。对这一套模板参数，特化的成员将使用特化版本，未特化的将依然使用泛型版本。
- 然后，普通类的成员模板也可以特化：
```C++
class Y
{
public:
    // member function template
    template<typename U>
    void tf()
    {
        std::cout << "Y::tf<U>()" << std::endl;
    }
    // static member function template
    template<typename U>
    static void stf()
    {
        std::cout << "Y::stf<U>()" << std::endl;
    }
    // static data member template
    template<typename U>
    inline static std::string ststr = "Y::ststr<U>()";
};

// full specialization of member template of ordinary class
template<>
inline void Y::tf<int>()
{
    std::cout << "Y::tf<int>()" << std::endl;
}
template<>
inline void Y::stf<int>()
{
    std::cout << "Y::stf<int>()" << std::endl;
}
template<>
inline std::string Y::ststr<int> = "Y::ststr<int>";
```
- 对类模板的静态成员进行全特化时，定义在头文件同样需要加`inline`，也可以头文件声明，源文件实现。
- 类模板的静态数据成员，如果不初始化，那么是不会被认为是一个定义的，只会被认为是一个声明，这点比较特殊（为了区分定义和声明）。
```C++
template<>
std::string X<int>::sstr; // this is just declaration
template<>
std::string X<int>::sstr{}; // this is definition
```
- 上面都没有涉及到成员类模板，原理是类似的，也可以单独被全特化，甚至全特化成员类模板的成员。
- 全特化总体上来说非常灵活，这里也无法穷尽，但记住有一点是不允许的：那就是外层模板没有全特化，而仅全特化内层模板。（更一般地说：**C++禁止在非全特化的东西里面搞出全特化的东西**）。
- 更多的细节还需要在实践中探索。

## 类模板偏特化

全特化是有用的，但是某些时候要求对某一簇类型都是同样实现，也就是特化之后依然是模板是参数化的，那么就需要偏特化（partial sepcialization）一展身手了：
- 模板对于泛化是有用的，但是某些时候可能会造成代码臃肿，比如考虑一个模板化的链表类：`template<typename T> class List<T>`，当模板实参是指针的时候，可以想象所有实现（甚至二进制）都是基本一样的。但是却会每个类型生成一个实例。为了降低代码臃肿问题，可以对指针进行偏特化`template<typename T> class List<T*>`，其中将所有操作委托给一个`List<void*>`成员（或者作为基类），最后操作完全转掉，类型相关时做一个指针转换即可。很明显这里`List<void*>`和`List<T*>`循环依赖了，所以需要先提供一个指针版本的公共实现`List<void*>`全特化。
- 偏特化并不是唯一的解决代码臃肿的方案，直接将所有不涉及参数化的东西想办法抽取到一个公共基类中也是一个方法。
- 偏特化的模板实参和形参会有一些限制：
    - 偏特化的模板实参在种类上（类型、非类型、模板）必须与主模板的形参匹配。
    - 偏特化的模板形参列表中不能有默认模板参数（与全特化类似），而是使用主模板的默认模板实参。
    - 偏特化的非类型模板实参必须是独立的值（编译期常量）或者单纯的非类型模板参数，不能是复杂的非编译期常量表达式。比如类模板`template<int N> class A`不能有`template<int N> class A<2*N>`这样的偏特化。
    - 偏特化的模板实参列表不能和主模板完全相同，会无法区分。比如`template<int N, typename T> class A`不能有偏特化`template<int N, typename T> class A<N, T>`。
    - 如果偏特化的一个实参是一个包扩展，那么必须放在模板实参列表末尾。
- 像全特化一样，偏特化也是和一个主模板（primary template）关联起来的。当该模板被使用时，一定是最先查找主模板，然后才尝试匹配这个主模板关联的特化以决定要选择哪个实现。
- 就像函数模板实参推导一样，SFINAE原则也会用在这里，当尝试匹配的一个全特化不合法时，这个特化会被单纯抛弃然后尝试选择其他特化。如果没有特化匹配，那么会选择主模板。
- 如果多个特化同时匹配，那么会选择最特化的那一个（判断规则和函数模板重载偏序规则相同），如果其中没有一个最特化的版本，那么会有歧义报错。
- 最后要指出，类模板偏特化的模板形参是可能比主模板更多或者更少的，但是实参数量一定是匹配的。类模板偏特化的形参是通过推导而来是无法指定的（有模板形参不能被推导出的话就是一个非法的偏特化），指定的永远是主模板的实参。
- 例子：
```C++
class Foo
{
public:
    int* pi;
    void* pv;
    double* pd;
};

class Bar
{
public:
    int* pi;
    void* pv;
    double* pd;
};

template<typename T>
class X
{
public:
    static void f()
    {
        std::cout << "X<T>::f()" << std::endl;
    }
};

// partial specialization
template<typename C>
class X<void* C::*> // #1
{
public:
    static void f()
    {
        std::cout << "X<void* C::*>::f()" << std::endl;
    }
};

template<typename T, typename C>
class X<T* C::*> // #2
{
public:
    static void f()
    {
        std::cout << "X<T* C::*>::f()" << std::endl;
    }
};

template<typename T>
class X<T* Foo::*> // #3
{
public:
    static void f()
    {
        std::cout << "X<T* Foo::*>" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    X<int>::f(); // generic version
    X<decltype(&Bar::pv)>::f(); // #1
    X<decltype(&Bar::pd)>::f(); // #2
    X<decltype(&Foo::pi)>::f(); // #2
    // X<decltype(&Foo::pv)>::f(); // ambiguous
    return 0;
}
```
- 很明显偏特化3和偏特化1都比偏特化2更特化，但是1和3之间无法比较，如果1和3都匹配就会有歧义。
- 偏特化的模板实参甚至可以与主模板不同，包含以下两种情况：
    - 主模板有默认实参，偏特化比主模板参数少，剩余实参使用主模板默认实参。
    - 主模板是可变参数模板，偏特化时把数量固定下来或者依旧使用参数包作为模板实参但其余参数数量不一致。只要偏特化模板实参能够与主模板形参相匹配均合法。
```C++
template<typename... Args>
class Tuple;
template<typename T1>
class Tuple;
template<typename T1, typename T2, typename... Rest>
class Tuple<T1, T2, Rest...>; // tuple with two or more elements
```

## 变量模板偏特化

简单来说和类模板完全一样，不赘述。

## 后记

- 模板特化可以终止模板递归的能力为人知很长一段时间后（`List<T*>`例子），Erwin Unruh提出了模板元编程（template metaprogramming）这个名词：使用模板实例化过程在编译期进行一些非平凡的计算。
- 每个人都会好奇为什么只有类模板和变量模板可以偏特化，而函数模板不可以。这个问题的更多是历史原因而非实现不了。函数模板有函数重载这个机制，从功能上来说完全可以替代（甚至更强）偏特化。
- 不过函数模板重载和类模板与变量模板的偏特化是有微妙的区别的：
    - 选择偏特化时一定是先查找主模板，再查找其偏特化，它们是从属的关系。
    - 而对于函数模板重载则是来自不同命名空间或者类的所有重载位于同一个候选集合中，它们的关系是平等的。
- 更多原因可能还是重载和偏特化已经能够满足所有需求，他们甚至使用相同的偏序关系决定规则。你甚至可以想象类模板可以重载、函数模板可以偏特化的情景，但除了引入更多复杂度可能并不会有功能性的提升。
