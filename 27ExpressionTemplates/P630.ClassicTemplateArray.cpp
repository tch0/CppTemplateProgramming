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

template<typename T>
void printArray(const SArray<T>& arr)
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
    SArray<int> a(10);
    printArray(a);
    a = a + 1;
    a = a * 3;
    printArray(a);
    return 0;
}
