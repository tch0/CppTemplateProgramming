#include <iostream>

class S {};

template<typename T>
class Wrapper
{
private:
    T object;
public:
    Wrapper(T obj) : object(obj) {}
    friend void foo(const Wrapper<T>&)
    {
        std::cout << "foo" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    S s;
    Wrapper<S> w(s);
    foo(w);
    // foo(s); // ERROR: can not find foo through ADL
    return 0;
}
