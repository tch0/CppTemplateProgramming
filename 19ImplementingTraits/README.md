<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第十九章：特征的实现](#%E7%AC%AC%E5%8D%81%E4%B9%9D%E7%AB%A0%E7%89%B9%E5%BE%81%E7%9A%84%E5%AE%9E%E7%8E%B0)
  - [例子：累加一个序列](#%E4%BE%8B%E5%AD%90%E7%B4%AF%E5%8A%A0%E4%B8%80%E4%B8%AA%E5%BA%8F%E5%88%97)
  - [特征与策略、策略类](#%E7%89%B9%E5%BE%81%E4%B8%8E%E7%AD%96%E7%95%A5%E7%AD%96%E7%95%A5%E7%B1%BB)
  - [类型函数](#%E7%B1%BB%E5%9E%8B%E5%87%BD%E6%95%B0)
  - [基于SFINAE的特征](#%E5%9F%BA%E4%BA%8Esfinae%E7%9A%84%E7%89%B9%E5%BE%81)
  - [IsConvertible](#isconvertible)
  - [检测成员](#%E6%A3%80%E6%B5%8B%E6%88%90%E5%91%98)
  - [其他特征技术](#%E5%85%B6%E4%BB%96%E7%89%B9%E5%BE%81%E6%8A%80%E6%9C%AF)
  - [类型分类](#%E7%B1%BB%E5%9E%8B%E5%88%86%E7%B1%BB)
  - [策略特征](#%E7%AD%96%E7%95%A5%E7%89%B9%E5%BE%81)
  - [标准库中的特征](#%E6%A0%87%E5%87%86%E5%BA%93%E4%B8%AD%E7%9A%84%E7%89%B9%E5%BE%81)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第十九章：特征的实现

## 例子：累加一个序列

固定特征：
- 考虑一个累加的例子：
```C++
template<typename T>
T accum(const T* beg, const T* end)
{
    T res{};
    while (beg != end)
    {
        res += *beg;
        beg++;
    }
    return res;
}
```
- 这个例子存在一个问题，当对`char`进行实例化时，结果也是`char`类型，很有可能会超过表示范围溢出。
- 一个可行的解决方案可以是添加一个模板参数，让用户显式指定结果保存为什么类型。但这样会增加用户的负担。
- 另一个选择可以是实现一个特征，让一个类型与其和的类型关联起来。
```C++
template<typename T>
struct AccumulationTraits;
template<>
struct AccumulationTraits<char>
{
    using AccT = int;
};
template<>
struct AccumulationTraits<short>
{
    using AccT = int;
};
template<>
struct AccumulationTraits<int>
{
    using AccT = long;
};
template<>
struct AccumulationTraits<unsigned int>
{
    using AccT = unsigned long;
};
template<>
struct AccumulationTraits<float>
{
    using AccT = double;
};
```
- 这个`AccumulationTraits`类模板称之为**特征**模板（traits template），因为他保存了其参数类型的一个特征，也就是其累加的结果类型。
- 有了这个特征（trait）之后，我们将累加的函数重写：
```C++
template<typename T>
auto accum(const T* beg, const T* end)
{
    using AccT = typename AccumulationTraits<T>::AccT;
    AccT res{};
    while (beg != end)
    {
        res += *beg;
        beg++;
    }
    return res;
}
```
- 我们添加了一种非常有用的用来自定义我们的算法的机制。当需要支持新的自定义类型时，可以通过对自己的类型特化`AccumulationTraits`做到。

值特征：
- 除了一个类型，也可以将一个值关联到一个类型上。
- 考虑上面的`accum`函数模板，如果结果类型没有一个默认构造函数就不能正常工作了。为了解决这个问题，我们可以在其特征中定义一个初始值用于结果的初始化。
```C++
template<typename T>
struct AccumulationTraits;
template<>
struct AccumulationTraits<char>
{
    using AccT = int;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<short>
{
    using AccT = int;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<int>
{
    using AccT = long;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<unsigned int>
{
    using AccT = unsigned long;
    static constexpr AccT zero = 0;
};
template<>
struct AccumulationTraits<float>
{
    using AccT = double;
    static constexpr AccT zero = 0;
};
```
- 重写`accum`：
```C++
template<typename T>
auto accum(const T* beg, const T* end)
{
    using AccT = typename AccumulationTraits<T>::AccT;
    AccT res = AccumulationTraits<T>::zero;
    while (beg != end)
    {
        res += *beg;
        beg++;
    }
    return res;
}
```
- 当我们要支持一个自定义类型`BigInt`使用这个算法时，可以通过特化`AccumulationTraits`做到：
```C++
template<>
struct AccumulationTraits<BigInt>
{
    using AccT = BigInt;
    inline static const AccT zero {};
};
```
- 如果`BigInt`不是字面量类型，则不能使用`constexpr`只能用`const`，自动C++17开始，可以使用`inline`进行静态变量的类内定义与初始化。
- 在C++17之前，为了定义零值这个值特征，也可以提供一个`zero`静态函数，作为替代，其中返回结果类型的零值。使用时唯一的区别是要使用函数调用形式。
```C++
template<>
struct AccumulationTraits<BigInt>
{
    using AccT = BigInt;
    static AccT zero()
    {
        return AccT(0);
    }
};
```
- 很显然，特征并不仅仅只是提供一个额外类型的机制，可以使用特征提供任何关于一个类型的必要的信息。

参数化特征：
- 前面的使用特征的方式是固定的。不能在算法中替换，某些时候可能想要替换这样的特征。
- 我们可以通过将特征本身作为模板参数添加到函数模板来做到将特征也参数化，以支持替换。
```C++
template<typename T, typename AT = AccumulationTraits<T>>
auto accum(const T* beg, const T* end)
{
    using AccT = typename AT::AccT;
    AccT res = AT::zero;
    while (beg != end)
    {
        res += *beg;
        beg++;
    }
    return res;
}
```
- 通过这种形式定义算法，大部分使用者可以忽略这个额外的模板参数，但是某些特殊的用户可以自己定义这个特征。

## 特征与策略、策略类

考虑与求和类似的累积过程，比如求积。我们只需要将`res += *beg`的过程替换成一个通用的操作，称之为**策略（policy）**，通过一个策略类传入：
```C++
class SumPolicy
{
public:
    template<typename T1, typename T2>
    static void accumulate(T1& total, const T2& value)
    {
        total += value;
    }
};

template<typename T, typename Policy = SumPolicy, typename Traits = AccumulationTraits<T>>
auto accum(const T* beg, const T* end)
{
    using AccT = typename Traits::AccT;
    AccT res = Traits::zero;
    while (beg != end)
    {
        Policy::accumulate(res, *beg);
        beg++;
    }
    return res;
}
```
- 考虑同样的求积过程，存在一个问题：初值。这里初值应该放在累积策略里面，或者显式传入。

特征和策略对比：
- 我们倾向于将策略类这个术语解释为封装和其他模板参数正交（也就是和其他模板参数无关，没有以任何形式用到其他模板参数）的功能的模板参数。
- 更具体的来说：
    - 策略和特征很像，但是不同于特征，因为策略更强调行为而不是类型。
    - 而特征类则是为了取代模板参数。作为类，其封装有用的类型与常数。作为模板，它是提供了“解决软件问题的那一层额外间接”的枢纽。
- 所以，通常来说，我们使用如下定义：
    - 特征表示模板参数上自然附加的属性。
    - 策略表示泛型函数或者类型的可定制行为。
- 为了更进一步说明他们的区别，这里提供几个关于特征的观察：
    - 特征作为固定特征（fixed traits）也很有用（不通过模板参数传递）。
    - 特征参数通常拥有非常自然的默认实参，这个默认实参很少被覆盖，或者压根不能被覆盖。
    - 特征参数会紧密依赖于其他的一个或者多个模板参数。
    - 特征大多都会组合类型和常数而不是成员函数。
    - 特征倾向于聚集在特征模板中。
- 相对地，关于策略类的观察：
    - 当策略类不做为模板参数传递时，基本不做任何贡献。
    - 策略类模板参数不需要默认实参，并且调用时通常通过显式指定（尽管大多数泛型组件可能会将最常用的策略配置为默认策略）。
    - 策略类的功能通常来说和其他模板参数是正交的（orthogonal）。
    - 策略类大多都会组合成员函数。
    - 策略可以聚集在普通类或者类模板中。

成员模板与模板模板参数：
- 在定义策略类时，一般来说有两个选择，可以定义为类模板，然后使用其普通成员函数。
- 也可以定义为普通类，使用其成员函数模板。
- 定义为类模板则需要在使用时显式指定模板参数，并且用在泛型组件中时需要定义为模板模板参数。
- 定义为类的成员模板则可以通过推导得到模板参数，用在泛型组件中时使用普通模板参数即可。不过就没办法在策略类内部定义依赖于模板参数的静态常量了。

组合多个策略与特征：
- 为了增加代码的泛用性，可以在一个类或者函数中定义多个策略与特征。
- 那么组织这些特征和策略的顺序就有讲究了，通常来说，最简单的组织方式就是将更通常会使用默认实参的放到后面。也就是说一般是策略在前，特征在后。
- 当然也可以随意组织，调用时传递所有模板参数，只是用起来太麻烦了。

## 类型函数

在传统C和C++代码中，我们编写的函数可以更具体地称为值函数（value functions），他们接受一些值作为参数，然后返回另一个值作为结果。

在现代C++中，我们可以通过模板定义**类型函数**：使用类型作为参数，然后产出一个类型或者常量作为结果。
- 一个非常有用的内建类型函数是`sizeof`。
- 类型函数的典型实现方法是模板参数作为输入，提取出一个成员类型或者静态常量数据成员作为输出。
- 比如定义一个等价于`sizeof`的类型函数：
```C++
template<typename T>
struct TypeSize
{
    static const std::size_t value = sizeof(T);
};
```
- 下面介绍一些非常有用的可以用在特征中的类型函数。

元素类型：
- 给定一个容器类型，得到其元素类型：
```C++
template<typename T>
struct ElementT;
template<typename T>
struct ElementT<std::vector<T>> {
    using type = T;
};
template<typename T>
struct ElementT<std::list<T>> {
    using type = T;
};
template<typename T, std::size_t N>
struct ElementT<T[N]> {
    using type = T;
};
template<typename T>
struct ElementT<T[]> {
    using type = T;
};
```
- 为所有可能的类型偏特化即可。
- 因为容器类型已经定义了元素类型，所以只需要为主模板提供如下定义。
```C++
template<typename C>
struct ElementT
{
    using type = typename C::value_type;
};
```
- 然后为数组等特殊类型提供偏特化即可。
- 通常来说，无论如何，在类模板中为模板参数提供类型别名都是一个好的实践，就像标准库容器那样，方便外部泛型代码中能够方便地获取到模板参数的信息。
```C++
template<typename T1, typename T2>
class X
{
    using ... = T1;
    using ... = T2;
    ...
};
```
- 通过类型函数的方式，我们可以为基础类型以及封闭的库（用户无法修改）添加非侵入式的扩展。
- 上述的`ElementT`也是一个特征类，因为他获取了给定类型的一个特征。
- 我们可以为特征类定义一个别名模板，那样就不需要用`typename Traits::type`这种冗长的写法了。
```C++
template<typename T>
using ElementType = typename Element<T>::type;
```
- 在标准库中这样的类型函数的别名通常是通过添加`_v`（对于常量）`_t`（对于类型）后缀来命名的。

转换特征（transformation traits）：
- 除了获取类型的某方面的属性，特征还可以实现为对类型的转换。比如添加或者移除特定的CV限定或者引用等。

移除引用：
```C++
// remove reference of type
template<typename T>
struct RemoveReference
{
    using type = T;
};
template<typename T>
struct RemoveReference<T&>
{
    using type = T;
};
template<typename T>
struct RemoveReference<T&&>
{
    using type = T;
};

// alias template for RemoveReference
template<typename T>
using RemoveReference_t = typename RemoveReference<T>::type;
```
- 对应的标准库特征是`std::remove_reference`。
- 这在某些特殊推导规则时很有用，比如使用`decltype`的推导，比如万能引用`T&&`。

添加引用：
```C++
template<typename T>
struct AddLvalueReference
{
    using type = T&;
};
template<typename T>
using AddLvalueReference_t = typename AddLvalueReference<T>::type;

template<typename T>
struct AddRvalueReference
{
    using type = T&&;
};
template<typename T>
using AddRvalueReference_t = typename AddRvalueReference<T>::type;
```
- 引用折叠的规则将会在这里应用。
- 如果在这里不需要进行特化，那么定义其他可以简化为：
```C++
template<typename T>
using AddLvalueReference2_t = T&;
template<typename T>
using AddRvalueReference2_t = T&&;
```
- 使用别名模板的话就不需要实例化了，使用起来会更轻量（light-weighted）。但是别名模板不能特化、偏特化，相对来说使用起来风险会更高一些，比如不能使用`void`作为模板参数。而使用前面的实现则可以对`void`做全特化。
- 标准库中的对应实现为`std::add_lvalue_reference std::add_rvalue_reference`，其中对`void`进行了特化。

移除限定符（removing qualifiers）：
- 比如移除`const`：
```C++
template<typename T>
struct RemoveConst
{
    using type = T;
};
template<typename T>
struct RemoveConst<const T>
{
    using type = T;
};
template<typename T>
using RemoveConst_t = typename RemoveConst<T>::type;
```
- 标准库提供了`std::remove_const std::remove_volatile std::remove_cv`。

**退化：**
- 我们知道，在参数值传递或者按值返回时，会进行自动退化。
    - 数组变指针、函数变函数指针。
    - 移除引用。
    - 移除顶层CV限定。
- 标准库`std::decay`做这个事情，典型实现：
```C++

template< class T >
struct decay {
private:
    typedef typename std::remove_reference<T>::type U;
public:
    typedef typename std::conditional< 
        std::is_array<U>::value,
        typename std::remove_extent<U>::type*,
        typename std::conditional< 
            std::is_function<U>::value,
            typename std::add_pointer<U>::type,
            typename std::remove_cv<U>::type
        >::type
    >::type type;
};
```
- 这里的逻辑是，如果是引用，先移除引用，然后如果是数组则变为指针，如果是函数则变为函数指针，否则移除顶层CV限定。
- 当然也可以通过偏特化来做，这里就不赘述了。

谓词特征：
- 前面的特征都是单个类型的类型函数。更一般地，我们还可以定义依赖多个参数的类型函数，下面讨论这些特殊形式的类型特征与类型谓词（输出一个bool值的类型函数）。

**`IsSame`**：
- 判断两个类型是否相同。
```C++
template<typename T1, typename T2>
struct IsSame
{
    static constexpr bool value = false;
};
template<typename T>
struct IsSame<T, T>
{
    static constexpr bool value = true;
};
template<typename T1, typename T2>
constexpr bool IsSame_v = IsSame<T1, T2>::value;
```
- 对于产出常量的类型函数，我们不能定义类型别名，但是可以通过提供一个`constexpr`变量模板来做到同样的事情。`constexpr`变量是隐式`inline`的，不必再显式添加`inline`。
- C++标准库提供了`std::is_same`。

**`std::true_type`和`std::false_type`**：
- 我们可以提供一个类模板来产出两个不同的`bool`值。
```C++
template<bool val>
struct BoolConstant
{
    using type = BoolConstant<val>;
    static constexpr bool value = val;
};
using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;
```
- 然后需要产出`bool`值的类型函数可以直接派生自这两个类型以简化定义：
```C++
template<typename T1, typename T2>
struct IsSame2 : public TrueType
{
};
template<typename T>
struct IsSame2<T, T> : public FalseType
{
};
template<typename T1, typename T2>
constexpr bool IsSame2_v = IsSame<T1, T2>::value;
```
- 除了产出一个`bool`类型的`value`常量。还可以将`TrueType FalseType`用于类型分发。
```C++
template<typename T>
void fooImpl(T, TrueType)
{
    std::cout << "void fooImpl(T, TrueType)" << std::endl;
}
template<typename T>
void fooImpl(T, FalseType)
{
    std::cout << "void fooImpl(T, FalseType)" << std::endl;
}
```
- 当然从功能上来说，类型分发可以被编译期if所替代。
- 标准库提供了`std::true_type std::false_type`，不需要自己再去定义。

结果类型特征（Results Type Traits）：
- 一个处理多个类型参数的类型函数的例子是结果类型特征。在编写运算符模板时非常有用。
- 比如我们可能很想要支持两个序列的加法操作：
```C++
template<typename T>
std::vector<T> operator+(const std::vector<T>& lhs, const std::vector<T>& rhs);
```
- 下一步我们可能很自然想要支持两种不同数据类型容器的加法操作，就像内置类型的`int`和`double`都可以相加一样，此时需要面临的一个问题是：结果类型是什么？我们可能需要一个类型函数来为我们决定结果类型是什么。
```C++
template<typename T1, typename T2>
std::vector<PlusResult_t<T1, T2>> operator+(const std::vector<T1>& lhs, const std::vector<T2>& rhs);
```
- 那么这个结果类型应该怎样定义呢？我们可以使用`decltype`进行推导：
```C++
template<typename T1, typename T2>
struct PlusResult
{
    using type = decltype(T1() + T2());
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;
```
- 这里存在一个问题就是`decltype`需要的信息太多了，`operator+`返回值可能具有CV限定，并且我们的模板并不准备接受引用类型。
- 为此我们可以通过`remove_reference remove_cv`或者直接使用`std::decay`来得到合适的结果类型，但最后一个问题是，这两个类型可能不能进行默认初始化，导致编译错误：
- 为了解决这个问题，可以使用前面提到过的`std::declval`（头文件`<utility>`），用在不求值表达式中。
```C++
template<typename T1, typename T2>
struct PlusResult
{
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;

template<typename T1, typename T2>
std::vector<std::decay_t<PlusResult_t<T1, T2>>> operator+(const std::vector<T1>& lhs, const std::vector<T2>& rhs);
```
- `std::declval`的定义就像是这个样子；
```C++
namespace std
{
    template<typename T>
    add_rvalue_reference_t<T> declval() noexcept;
}
```
- 这个函数是故意未定义的，以防止真的在运行时被调用，鉴于`std::declval`只应该用在不求值表达式中。
- 最后提供一个实现：
```C++
template<typename T1, typename T2>
auto operator+(const std::vector<T1>& lhs, const std::vector<T2>& rhs)
{
    std::vector<std::decay_t<PlusResult_t<T1, T2>>> res;
    res.reserve(std::max(lhs.size(), rhs.size()));
    int i = 0;
    while (i < lhs.size() && i < rhs.size())
    {
        res.push_back(lhs[i] + rhs[i]);
        i++;
    }
    while (i < lhs.size())
    {
        res.push_back(lhs[i]);
        i++;
    }
    while (i < rhs.size())
    {
        res.push_back(rhs[i]);
        i++;
    }
    return res;
}
```

## 基于SFINAE的特征

有两个途径可以实现基于SFINAE的特征：函数重载与模板偏特化。

使用函数重载实现基于SFINAE的特征：
- 第一个例子：判断类是否可以默认初始化。
```C++
template<typename T>
struct IsDefaultConstructible
{
private:
    // test for types who has a default constructor
    template<typename U, typename = decltype(U())>
    static char test(void*);
    // test fallback
    template<typename>
    static long test(...);
public:
    static constexpr bool value = std::is_same_v<decltype(test<T>(nullptr)), char>;
};
template<typename T>
constexpr bool IsDefaultConstructible_v = IsDefaultConstructible<T>::value;
```
- 这里借助SFINAE机制以及函数重载来实现了特征的功能。
- 当然在C++20中我们完全可以使用概念来做。
- 注意这里必须将`test`定义为成员模板，并且第二个模板参数使用第一个模板参数来进行初始化，而不能直接使用`decltype(T())`。
- 在早期版本中，我们可以使用`enum { value = sizeof(test<T>(0)) == 1 }`这种技巧来实现。

基于SFAINE的谓词特征：
- 我们可以将上述测试逻辑的返回值定义为`std::true_type std::false_type`，最后直接`using type = decltype(test<T>(nullptr))`。
- 当然这样得到的结果是类型，所以上面需要实现在辅助类中，比如命名为`IsDefaultConstructibleHelper`，要得到bool值结果可以使用继承：
```C++
template<typename T>
struct IsDefaultConstructible2Helper
{
private:
    // test for types who has a default constructor
    template<typename U, typename = decltype(U())>
    static std::true_type test(void*);
    // test fallback
    template<typename>
    static std::false_type test(...);
public:
    using type = decltype(test<T>(nullptr));
};

template<typename T>
struct IsDefaultConstructible2 : public IsDefaultConstructible2Helper<T>::type
{};
template<typename T>
constexpr bool IsDefaultConstructible2_v = IsDefaultConstructible2<T>::value;
```

使用偏特化实现基于SFINAE的特征：
```C++
template<typename...>
using VoidT = void;

// primary template
template<typename, typename=VoidT<>>
struct IsDefaultConstructible : std::false_type {};

// partial sepcilization for type that can be default constructed
template<typename T>
struct IsDefaultConstructible<T, VoidT<decltype(T())>> : std::true_type {};

template<typename T>
constexpr bool IsDefaultConstructible_v = IsDefaultConstructible<T>::value;
```
- 比较有趣的点在于，第二个模板参数有一个默认参数，并且这个默认参数是一个可变参数类模板，在偏特化时通过显式指定这个模板实参来进行SFINAE。因为是可变参数类模板，所以可以通过指定多个实参来要求多个条件得到满足。
- C++17起标准库`<type_traits>`中提供了`std::void_t`，和上面的`VoidT`一样：是将任意类型的序列映射到类型`void`的工具元函数。用于检测SFINAE语境中的非良构类型。
```C++
template<typename...>
using void_t = void;
```

使用泛型lambda来实现SFINAE：
- 不管是函数重载还是类模板偏特化，我们都需要一个SFINAE的样板来实现特征。
- 在C++17中，我们可以通过亿点点泛型lambda的技巧将这个样板最小化：
```C++
// helper: checking validity of f(args...) for F f and Args... args
template<typename F, typename... Args,
    typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

// fallback if helpe SFINAE'd out
template<typename F, typename... Args>
std::false_type isValidImpl(...);

// define a lambda that takes a lambda f and return whether calling f with args is valid
inline constexpr auto isValid = [](auto f) {
    return [](auto&&... args) {
        return decltype(isValidImpl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

// helper template to represent a type as a value
template<typename T>
struct TypeT
{
    using type = t;
};
// helper to wrap a tyep as a value
template<typename T>
constexpr auto type = TypeT<T>{};

// helper to unwarp a wrapped type in unevaluated contexts
template<typename T>
T valueT(TypeT<T>); // no definition needed
```
- 在这些难读的代码之后，先看一个例子，使用这个lambda来实现`IsDefaultConstructible`特征的功能：
```C++
constexpr auto isDefaultConstructible = 
    isValid([](auto x) -> decltype((void)decltype(valueT(x))()){});

template<typename T>
constexpr bool isDefaultConstructible_v = isDefaultConstructible(type<T>);
```
- `isValid`是一个特征工厂，传入一个对应的lambda，它会生成对应的特征检查对象。
- 在调用时则需要将类型使用`type`进行包装（成对象）传入这个检查对象中。
- 传入的lambda中，为了将对象还原为类型，需要使用`decltype(valueT(x))`。这就是传入的`type<xxx>`中的那个`xxx`，在上面的例子中，使用这个类型进行了默认构造，就表明这是判断类型能否默认构造的特征。
- 如果不需要还原为类型，则可以直接使用`valueT(x)`进行对象的操作。
- 不得不说这种实现方式太晦涩太技巧性了，就不做更多解读了。
- 例子2，判断某个类型是否有`first`成员的的特征：
```C++
constexpr auto hasFirst = 
    isValid([](auto x) -> decltype((void)valueT(x).first){});
```

**SFINAE友好的特征：**
- 一般来说，一个类型特征应该要回答一个特定的查询，并且这个过程中不能导致程序的的非良构。基于SFINAE的特征需要在SFAINE上下文中谨慎地处理可能的潜在问题。
- 回顾前面的例子：
```C++
template<typename T1, typename T2>
struct PlusResult
{
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;
```
- 在这个定义中`opertor +`的使用没有受到SFIANE的保护，如果程序尝试计算没有合适的加号运算符的类型，那么就会导致非良构直接报错。
- 如果我们现在有一个两个不能使用`operator+`的类型`A B`，但是我们通过重载`operator+`的方式提供一个特殊实现，让其容器类型`std::vector<A> std::vector<B>`能够相加。但是因为泛化的重载版本`operator+(std::vector<T1>, std::vector<T2>)`实例化`PlusResult<A, B>`失败，因为不在即时上下文中，得不到SFINAE保护。就会产生这里的问题。
- 为了解决这个问题，就需要将`PlusResult`特征变成SFINAE友好的特征。也就是说让出现在即时上下文中，实例化失败是软错误而不是硬错误。
- 解决方式：特化`PlusResult`，让其无论如何都能实例化成功，不可能实例化失败。具体做法是通过SFINAE来保护`PlusResult`的实例化，在两个类型不支持`operator+`时就不定义嵌套类型`type`。
```C++
template<typename, typename, typename = std::void_t<>>
struct HasPlus : public std::false_type {};

template<typename T1, typename T2>
struct HasPlus<T1, T2, std::void_t<decltype(std::declval<T1>() + std::declval<T2>())>> 
    : public std::true_type {};

template<typename T1, typename T2, bool = HasPlus<T1, T2>::value>
struct PlusResult
{
    using type = decltype(std::declval<T1>() + std::declval<T2>()); // for true
};
template<typename T1, typename T2>
struct PlusResult<T1, T2, false>
{
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;
```

## IsConvertible

在实践中，基于SFINAE的特征经常会非常复杂，看一个例子：
```C++
template<typename From, typename To>
struct IsConvertibleHelper
{
private:
    static void aux(To);
    template<typename F, typename = decltype(aux(std::declval<F>()))>
    static std::true_type test(void*);
    template<typename>
    static std::false_type test(...);
public:
    using type = decltype(test<From>(nullptr));
};

template<typename From, typename To>
struct IsConvertible : IsConvertibleHelper<From, To>::type {};

template<typename From, typename To>
using IsConvertible_t = IsConvertible<From, To>::type;

template<typename From, typename To>
constexpr bool IsConvertible_v = IsConvertible<From, To>::value;
```
- 注意到有几个特殊情况这里没有处理：
    - 转换为数组的情况应该始终得到false，但是这里没有处理。数组作为目标类型时，因为是利用函数参数来实现的，所以会自动退化。最终的含义变成了是否能够转换为指针。
    - 转换为函数的情况应该始终得到false，同理。
    - 转换为`void`的情况没有处理，因为`void`类型不能作为参数。
- 可以通过偏特化对这些情况进行处理：
```C++
// special cases
template<typename From, typename To, bool = std::is_void_v<To> || std::is_array_v<To> || std::is_function_v<To>>
struct IsConvertibleHelper
{
    using type = std::bool_constant<std::is_void_v<To> && std::is_void_v<From>>; // If From and To are both void, the convesion will be valid.
};

// normal cases
template<typename From, typename To>
struct IsConvertibleHelper<From, To, false>
{
private:
    static void aux(To);
    template<typename F, typename = decltype(aux(std::declval<F>()))>
    static std::true_type test(void*);
    template<typename>
    static std::false_type test(...);
public:
    using type = decltype(test<From>(nullptr));
};
```
- C++标准库提供了`std::is_convertible`。

## 检测成员

另一个涉及到基于SFINAE的特征的例子是：检测给定类型是否有一个给定名称的成员。

检测成员类型：
```C++
template<typename, typename = std::void_t<>>
struct HasSizeType : public std::false_type {};

template<typename T>
struct HasSizeType<T, std::void_t<typename T::size_type>> : public std::true_type {};

template<typename T>
constexpr bool HasSizeType_t = HasSizeType<T>::value;
```
- 上面提到很多次了，通过偏特化来实现基于SFINAE的特征。注意这里没有特殊访问权限，所以只能检测公有成员。

处理引用类型：
- 上面的检测成员类型如果传入引用类型则会失败。因为引用类型并没有成员，但是按照常理上来说此时检测其底层类型的成员类型可能是比较合理的。
- 可以这样实现：
```C++
template<typename, typename = std::void_t<>>
struct HasSizeType : public std::false_type {};

template<typename T>
struct HasSizeType<T, std::void_t<typename std::decay_t<T>::size_type>> : public std::true_type {};

template<typename T>
constexpr bool HasSizeType_t = HasSizeType<T>::value;
```

检测任意成员类型：
- 很遗憾的是当前只能通过宏来实现：
```C++
#define DEFINE_HAS_TYPE(MemType)\
    template<typename, typename = std::void_t<>>\
    struct HasMemberType_##MemType : public std::false_type {};\
    template<typename T>\
    struct HasMemberType_##MemType<T, std::void_t<typename std::decay_t<T>::MemType>> : public std::true_type {};\
    template<typename T>\
    constexpr bool HasMemberType_##MemType##_t = HasMemberType_##MemType<T>::value;
```
- 未来也许可以通过反射实现。

检测非类型成员：
- 同样为了检测所有任意成员类型，需要使用宏。
```C++
#define DEFINE_HAS_MEMBER(Member)\
    template<typename, typename = std::void_t<>>\
    struct HasMember_##Member : public std::false_type {};\
    template<typename T>\
    struct HasMember_##Member<T, std::void_t<decltype(&T::Member)>> : public std::true_type {};\
    template<typename T>\
    constexpr bool HasMember_##Member##_v = HasMember_##Member<T>::value;
```
- 这里可以同时检测数据成员或者成员函数，并且会包含静态成员。
- 不能有歧义，比如有成员函数重载时取地址会有歧义所以检测会失败。

检测成员函数：
- 前面的检测非类型成员只能检测单一无歧义的情况。但是检测重载的成员函数也是一个重要需求。
- 可以使用前面用过的技巧，用`std::declval`声明一个值，并在不求值表达式中去调用这个函数：
```C++
template<typename, typename = std::void_t<>>
struct Has_begin : public std::false_type {};

template<typename T>
struct Has_begin<T, std::void_t<decltype(std::declval<T>().begin())>> : public std::true_type {};

template<typename T>
constexpr bool Has_begin_t = Has_begin<T>::value;
```

检测其他表达式：
- 任何需要检测其是否有效的表达式都可以通过偏特化与`decltype(expression)`结合来检测。
- 其中需要使用某个类型的值时应该使用`std::declval<T>()`而不是显式以任何形式调用构造函数，那会对相关类型产生多余的要求。
- 比如检测两个类型是否支持`operator<`，可以使用`std::void_t<decltype<std::declval<T1>() < std::declval<T2>()>>`。
- 当然在C++20中，使用概念是最好的方式。

使用泛型lambda实现成员检测：
- 又到了看不懂的环节，虽然勉勉强强可以看懂可以用起来，但是这种技巧看起来并非凡人能够掌握并灵活应用的。至少我是不会用的。
- 惯例使用前述的几个基本labmda与包装类型：
```C++
// helper: checking validity of f(args...) for F f and Args... args
template<typename F, typename... Args,
    typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

// fallback if helpe SFINAE'd out
template<typename F, typename... Args>
std::false_type isValidImpl(...);

// define a lambda that takes a lambda f and return whether calling f with args is valid
inline constexpr auto isValid = [](auto f) {
    return [](auto&&... args) {
        return decltype(isValidImpl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

// helper template to represent a type as a value
template<typename T>
struct TypeT
{
    using type = T;
};
// helper to wrap a tyep as a value
template<typename T>
constexpr auto type = TypeT<T>{};

// helper to unwarp a wrapped type in unevaluated contexts
template<typename T>
T valueT(TypeT<T>); // no definition needed
```
- 然后实现成员检测：
```C++
// detecting member
constexpr auto hasSizeType = 
    isValid([](auto x) -> typename decltype(valueT(x))::size_type {});
template<typename T>
constexpr bool hasSizeType_v = hasSizeType(type<T>);

constexpr auto hasFirst = 
    isValid([](auto x) -> decltype((void)valueT(x).first) {});
template<typename T>
constexpr bool hasFirst_t = hasFirst(type<T>);
```

## 其他特征技术

**IF-THEN-ELSE**：
```C++
template<bool Condition, typename TrueType, typename FalseType>
struct IfThenElse
{
    using type = TrueType;
};
template<typename TrueType, typename FalseType>
struct IfThenElse<false, TrueType, FalseType>
{
    using type = FalseType;
};

template<bool Condition, typename TrueType, typename FalseType>
using IfThenElse_t = typename IfThenElse<Condition, TrueType, FalseType>::type;
```
- 不同于普通的if-else逻辑，这里的`IfThenElse`的then和else分支都会被求值。所以两个分支都不能是非良构的。
- 为了避免两个分支都会求值导致的问题，可以加一层间接层次（将`TrueType FalseType`用`typename xxx<T>::type`代替）。
- 标准库提供了`std::conditional`做这个事情。

检查不抛出异常操作：
- 某些时候能够检查一个操作是否会抛出异常是有用的。
- 比如说一个类的移动构造函数是否会抛出异常会取决于其成员移动构造时是否会抛出异常，那么其异常声明中可能会需要`IsNothrowMoveConstructible`这种特征。
```C++
template<typename T>
struct IsNothrowConstructible : public std::bool_constant<noexcept(T(std::declval<T>()))>
{};

template<typename T>
constexpr bool IsNothrowConstructible_v = IsNothrowConstructible<T>::value;
```
- 其中使用了`noexcept`运算符。
- 这里有一个问题是，这个特征不是SFINAE友好的，如果传入类型没有移动构造函数或者拷贝构造函数，那么会导致特征实例化失败。
- 所以在使用`noexcept`运算符之前，需要先确定移动操作是否合法。前面提到过，可以使用类模板偏特化实现。
```C++
template<typename T, typename = std::void_t<>>
struct IsNothrowConstructible : public std::false_type {};

template<typename T>
struct IsNothrowConstructible<T, std::void_t<decltype(T(std::declval<T>()))>> 
    : public std::bool_constant<noexcept(T(std::declval<T>()))> {};

template<typename T>
constexpr bool IsNothrowConstructible_v = IsNothrowConstructible<T>::value;
```
- C++标准库提供了`std::is_move_constructible std::is_nothrow_move_constructible`来做这件事。

特征的便利性：
- 无论一个特征是产出值还是产出类型，我们都需通过要`typename xxx::type`与`xxx::value`这样冗长的方式去使用。
- 为了简化书写，我们需要上面已经用过多次的技巧：
    - 对类型添加别名模板，通常来说命名时添加`_t`后缀。
    - 对产出值的特征，则是添加`constexpr bool`变量模板，通常来说命名时添加`_v`后缀。
- 在通常情况下使用别名模板与变量模板是没有问题的，但在某些上下文中，只能使用原版写法，需要特别注意。
- 相对于`typename xxx::type`使用别名模板的缺点：
    - 别名模板不能被特化，然而很多编写特征的技巧依赖于特化。别名模板无论如何都需要重定位到一个类模板。这无伤大雅，因为通常来说我们并不会直接用别名模板实现特征（最好不要这么做）。
    - 某些特征是可以提供给用户去特化或者偏特化的，用别名模板描述时需要对其指向的那个类模板进行特化。
    - 使用别名模板时始终会特化那个类型，而使用嵌套类型则不会始终实例化。（所以在前面提到的`IfThenElse`实现某些特征时就不能使用别名模板）。
- 实践建议是都是用类模板实现特征，并配以必要的`_t`别名模板与`_v`变量模板，以什么方式使用都行。

## 类型分类

某些时候知道一个类型是什么样子的类型是有用的：内建类型、指针类型、类类型等。接下来讨论怎样编写类型分类的特征。

**基础类型**：
```C++
template<typename T>
struct IsFundamentalType : public std::false_type {};

template<typename T>
constexpr bool IsFundamentalType_v = IsFundamentalType<T>::value;

#define MK_FUNDAMENTAL_TYPE(T)\
    template<>\
    struct IsFundamentalType<T> : public std::true_type {}

MK_FUNDAMENTAL_TYPE(bool);
MK_FUNDAMENTAL_TYPE(char);
MK_FUNDAMENTAL_TYPE(signed char);
MK_FUNDAMENTAL_TYPE(unsigned char);
MK_FUNDAMENTAL_TYPE(wchar_t);
MK_FUNDAMENTAL_TYPE(char16_t);
MK_FUNDAMENTAL_TYPE(char32_t);
MK_FUNDAMENTAL_TYPE(signed short);
MK_FUNDAMENTAL_TYPE(unsigned short);
MK_FUNDAMENTAL_TYPE(signed int);
MK_FUNDAMENTAL_TYPE(unsigned int);
MK_FUNDAMENTAL_TYPE(signed long);
MK_FUNDAMENTAL_TYPE(unsigned long);
MK_FUNDAMENTAL_TYPE(signed long long);
MK_FUNDAMENTAL_TYPE(unsigned long long);
MK_FUNDAMENTAL_TYPE(float);
MK_FUNDAMENTAL_TYPE(double);
MK_FUNDAMENTAL_TYPE(long double);
MK_FUNDAMENTAL_TYPE(std::nullptr_t);
```
- 类似的更加细分的类型可以有：`IsIntegral IsFloating`等。
- 标准库提供了：
    - `std::is_void` 
    - `std::is_null_pointer`
    - `std::is_integral`
    - `std::is_floating_point`
    - `std::is_fundamental`
    - `std::is_arithmetic`
    - `std::is_scalar`

**复合类型**：
- 复合类型包括：指针类型、左值和右值引用类型、成员指针类型、数组类型。
- 指针：仅在是指针类型时定义`base`嵌套类型，以支持SFINAE。
```C++
// pointer
template<typename T>
struct IsPointer : public std::false_type {};

template<typename T>
struct IsPointer<T*> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsPointer_v = IsPointer<T>::value;
```
- 引用：
```C++
// reference
// lvalue reference
template<typename T>
struct IsLvalueReference : public std::false_type {};

template<typename T>
struct IsLvalueReference<T&> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsLvalueReference_v = IsLvalueReference<T>::value;

// rvalue reference
template<typename T>
struct IsRvalueReference : public std::false_type {};

template<typename T>
struct IsRvalueReference<T&&> : public std::true_type
{
    using base = T;
};

template<typename T>
constexpr bool IsRvalueReference_v = IsRvalueReference<T>::value;

// reference
template <class T> struct IsReference      : std::false_type {};
template <class T> struct IsReference<T&>  : std::true_type {};
template <class T> struct IsReference<T&&> : std::true_type {};
```
- 数组：
```C++
// array
template<typename T>
struct IsArray : public std::false_type {};
template<typename T, std::size_t N>
struct IsArray<T[N]> : public std::true_type
{
    using base = T;
    static constexpr std::size_t size = N;
};
template<typename T>
struct IsArray<T[]> : public std::true_type
{
    using base = T;
    static constexpr std::size_t size = 0;
};
template<typename T>
constexpr bool IsArray_v = IsArray<T>::value;
```
- 成员指针：包含非静态数据成员指针以及非静态成员函数指针。
```C++
// pointer to member
template<typename T>
struct IsMemberPointer : public std::false_type {};
template<typename T, typename C>
struct IsMemberPointer<T C::*> : public std::true_type
{
    using memberType = T;
    using classType = C;
};
template<typename T>
constexpr bool IsMemberPointer_v = IsMemberPointer<T>::value;
```
- 标准库提供了上述所有特征：
    - `std::is_pointer`
    - `std::is_lvalue_reference`
    - `std::is_rvalue_reference`
    - `std::is_reference`
    - `std::is_array`，并且提供了`std::rank<> std::extent<>`来获取数组维度以及某个维度的长度。
    - `std::is_member_object`
    - `std::is_member_object_pointer`
    - `std::is_member_function_pointer`
    - `std::is_compound`

**函数类型**：
```C++
template<typename... Args>
struct TypeList {};

template<typename T>
struct IsFunction : std::false_type {};

template<typename R, typename... Args>
struct IsFunction<R(Args...)> : std::true_type
{
    using RetType = R;
    using Params = TypeList<Args...>;
    static constexpr bool variadic = false;
};
template<typename R, typename... Args>
struct IsFunction<R(Args..., ...)> : std::true_type
{
     using RetType = R;
    using Params = TypeList<Args...>;
    static constexpr bool variadic = true; // C-style varargs
};

template<typename T>
constexpr bool IsFunction_v = IsFunction<T>::value;
```
- 需要注意的是，其中的可变参数版本匹配的是C风格的可变参数函数。
- 注意还需要对CV限定、左值右值引用限定、`noexcept`修饰、C风格可变参数的各种排列组合进行偏特化。
- 标准库提供了`std::is_function`。

**类类型**：
```C++
template<typename T, typename = std::void_t<>>
struct IsClass : std::false_type {};
template<typename T>
struct IsClass<T, std::void_t<int T::*>> : std::true_type {};
template<typename T>
constexpr bool IsClass_v = IsClass<T>::value;
```
- 核心原理是类类型可以有成员。联合在C++中也被归类为类类型。
- 要区分联合与普通类则需要编译器提供支持，无法通过C++语言核心特性实现。
- C++标准库提供了`std::is_union`检查是否是联合，`std::is_class`检查是否是非联合类型，这里的`IsClass`则包含了两者。

**枚举类型**：
- 如果要我们来实现一个辨别类型是否是枚举类型的特征，只能是通过排除基础类型、复合类型（指针、引用、数组、成员指针）、函数以及类类型来实现。
- 不过如果是标注库实现的话可以采用编译器特性来做。
- 标注库提供了`std::is_enum`。

## 策略特征

前面编写的特征都是获取模板参数属性的特征，我们称之为**属性特征**（property traits），比如一个类型的分类、一个元素符对两个类型应用之后的结果类型等。
- 在此之外，一些特征定义了对待类型的方式，称之为**策略特征**（policy traits）。
- 前面已经提到过特征与策略并不是那么明确区分的两个概念，这里的策略特征通常来说指的是关联于模板形参的策略。
- 属性特征通常以类型函数的方式来实现，而策略特征通常将策略包装在成员函数中。

只读参数类型：
- 在C++中，我们通过`const`指针或者引用传递只读的大型对象以避免拷贝开销。
- 在对象拷贝开销比较小时，值传递还是引用指针传递可能并不重要，也可能非常重要（性能极端敏感场景）。
- 在模板中，我们无法预先知道模板参数是什么样的类型，拷贝开销如何。通过对象大小来判断并不一定合理，小对象也可能会有很大的拷贝开销，因为可能会有动态内存分配。
- 我们可能会想实现泛型组件时依赖这样一种策略：由用户来确定是值传递还是引用传递只读对象。
- 典型实现会像是这个样子的：
```C++
template<typename T>
struct RParam
{
    using type = std::conditional_t<(sizeof(T) <= 2*sizeof(void*))
                                     && std::is_trivially_copy_constructible_v<T>
                                     && std::is_trivially_move_constructible_v<T>,
                                    T,
                                    const T&>;
};
```
- 如果我们能够确定一个类型的小于两个指针大小并且是平凡拷贝构造和移动构造，那么它的拷贝开销一定是很小的。
- 用户可以对自己的类型进行特化。
- 使用时：
```C++
template<typename T1, typename T2>
void foo(RParam_t<T1> a, RParam_t<T2> b)
```
- 同一个形式对不同类型的只读参数可能是值传递也可能是`const`引用传递。
- 缺点也有很多：丧失了不少可读性、冗长、需要额外特殊处理、无法使用模板实参推导。所以看起来实用价值不大。
- 当然在模板中始终值传递配合`std::ref std::cref`与模板实参推导都是可行的。实践时怎么做可以自行灵活选择，这里只是介绍了又一种选择而已。

## 标准库中的特征

- C++11开始，标准库中就引入了`<type_traits>`，基本包含了这一章介绍的所有特征。
- 其中某些东西并非仅靠语言特性就能实现，而是需要编译器提供支持，比如`std::is_union`。
- 在需要使用特征时，推荐的做法都是在标准库中有的情况下优先使用标准库中的。
- 需要注意一些特征可能会有出人意料的行为（至少是对不了解的人来说），需要仔细阅读文档。
- [附录D](../AppendixD)对标准库特征提供了更具体的说明。
- C++标准库还定义了一些其他的策略特征和属性特征：
    - `std::char_traits`是一个用于字符串和IO流的策略特征。
    - 为了让算法能够更好的使用标准迭代器，提供了`std::iterator_traits`属性特征。
    - `std::numeric_limits`也是一个有用的属性特征模板。
    - C++11起`std::allocator_traits`策略特征来处理标准分配器的内存分配。

## 后记

特征的发展历史，略。
