#include <type_traits>
#include <iostream>
#include <string>
#include <utility>
#include <new>
#include <exception>
#include <cassert>

// ================================================ typelist ================================================
template<typename... Elements>
struct Typelist {};

// extract first element
template<typename List>
struct Front;

template<typename Head, typename... Tail>
struct Front<Typelist<Head, Tail...>>
{
    using type = Head;
};

template<typename T>
using Front_t = typename Front<T>::type;

// pop first element, get the rest typelist
template<typename List>
struct Back;

template<typename Head, typename... Tail>
struct Back<Typelist<Head, Tail...>>
{
    using type = Typelist<Tail...>;
};

template<typename T>
using Back_t = typename Back<T>::type;

// largest type
template<typename List>
struct LargestType
{
private:
    using First = Front_t<List>;
    using Rest = typename LargestType<Back_t<List>>::type;
public:
    using type = std::conditional_t<(sizeof(First) > sizeof(Rest)), First, Rest>;
};
template<>
struct LargestType<Typelist<>>
{
    using type = char; // the smallest type
};
template<typename List>
using LargestType_t = typename LargestType<List>::type;

// ================================================ storage ================================================

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

// ================================================ design ================================================

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
    constexpr static unsigned Discriminator = FindIndexOf<Typelist<Types...>, T>::value + 1; // compute the discriminator to be used for this type, 0 is reserved for no value.
public:
    VariantChoice() {}
    VariantChoice(const T& value);
    VariantChoice(T&& value);
    bool destroy();
    Derived& operator=(const T& value);
    Derived& operator=(T&& value);
};

// an incomplete sentinel type for result type
class ComputedReusltType;

// visit result type
template<typename R, typename Visitor, typename... ElementTypes>
struct VisitResultT
{
    using type = R;
};

// not sepcifying the result type
template<typename Visitor, typename... ElementTypes>
struct VisitResultT<ComputedReusltType, Visitor, ElementTypes...>
{
    using type = std::common_type_t<decltype(std::declval<Visitor>()(std::declval<ElementTypes>()))...>;
};
template<typename R, typename Visitor, typename... ElementTypes>
using VisitResult = typename VisitResultT<R, Visitor, ElementTypes...>::type;

// definition of Variant
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

    // visit
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, Types&...> visit(Visitor&& vis) &;
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, const Types&...> visit(Visitor&& vis) const &;
    template<typename R = ComputedReusltType, typename Visitor>
    VisitResult<R, Visitor, Types&&...> visit(Visitor&& vis) &&;

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

// is
template<typename... Types>
template<typename T>
bool Variant<Types...>::is() const
{
    return this->getDiscriminator() == VariantChoice<T, Types...>::Discriminator;
}

// get
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

template<typename... Types>
template<typename T>
const T& Variant<Types...>::get() const&
{
    if (empty())
    {
        throw EmptyVariant();
    }
    assert(is<T>());
    return *this->template getBufferAs<T>();
}

template<typename... Types>
template<typename T>
T&& Variant<Types...>::get() &&
{
    if (empty())
    {
        throw EmptyVariant();
    }
    assert(is<T>());
    return std::move(*this->template getBufferAs<T>());
}

// element initialization, assignemnt and destruction
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

// empty of Variant
template<typename... Types>
bool Variant<Types...>::empty() const
{
    return this->getDiscriminator() == 0;
}

// destroy of Variant
template<typename... Types>
void Variant<Types...>::destroy()
{
    (..., VariantChoice<Types, Types...>::destroy());
    this->setDiscriminator(0);
}

// visit
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
        return variantVisitImpl<R>(std::forward<V>(variant),
                                    std::forward<Visitor>(vis),
                                    Typelist<Tail...>());
    }
    else
    {
        throw EmptyVariant(); // no value
    }
}

template<typename... Types>
template<typename R, typename Visitor>
VisitResult<R, Visitor, Types&...> Variant<Types...>::visit(Visitor&& vis) &
{
    using Result = VisitResult<R, Visitor, Types&...>;
    return variantVisitImpl<Result>(*this, std::forward<Visitor>(vis), Typelist<Types...>());
}

template<typename... Types>
template<typename R, typename Visitor>
VisitResult<R, Visitor, const Types&...> Variant<Types...>::visit(Visitor&& vis) const &
{
    using Result = VisitResult<R, Visitor, const Types&...>;
    return variantVisitImpl<Result>(*this, std::forward<Visitor>(vis), Typelist<Types...>());
}

