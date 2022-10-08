#include <iostream>
#include <type_traits>
#include <vector>

// primary template: for standard library containers
template<typename C>
struct ElementT
{
    using type = typename C::value_type;
};

template<typename T, std::size_t N>
struct ElementT<T[N]> {
    using type = T;
};
template<typename T>
struct ElementT<T[]> {
    using type = T;
};

// alias template for type function to avoid verbose writing
template<typename T>
using ElementType = typename ElementT<T>::type;

// transformation traits
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

// add reference
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

// another simple implementation
// it's risky, because it can't specialize (like for void)
template<typename T>
using AddLvalueReference2_t = T&;
template<typename T>
using AddRvalueReference2_t = T&&;

// removing qualifiers
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

// check whether two types are the same
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

// true_type and false_type
template<bool val>
struct BoolConstant
{
    using type = BoolConstant<val>;
    static constexpr bool value = val;
};
using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;

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

// using TrueType and FalseType for dispatching
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

// result type traits
template<typename T1, typename T2>
struct PlusResult
{
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};
template<typename T1, typename T2>
using PlusResult_t = typename PlusResult<T1, T2>::type;

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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "vector: ";
    for (auto & val : vec)
    {
        os << val << " ";
    }
    return os;
}


int main(int argc, char const *argv[])
{
    static_assert(std::is_same_v<ElementType<int[10]>, int>);
    static_assert(std::is_same_v<ElementType<std::vector<int>>, int>);
    static_assert(std::is_same_v<RemoveReference_t<int&&>, int>);
    static_assert(std::is_same_v<RemoveReference_t<int&>, int>);
    static_assert(std::is_lvalue_reference_v<AddLvalueReference_t<int>>);
    static_assert(std::is_rvalue_reference_v<AddRvalueReference_t<int>>);
    static_assert(std::is_lvalue_reference_v<AddLvalueReference2_t<int>>);
    static_assert(std::is_rvalue_reference_v<AddRvalueReference2_t<int>>);
    static_assert(std::is_same_v<RemoveConst_t<const int>, int>);
    static_assert(std::is_same_v<RemoveConst_t<const std::vector<int>>, std::vector<int>>);
    static_assert(IsSame_v<int, RemoveReference_t<int&&>>);
    static_assert(IsSame2_v<int, RemoveReference_t<int&&>>);

    std::vector<int> vec1{1, 2, 3, 4};
    std::vector<double> vec2{1.1, 2.2, 3.3, 4.4, 5.6, 9.9};
    std::cout << (vec1 + vec2) << std::endl;
    return 0;
}
