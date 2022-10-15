#include <iostream>
#include <type_traits>
#include <cstddef>
#include <cassert>

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

// class that representat the addition of two operands
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

// class that representat the multiplication of two operands
template<typename T, typename OP1, typename OP2>
class A_Mult
{
private:
    typename A_Traits<OP1>::ExprRef op1; // first operand
    typename A_Traits<OP2>::ExprRef op2; // second operand
public:
    A_Mult(const OP1& a, const OP2& b) : op1(a), op2(b) {}
    // compute product when value requested
    T operator[](std::size_t idx) const
    {
        return op1[idx] * op2[idx];
    }
    // maximum size
    std::size_t size() const
    {
        assert(op1.size() == 0 || op2.size() == 0 || op1.size() == op2.size());
        return op1.size() != 0 ? op1.size() : op2.size();
    }
};

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

// operators
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

template<typename T, typename R1, typename R2>
Array<T, A_Mult<T, R1, R2>> operator*(const Array<T, R1>& a, const Array<T, R2>& b)
{
    return Array<T, A_Mult<T, R1, R2>>(A_Mult<T, R1, R2>(a.rep(), b.rep()));
}

template<typename T, typename R1>
Array<T, A_Mult<T, R1, A_Scalar<T>>> operator*(const Array<T, R1>& a, const T& b)
{
    return Array<T, A_Mult<T, R1, A_Scalar<T>>>(A_Mult<T, R1, A_Scalar<T>>(a.rep(), A_Scalar<T>(b)));
}

template<typename T, typename R2>
Array<T, A_Mult<T, A_Scalar<T>, R2>> operator*(const T& a, const Array<T, R2>& b)
{
    return Array<T, A_Mult<T, A_Scalar<T>, R2>>(A_Mult<T, A_Scalar<T>, R2>(A_Scalar<T>(a), b.rep()));
}

template<typename T, typename Rep>
void printArray(const Array<T, Rep>& arr)
{
    assert(arr.size() > 0);
    std::cout << "SArray[" << arr.size() << "]: ";
    for (std::size_t i = 0; i < arr.size(); ++i)
    {
        std::cout << arr[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[])
{
    Array<double> x(10);
    Array<double> y(10);
    auto res = (x+1.0)*1.2 + (x+1.0)*(y+1.0);
    printArray(res);
    return 0;
}
