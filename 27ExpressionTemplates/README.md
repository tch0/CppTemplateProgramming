<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第二十七章：表达式模板](#%E7%AC%AC%E4%BA%8C%E5%8D%81%E4%B8%83%E7%AB%A0%E8%A1%A8%E8%BE%BE%E5%BC%8F%E6%A8%A1%E6%9D%BF)
  - [临时变量与分割循环](#%E4%B8%B4%E6%97%B6%E5%8F%98%E9%87%8F%E4%B8%8E%E5%88%86%E5%89%B2%E5%BE%AA%E7%8E%AF)
  - [在模板参数中编码表达式](#%E5%9C%A8%E6%A8%A1%E6%9D%BF%E5%8F%82%E6%95%B0%E4%B8%AD%E7%BC%96%E7%A0%81%E8%A1%A8%E8%BE%BE%E5%BC%8F)
  - [表达式模板的性能与约束](#%E8%A1%A8%E8%BE%BE%E5%BC%8F%E6%A8%A1%E6%9D%BF%E7%9A%84%E6%80%A7%E8%83%BD%E4%B8%8E%E7%BA%A6%E6%9D%9F)
  - [后记](#%E5%90%8E%E8%AE%B0)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第二十七章：表达式模板

本章探索一种C++模板编程技术，称之为表达式模板（expression template）：
- 最初表达式模板是被发明了用来支持数值数组类的。
- 数值数组是指支持对整个数组做数值计算的数组。比如`Array<int> a, b;`支持`a+b a*b`等操作的数组。
- 表达式模板和模板递归实例化是互补的。元编程适用于小的固定尺寸的数组，而表达式模板适用于运行时的中等或者大型数组。

## 临时变量与分割循环

首先直接使用模板实现数值数组的操作：
```C++
template<typename T>
class SArray
{
private:
    T* storage;
    std::size_t storage_size;
protected:
    void init()
    {
        for (std::size_t idx = 0; idx < size(); ++idx)
        {
            storage[idx] = T();
        }
    }
    void copy(const SArray<T>& source)
    {
        assert(size() == source.size());
        for (std::size_t idx = 0; idx < size(); ++idx)
        {
            storage[idx] = source.storage[idx];
        }
    }
public:
    explicit SArray(std::size_t s) : storage(new T[s]), storage_size(s)
    {
        init();
    }
    SArray(const SArray<T>& source) : storage(new T[source.size()]), storage_size(source.size())
    {
        copy(source);
    }
    ~SArray()
    {
        delete [] storage;
    }
    SArray& operator=(const SArray<T>& source)
    {
        if (&source != this)
        {
            copy(source);
        }
        return *this;
    }
    std::size_t size() const
    {
        return storage_size;
    }
    const T& operator[](std::size_t idx) const
    {
        return storage[idx];
    }
    T& operator[](std::size_t idx)
    {
        return storage[idx];
    }
};
```
- 然后我们还需要定义运算符：
```C++
// operators
template<typename T>
SArray<T> operator+(const SArray<T>& a, const SArray<T>& b)
{
    assert(a.size() == b.size());
    SArray<T> res(a.size());
    for (std::size_t idx = 0; idx < a.size(); ++idx)
    {
        res[idx] = a[idx] + b[idx];
    }
    return res;
}

template<typename T>
SArray<T> operator*(const SArray<T>& a, const SArray<T>& b)
{
    assert(a.size() == b.size());
    SArray<T> res(a.size());
    for (std::size_t idx = 0; idx < a.size(); ++idx)
    {
        res[idx] = a[idx] * b[idx];
    }
    return res;
}

template<typename T>
SArray<T> operator+(const T& a, const SArray<T>& b)
{
    SArray<T> res(b.size());
    for (std::size_t idx = 0; idx < b.size(); ++idx)
    {
        res[idx] = a + b[idx];
    }
    return res;
}

template<typename T>
SArray<T> operator+(const SArray<T>& a, const T& b)
{
    SArray<T> res(a.size());
    for (std::size_t idx = 0; idx < a.size(); ++idx)
    {
        res[idx] = a[idx] + b;
    }
    return res;
}

template<typename T>
SArray<T> operator*(const T& a, const SArray<T>& b)
{
    SArray<T> res(b.size());
    for (std::size_t idx = 0; idx < b.size(); ++idx)
    {
        res[idx] = a * b[idx];
    }
    return res;
}

template<typename T>
SArray<T> operator*(const SArray<T>& a, const T& b)
{
    SArray<T> res(a.size());
    for (std::size_t idx = 0; idx < a.size(); ++idx)
    {
        res[idx] = a[idx] * b;
    }
    return res;
}
// ...
```
- 这里只简单定义了几个，可能还需要更多。
- 这种做法有几个问题：
    - 每个运算符都使用了一个中间临时数组，当数组长度很大时这个开销是很大的。
    - 每个运算符的使用都会产生数组拷贝。
- 可以通过将运算符定义为`*= +=`等复合赋值运算符来避免临时对象和不必要的拷贝。
- 但是如此一来就必须这样用：
```C++
SArray<int> x(1000), y(1000);
SArray<int> tmp(x);
tmp *= y;
x *= 1.2;
x += tmp;
```
- 而不能直接写成一个式子`x = 1.2*x + x*y`，中间依然涉及了太多不必要的数据读取写入与多轮不必要的循环。
- 单论性能最佳的情况可能是直接写成裸循环，而不去用运算符重载：
```C++
for(std::size_t i = 0; i < x.size(); ++i)
{
    x[i] = 1.2*x[i] + x[i]*y[i];
}
```
- 这种方法能最小化数据的读取与写入，只需要一轮循环，也不会有临时对象。但是非常笨拙与容易出错。
- 很自然地，我们想要一种和手写的裸循环一个性能但是同时又易于表示和复用的方法。

## 在模板参数中编码表达式

很显然，解决这个问题的关键是要直到看到整个表达式的时候才去求值：
- 比如对于`1.2*x + x*y`这个表达式，不应该首先就将`1.2*x`计算出来，而是用`1.2*x`表示`1.2`与`x`相乘的计算结果，但不做实际计算。实际的计算等到要用到整个表达式的值的时候再去算。
- 这有点类似于函数式编程的**惰性求值**（lazy evaluation）。
- 在实现时，做法是将`1.2*x + x*y`这个表达式转化成一个类似于`A_Add<A_Mult<A_Scalar<double>, Array<double>>, A_Mult<Array<double>, Array<double>>>`的对象。

表达式模板的运算符：
- 表示加法的运算表达式定义：
```C++
template<typename T, typename OP1, typename OP2>
class A_Add
{
private:
    typename A_Traits<OP1>::ExprRef op1; // first operand
    typename A_Traits<OP2>::ExprRef op2; // second operand
public:
    A_Add(const OP1& a, const OP2& b) : op1(a), op2(b) {}
    // compute sum when value requested
    T operator[](std::size_t idx) const
    {
        return op1[idx] + op2[idx];
    }
    // maximum size, size of scalar is 0
    std::size_t size() const
    {
        assert(op1.size() == 0 || op2.size() == 0 || op1.size() == op2.size());
        return op1.size() != 0 ? op1.size() : op2.size();
    }
};
```
- 当通过`operator[]`用到结果时才去计算。
- 为了统一并区分数组和标量类型，将标量类型的`size`定义为`0`。
- 乘法操作同样方式定义。
- 标量类型定义：
```C++
template<typename T>
class A_Scalar
{
private:
    const T& s; // value of scalar
public:
    constexpr A_Scalar(const T& v) : s(v) {}
    // for index operations, always return the scalar itself
    constexpr const T& operator[](std::size_t) const
    {
        return s;
    }
    // scalars has size of 0
    constexpr std::size_t size() const
    {
        return 0;
    }
};
```
- 这里的`A_Traits`用来定义表达式的操作数类型，通常来说直接定义为`const&`即可，但是这里还可能引用到可能存活不到表达式完整求值的标量类型。所以需要对标量类型直接拷贝，对数组才取引用。
```C++
template<typename T> class A_Scalar;

// for array
template<typename T>
struct A_Traits
{
    using ExprRef = const T&;
};
// for scalar type
template<typename T>
struct A_Traits<A_Scalar<T>>
{
    using ExprRef = A_Scalar<T>;
};
```
- 注意表达式顶层的标量类型是可以取引用的，`A_Scalar`中就是保存的其引用。

`Array`类型：
- 在知道了表达式模板的存在的情况下，定义`Array`类型来管理实际的数组内存：将底层数组实现作为模板参数，实现基本一样的接口：
```C++
template<typename T, typename Rep = SArray<T>>
class Array
{
private:
    Rep arr; // data of array
public:
    // create array with initial size
    explicit Array(std::size_t s) : arr(s) {}
    // create array from possible representation
    Array(const Rep& r) : arr(r) {}
    // assignment for same type array
    Array& operator=(const Array& rhs)
    {
        assert(size() == rhs.size());
        for (std::size_t i = 0; i < rhs.size(); ++i)
        {
            arr[i] = rhs[i];
        }
        return *this;
    }
    // assignment for arrays of different type
    template<typename T2, typename Rep2>
    Array& operator=(const Array<T2, Rep2>& rhs)
    {
        assert(size() == rhs.size());
        for (std::size_t i = 0; i < rhs.size(); ++i)
        {
            arr[i] = rhs[i];
        }
        return *this;
    }
    // size
    std::size_t size() const
    {
        return arr.size();
    }
    // index operator
    decltype(auto) operator[](std::size_t idx) const
    {
        assert(idx < size());
        return arr[idx];
    }
    T& operator[](std::size_t idx)
    {
        assert(idx < size());
        return arr[idx];
    }
    // underlying array
    const Rep& rep() const
    {
        return arr;
    }
    Rep& rep()
    {
        return arr;
    }
};
```

最后就只差运算符了：
- 这些运算符符只需要组装表达式模板，不需要做实际的计算。
- 对于每个运算符都需要实现三个版本：数组-数组、数组-值、值-数组。
```C++
template<typename T, typename R1, typename R2>
Array<T, A_Add<T, R1, R2>> operator+(const Array<T, R1>& a, const Array<T, R2>& b)
{
    return Array<T, A_Add<T, R1, R2>>(A_Add<T, R1, R2>(a.rep(), b.rep()));
}

template<typename T, typename R1>
Array<T, A_Add<T, R1, A_Scalar<T>>> operator+(const Array<T, R1>& a, const T& b)
{
    return Array<T, A_Add<T, R1, A_Scalar<T>>>(A_Add<T, R1, A_Scalar<T>>(a.rep(), A_Scalar<T>(b)));
}

template<typename T, typename R2>
Array<T, A_Add<T, A_Scalar<T>, R2>> operator+(const T& a, const Array<T, R2>& b)
{
    return Array<T, A_Add<T, A_Scalar<T>, R2>>(A_Add<T, A_Scalar<T>, R2>(A_Scalar<T>(a), b.rep()));
}
```
- 同样实现`operator*`之后，我们就可以通过`1.2*x+x*y`这种表达式来操作了。并且其值只有在用到时（通过`operator[]`去取的时候）才会计算。

## 表达式模板的性能与约束

表达式模板并不能解决所有的数值数组操作问题，使用场景比较有限，比如它不能用在`x = A*x`的矩阵-数组相乘运算中。但是它确实可以在某些场景中用于优化数值数组的表达式操作。

## 后记

略。
