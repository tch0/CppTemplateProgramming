#include <iostream>

template<typename T>
struct Array
{
    using iterator = T*;
};

template<typename T>
void f(typename Array<T>::iterator first, typename Array<T>::iterator last); // 1

template<typename T>
void f(T*, T*); // 2

int main(int argc, char const *argv[])
{
    f<int&>(nullptr, nullptr); // will trigger a hard error, when instantiate the first version of f
    return 0;
}
