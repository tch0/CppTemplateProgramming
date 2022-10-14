<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十六章：可识别的联合](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E5%85%AD%E7%AB%A0%E5%8F%AF%E8%AF%86%E5%88%AB%E7%9A%84%E8%81%94%E5%90%88)
  - [存储](#%E5%AD%98%E5%82%A8)
  - [设计](#%E8%AE%BE%E8%AE%A1)
  - [值查询与提取](#%E5%80%BC%E6%9F%A5%E8%AF%A2%E4%B8%8E%E6%8F%90%E5%8F%96)
  - [元素初始化、赋值和销毁](#%E5%85%83%E7%B4%A0%E5%88%9D%E5%A7%8B%E5%8C%96%E8%B5%8B%E5%80%BC%E5%92%8C%E9%94%80%E6%AF%81)
  - [访问](#%E8%AE%BF%E9%97%AE)
  - [Variant初始化与赋值](#variant%E5%88%9D%E5%A7%8B%E5%8C%96%E4%B8%8E%E8%B5%8B%E5%80%BC)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十六章：可识别的联合

上一章讨论的元组是对应于C++的结构的。
- 很自然想到可以实现一个对应于内建的联合的类型安全的联合，即是能够知道目前内部存储了什么类型的值的联合。
- C++17标准库提供了`std::variant`。这里将实现一个简化版，接口有区别，但基本原理差不多。
- 这是一个可变参数类模板，可以存储多种类型值，比如`std::variant<int, double, std::string>`，但同一时刻只能存储一个值。

## 存储

- 首先可能会先到像`Tuple`一样每个类型值存一份，虽然实现简单，但是空间利用率太低了。
- 其次可以像定义`Tuple`一样定义一个联合来存储。
```C++
template<typename... Types>
union VariantStorage;

template<typename Head, typename... Tail>
union VariantStorage<Head, Tail...>
{
    Head head;
    VariantStorage<Tail...> tail;
};

template<>
union VariantStorage<> {};

template<typename... Types>
class Variant
{
public:
    VariantStorage<Types...> storage;
    unsigned char discriminator;
};
```
- 这种定义方法没有空间浪费，但可能会有对齐问题，并且需要所有类型都能够默认构造。
- 所以这里我们选择直接用一个底层的buffer（字符数组）来存储类型，外加一个标志用来区分类型。使用`alignas`说明符解决对齐问题：
```C++
template<typename... Types>
class VariantStorage
{
private:
    using LargestT = LargestType_t<Typelist<Types...>>;
    alignas(Types...) unsigned char buffer[sizeof(LargestT)];
    unsigned char discriminator = 0;
public:
    unsigned char getDiscriminator() const { return discriminator; }
    void setDiscriminator(unsigned char d) { discriminator = d; }
    void* getRawBuffer() { return buffer; }
    const void* getRawBuffer() const { return buffer; }
    template<typename T>
    T* getBufferAs()
    {
        return std::launder(reinterpret_cast<T*>(buffer)); // pointer optimization barrier
    }
    template<typename T>
    const T* getBufferAs() const
    {
        return std::launder(reinterpret_cast<const T*>(buffer));
    }
};
```
- 其中typelist相关基础设施见[第24章](../24Typelists)。
- `alignas(Types...)`确保了这个`buffer`对所有类型都会有合适的对齐。编译器会自动计算需要的最大对齐，不需要我们写一个元程序来手动计算。
- `std::launder`实际不做任何事情，作用是保持地址一致，抑制可能的不正确的优化（一般是抑制错误的常量传播）。主要是用于`reinterpret_cast`前后指针可互相转换导致可能会改变地址的场景（用在这里提供正确性保证）。包含在`<new>`中。

## 设计

`Variant`类设计如下：
```C++
// foward declaration
template<typename... Types>
class Variant;

// find index of a specific type
template<typename List, typename T, unsigned N = 0> // recursive case
struct FindIndexOf : public std::conditional_t<std::is_same_v<Front_t<List>, T>,
                                               std::integral_constant<unsigned, N>,
                                               FindIndexOf<Back_t<List>, T, N+1>>
{
};

template<typename T, unsigned N>
struct FindIndexOf<Typelist<>, T, N> {}; // basis case


// VariantChoice: the CRTP base of Variant, no storage, just for accessing the data.
template<typename T, typename... Types>
class VariantChoice
{
private:
    using Derived = Variant<Types...>;
    Derived& getDerived() { return *static_cast<Derived*>(this); }
    const Derived& getDerived() const { return *static_cast<const Derived*>(this); }
protected:
    constexpr static unsigned Discriminator = FindIndexOf<Typelist<Types...>, T>::value + 1; // compute the discriminator to be used for this type
public:
    VariantChoice() {}
    VariantChoice(const T& value);
    VariantChoice(T&& value);
    bool destroy();
    Derived& operator=(const T& value);
    Derived& operator=(T&& value);
};

// definition of Variant
template<typename... Types>
class Variant : private VariantStorage<Types...>, private VariantChoice<Types, Types...>...
{
    template<typename T, typename... OtherTypes>
    friend class VariantChoice; // enable CRTP
};
```
- 通过多个空基类来实现各个类型的行为，获取各个类型的值。每个基类中都保存了这个类型的索引。为了能够在基类中访问派生类的成员。使用了CRTP定义基类（实际上并没有显式使用CRTP，不过处理是类似的），因为他们的接口并不是最终接口的一部分，所以需要使用私有继承，并将在派生类中将基类定义为友元以便支持基类中向下转换为派生类`Variant`。
- `private VariantChoice<Types, Types...>...`是一个嵌套包展开，会先展开内层，再展开外层。
- 当`VariantStorage`中的索引值与`VariantChoice`中的索引值匹配时，由该`VariantChoice`基类负责管理当前保存的值。
- `VariantChoice`中的值是实际找到的索引加`1`，所以索引有效取值范围是`1,2,...N`，`0`保留作为空值的情况，空值需要被妥当地处理。
- `Variant`的参数类型是不能重复的，重复会导致编译报错，因为这时这时会有两个相同的基类。（如果要支持两个相同类型参数的话，那就还需要在基类中加一个类似于索引的非类型模板参数以作区分）。

完整定义：
```C++
template<typename... Types>
class Variant : private VariantStorage<Types...>, private VariantChoice<Types, Types...>...
{
    template<typename T, typename... OtherTypes>
    friend class VariantChoice; // enable CRTP
public:
    // is and get
    template<typename T> bool is() const;
    template<typename T> T& get() &;
    template<typename T> const T& get() const&;
    template<typename T> T&& get() &&;

    // visist
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, Types&...> visist(Visitor&& vis) &;
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, const Types&...> visist(Visitor&& vis) const &;
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, Types&&...> visist(Visitor&& vis) &&;

    // constructors
    using VariantChoice<Types, Types...>::VariantChoice...; // inherit constructors
    Variant();
    Variant(const Variant& source);
    Variant(Variant&& source);
    template<typename... SourceTypes>
    Variant(const Variant<SourceTypes...>& source);
    template<typename... SourceTypes>
    Variant(Variant<SourceTypes...>&& source);

    // assignment
    using VariantChoice<Types, Types...>::operator=...;
    Variant& operator=(const Variant& source);
    Variant& operator=(Variant&& source);
    template<typename... SourceTypes>
    Variant& operator=(const Variant<SourceTypes...>& source);
    template<typename... SourceTypes>
    Variant& operator=(Variant<SourceTypes...>&& source);

    bool empty() const;

    ~Variant() { destroy(); }
    void destroy();
};
```
- 其中用到的一些东西还没有定义，大体结构已经有了。

## 值查询与提取

最常用的操作是查询当前是否保存了某一个类型的值：
- 如果查询失败，则会实例化失败报出编译错误，而不是出现运行时错误：
```C++
template<typename... Types>
template<typename T>
bool Variant<Types...>::is() const
{
    return this->getDiscriminator() == VariantChoice<T, Types...>::Discriminator;
}
```

提取值：
```C++
class EmptyVariant : public std::exception {};

template<typename... Types>
template<typename T>
T& Variant<Types...>::get() &
{
    if (empty())
    {
        throw EmptyVariant();
    }
    assert(is<T>());
    return *this->template getBufferAs<T>();
}
```
- 三个重载实现基本相同，如果为空，则抛出异常，如果不是要提取的类型，则会断言失败。右值版本需要对返回值做`std::move`，因为对指针解引用得到的是一个左值。

## 元素初始化、赋值和销毁

每个类型的初始化、赋值、销毁是由对应基类`VariantChoice`负责的，这里来实现这些内容。

**初始化**：
```C++
template<typename T, typename... Types>
VariantChoice<T, Types...>::VariantChoice(const T& value)
{
    new (getDerived().getRawBuffer()) T(value);
    getDerived().setDiscriminator(Discriminator);
}

template<typename T, typename... Types>
VariantChoice<T, Types...>::VariantChoice(T&& value)
{
    new (getDerived().getRawBuffer()) T(std::move(value));
    getDerived().setDiscriminator(Discriminator);
}
```
- 通过定位new构造对象，并设置索引表明存储的类型。
- 通过在`Variant`中继承所有基类的构造函数，实现了通过任意类型对象构造`Variant`，即使通过隐式转换。

**销毁**：
- 当`Variant`初始化后，一个对应类型对象就被构造到`buffer`中了，在设置其他值或者销毁整个`Variant`时就需要析构这个值。
```C++
template<typename T, typename... Types>
bool VariantChoice<T, Types...>::destroy()
{
    if (getDerived().getDiscriminator() == Discriminator)
    {
        getDerived().template getBufferAs<T>()->~T();
        return true;
    }
    return false;
}
```
- 每个基类只负责销毁它负责的那个类型，如果不是当前类型，就什么都不做。
- 但是在`Variant`中我们想要无论什么类型都能够直接销毁，可以通过直接调用所有基类`destroy`做到：
```C++
template<typename... Types>
void Variant<Types...>::destroy()
{
    // bool rusults[] = {
    //     VariantChoice<Types, Types...>::destroy()...
    // };
    (..., VariantChoice<Types, Types...>::destroy());
    this->setDiscriminator(0);
}
```
- 这里可以通过嵌套展开来做（数组初始化只是提供一个展开的上下文，实际上不使用结果），C++17后也可以通过折叠表达式来做，更简单。
- 销毁之后将状态设置为`0`，表示当前什么值都没有。

**赋值**：
```C++
template<typename T, typename... Types>
VariantChoice<T, Types...>::Derived& VariantChoice<T, Types...>::operator=(const T& value)
{
    // assign new value of same type
    if (getDerived().getDiscriminator() == Discriminator)
    {
        *getDerived().template getBufferAs<T>() = value;
    }
    // assign new value of different type
    else
    {
        getDerived().destroy();
        new (getDerived().getRawBuffer()) T(value);
        getDerived().setDiscriminator(Discriminator);
    }
    return getDerived();
}

template<typename T, typename... Types>
VariantChoice<T, Types...>::Derived& VariantChoice<T, Types...>::operator=(T&& value)
{
    // assign new value of same type
    if (getDerived().getDiscriminator() == Discriminator)
    {
        *getDerived().template getBufferAs<T>() = std::move(value);
    }
    // assign new value of different type
    else
    {
        getDerived().destroy();
        new (getDerived().getRawBuffer()) T(std::move(value));
        getDerived().setDiscriminator(Discriminator);
    }
    return getDerived();
}
```
- 如果传入新类型和原类型一样（或者是能够转换为该类型），那么直接赋值，否则就会先销毁再重新构造。
- 注意赋值时如果传入的不同类型也是设置为当前类型。因为由函数重载已经决定当前类型已经是最佳匹配，而不会更匹配另外一个类型。
- `Variant`中同样继承了所有赋值运算符，不需要再次定义。
- 第二个分支中分为两步的赋值需要考虑几个典型问题：
- **自赋值**：
    - 自赋值发生在`v = v.get<T>()`时，但自赋值意味着源类型和目标类型一定一致，所以一定是走第一个分支执行一步赋值过程，而不会发生两步赋值。所以不需要处理。
- **异常**：
    - 当一个不能拷贝构造且不能拷贝赋值时（这两者的状态通常是一致的），那一定不可能预先存在一个保存了该类型值的状态，所以一定是走第二个分支，最后拷贝构造抛出异常。因为预先已经调用了`destroy`，所以异常抛出时的状态一定是空`Variant`。这是一个合法状态，也就是说这个实现是异常安全的。
- **`std::launder`**：
    - C++编译器为了生成高性能的代码，会对代码做一些推断和假设，典型如某些量是不会变得：`const`数据、引用（其指向的对象）、虚表中的一些固定信息等。这里我们在编译期可能不知道的情况下去构造和析构了`buffer`中保存的对象，当编译器做出激进的优化时，可能就会出现问题，所以每一次取`buffer`地址时，我们都使用了`std::launder`（C++17引入）。
    - 这个问题非常微妙、难于感知、从代码中看不出来，需要特别注意。

## 访问

访问器（Visitors）：
- `Variant`只能存储一个值，`is get`提供了检查类型与提取值的操作，但在不知道任何信息的情况下查询其值就需要使用一长串的if-else。
- 如果要编写一个通用的访问其值的操作，就需要使用模板递归实例化，非常麻烦。
- 一个更直接的访问方式是将要做的事情作为一个函数传入`Variant`中，使用其内部数据去调用。
- 为了能够使任何数据类型都能够访问，传入的这个函数对象应该是一个泛型函数对象（比如泛型lambda或者泛型`operator()`的函数对象），或者对所有类型都重载了`operator()`的函数对象。
- 实现访问时同样需要模板递归，借助一个辅助函数模板实现：
```C++
// visit helper function template
template<typename R, typename V, typename Visitor, typename Head, typename... Tail>
R variantVisitImpl(V&& variant, Visitor&& vis, Typelist<Head, Tail...>)
{
    if (variant.template is<Head>()) // runtime if
    {
        return static_cast<R>(
            std::forward<Visitor>(vis)(
                std::forward<V>(variant).template get<Head>()));
    }
    else if constexpr (sizeof...(Tail) > 0)
    {
        return variantVisistImpl<R>(std::forward<V>(variant),
                                    std::forward<Visitor>(vis),
                                    Typelist<Tail...>())
    }
    else
    {
        throw EmptyVariant(); // no value
    }
}

template<typename... Types>
template<typename R = ComputedReusltType, typename Visitor>
VisitResult<R, Visitor, Types&...> Variant<Types...>::visit(Visitor&& vis) &
{
    using Result = VisitResult<R, Visitor, Types&...>;
    return variantVisitImpl<Result>(*this, std::forward<Visitor>(vis), Typelist<Types...>());
}

template<typename... Types>
template<typename R = ComputedReusltType, typename Visitor>
VisitResult<R, Visitor, const Types&...> Variant<Types...>::visit(Visitor&& vis) const &
{
    using Result = VisitResult<R, Visitor, const Types&...>;
    return variantVisitImpl<Result>(*this, std::forward<Visitor>(vis), Typelist<Types...>());
}

template<typename... Types>
template<typename R = ComputedReusltType, typename Visitor>
VisitResult<R, Visitor, Types&&...> Variant<Types...>::visit(Visitor&& vis) &&
{
    using Result = VisitResult<R, Visitor, Types&&...>;
    return variantVisitImpl<Result>(std::move(*this), std::forward<Visitor>(vis), Typelist<Types...>());
}
```
- 都是直接将访问操作委托给`variantVisitImpl`，这里的确定返回值类型的模板`VisitResult`尚未实现。

访问返回类型：
- 上面的代码中返回类型依然不明了，重载的或者泛型的`operator()`的返回类型可能是不同的或者依赖于参数类型。
- 所以我们允许显式提供返回值类型，也就是第一个模板参数。
- 但是总是这样指定会显得非常冗长，所以我们也提供了一个默认参数`ComputedReusltType`，这是一个用来占位的不完整类型：
```C++
class ComputedReusltType;
```
- 当不指定返回类型时，默认返回类型`ComputedReusltType`，这时将会通过`VisitResult`元程序推导出一个合适的类型来接受返回值。
- 指定了返回值类型的情况，则会使用指定的返回值类型：
```C++
template<typename R, typename Visitor, typename... ElementTypes>
struct VisitResultT
{
    using type = R;
};
template<typename R, typename Visitor, typename... ElementTypes>
using VisitResult = typename VisitResultT<R, Visitor, ElementTypes...>::type;
```
- 未指定的情况，针对`ComputedReusltType`偏特化即可，此时使用所有可能返回值类型的公共类型（即类型列表中所有类型都可以提升/转换到的类型）：
```C++
template<typename Visitor, typename... ElementTypes>
struct VisitResultT<ComputedReusltType, Visitor, ElementTypes...>
{
    using type = std::common_type_t<decltype(std::declval<Visitor>()(std::declval<ElementTypes>()))...>;
};
```
- `std::common_type`如果要实现的话可以通过`?:`运算符，使用`using type = decltype(true ? std::declval<T1>() : std::declval<T2>())`配合模板递归即可轻松实现。

## Variant初始化与赋值

默认构造，同`std::variant`默认构造为第一个类型的默认值：
```C++
template<typename... Types>
Variant<Types...>::Variant() // default constructor
{
    *this = Front_t<Typelist<Types...>>();
}
```
- 注意不应该默认构造为空，因为值为空通常来说是被认为一个错误状态（比如对于`std::variant`）。

拷贝、移动构造：
```C++
template<typename... Types>
Variant<Types...>::Variant(const Variant& source)
{
    if (!source.empty())
    {
        source.visit([&](const auto& value) {
            *this = value;
        });
    }
}

template<typename... Types>
Variant<Types...>::Variant(Variant&& source)
{
    if (!source.empty())
    {
        std::move(source).visit([&](auto&& value) {
            *this = std::move(value);
        });
    }
}
```

从兼容类型拷贝、移动构造：
```C++
template<typename... Types>
template<typename... SourceTypes>
Variant<Types...>::Variant(const Variant<SourceTypes...>& source)
{
    if (!source.empty())
    {
        source.visit([&](const auto& value) {
            *this = value;
        });
    }
}

template<typename... Types>
template<typename... SourceTypes>
Variant<Types...>::Variant(Variant<SourceTypes...>&& source)
{
    if (!source.empty())
    {
        std::move(source).visit([&](auto&& value) {
            *this = std::move(value);
        });
    }
}
```
- 此时需要每个源`Variant`类型都能够转换或者提升为存储在当前`Variant`中的类型，重载决议时会为每个类型找到最匹配的构造函数，执行必要的隐式类型转换。
- 如果源是空的，那么直接类内初始化索引为`0`，即整个`Variant`为空。

赋值：
- 和构造同样实现方法，只需要额外考虑源为空的情况需要清除当前`Variant`的值。
```C++
template<typename... Types>
Variant<Types...>& Variant<Types...>::operator=(const Variant& source)
{
    if (!source.empty())
    {
        source.visit([&](const auto& value) {
            *this = value;
        });
    }
    else
    {
        destroy();
    }
    return *this;
}

template<typename... Types>
Variant<Types...>& Variant<Types...>::operator=(Variant&& source)
{
    if (!source.empty())
    {
        std::move(source).visit([&](auto&& value) {
            *this = std::move(value);
        });
    }
    else
    {
        destroy();
    }
    return *this;
}
```
- 模板版本完全相同实现，略。
- `Variant`完整实现与测试见[P603.Variant.cpp](P603.Variant.cpp)。

## 后记

- C++17标准库引入的`std::variant`作为一个封闭的类型安全的联合（closed discriminated union）。类似于这里的`Variant`，但细节和接口有一些区别。
- 另一个开放的联合（open discriminated union）是C++17引入的`std::any`，他可以存储任何类型的值。他们都是从Boost里面来的。