template<typename... Types>
template<typename R, typename Visitor>
VisitResult<R, Visitor, Types&&...> Variant<Types...>::visit(Visitor&& vis) &&
{
    using Result = VisitResult<R, Visitor, Types&&...>;
    return variantVisitImpl<Result>(std::move(*this), std::forward<Visitor>(vis), Typelist<Types...>());
}

// constructors
template<typename... Types>
Variant<Types...>::Variant() // default constructor
{
    *this = Front_t<Typelist<Types...>>();
}

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

// assignment
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

template<typename... Types>
template<typename... SourceTypes>
Variant<Types...>& Variant<Types...>::operator=(const Variant<SourceTypes...>& source)
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
template<typename... SourceTypes>
Variant<Types...>& Variant<Types...>::operator=(Variant<SourceTypes...>&& source)
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

using namespace std::literals;
int main(int argc, char const *argv[])
{
    // test
    // is and get
    {
        Variant<int, short, double, std::string> v("hello");
        assert(v.is<std::string>());
        assert(v.get<std::string>() == "hello");
    }
    {
        const Variant<int, short, double, std::string> v("hello");
        assert(v.is<std::string>());
        assert(v.get<std::string>() == "hello");
    }
    {
        assert((Variant<int, short, double, std::string>("hello")).is<std::string>());
        assert((Variant<int, short, double, std::string>("hello")).get<std::string>() == "hello");
    }
    // constructors
    // default constructor
    {
        Variant<int, short, double, std::string> v;
        assert(v.is<int>());
        assert(v.get<int>() == 0);
    }
    // from values
    {
        short a = 10;
        Variant<int, short, double, std::string> v(a);
        assert(v.is<short>());
        assert(v.get<short>() == a);
    }
    {
        int a = 100;
        Variant<int, short, double, std::string> v(a);
        assert(v.is<int>());
        assert(v.get<int>() == a);
    }
    {
        double a = 10.1;
        Variant<int, short, double, std::string> v(a);
        assert(v.is<double>());
        assert(v.get<double>() == a);
    }
    {
        std::string s = "hello";
        Variant<int, short, double, std::string> v(s);
        assert(v.is<std::string>());
        assert(v.get<std::string>() == s);
        Variant<int, short, double, std::string> v2(std::move(s));
        assert(v2.get<std::string>() == "hello");
        assert(s == "");
    }
    // copy constructor
    {
        Variant<int, short, double, std::string> v("hello");
        Variant<int, short, double, std::string> v2(v);
        assert(v2.is<std::string>());
        assert(v.get<std::string>() == v2.get<std::string>());
        Variant<int, const char*> v3("hello");
        Variant<int, short, double, std::string> v4(v3);
        assert(v4.get<std::string>() == v3.get<const char*>());
    }
    // move constructor
    {
        Variant<int, short, double, std::string> v("hello");
        Variant<int, short, double, std::string> v2(std::move(v));
        assert(v2.get<std::string>() == "hello");
        assert(v.get<std::string>() == ""); // just move the std::string
        assert(v.is<std::string>());
        Variant<double, std::string> v3(std::move(v2));
        assert(v3.get<std::string>() == "hello");
        assert(v2.get<std::string>() == "");
    }
    // assignment
    {
        Variant<int, double, std::string> v("yes");
        v = 1;
        assert(v.is<int>());
        assert(v.get<int>() == 1);
        v = 1.1;
        assert(v.is<double>());
        assert(v.get<double>() == 1.1);
        v = "yes";
        assert(v.is<std::string>());
        assert(v.get<std::string>() == "yes");
        std::string s = "hello";
        v = std::move(s);
        assert(v.get<std::string>() == "hello");
        assert(s == "");
        Variant<int, double, std::string> v2;
        v2 = v;
        assert(v2.get<std::string>() == v.get<std::string>());
        v2 = std::move(v);
        assert(v2.get<std::string>() == "hello");
        assert(v.get<std::string>() == "");
        Variant<double, std::string> v3;
        v3 = v2;
        assert(v3.get<std::string>() == "hello");
        assert(v2.get<std::string>() == "hello");
        v3 = std::move(v2);
        assert(v3.get<std::string>() == "hello");
        assert(v2.get<std::string>() == "");
    }
    // empty and destroy
    {
        Variant<int, double, std::string> v("hello");
        v.destroy();
        assert(v.empty());
        assert(!v.is<std::string>());
    }
    return 0;
}
