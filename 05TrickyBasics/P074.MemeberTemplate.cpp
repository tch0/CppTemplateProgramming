#include <iostream>

template<typename T = int>
class Foo
{
    template<typename>
    friend class Foo;
private:
    T val;
public:
    Foo(T _val = T{}) : val(_val) {} // T{} : zero initialization
    // member template
    template<typename U>
    Foo& operator=(const Foo<U>& foo)
    {
        val = foo.val;
        return *this;
    }
    // member template of a class template
    template<typename U1, typename U2>
    void print()
    {
        std::cout << "Foo<T>::print<U1, U2>: " << val << std::endl;
    }
    
};

// member template of specialiazation of class template
template<>
template<typename U1, typename U2>
void Foo<double>::print()
{
    std::cout << "Foo<double>::print<U1, U2>: " << val << std::endl;
}

// full specialization of member template of specilization of class template
template<>
template<>
void Foo<double>::print<bool, bool>()
{
    std::cout << "Foo<double>::print<bool, bool>:" << val << std::endl;
}

// full sepcialization of class template Foo
template<>
class Foo<bool>
{
    template<typename>
    friend class Foo;
private:
    bool val;
public:
    Foo(bool _val = false) : val(_val) {}
    // member template
    template<typename U>
    Foo& operator=(const Foo<U>& foo)
    {
        val = (foo.val != U{});
        return *this;
    }
    // member template of a fully specialized template class
    template<typename U1, typename U2>
    void print()
    {
        std::cout << std::boolalpha << "Foo<bool>::print<U1, U2>: " << val << std::endl;
    }
};

// full sepecialization of member template of fully sepcialized class Foo<bool>
template<>
inline void Foo<bool>::print<bool, bool>()
{
    std::cout << std::boolalpha << "Foo<bool>::print<bool, bool>: " << val << std::endl;
}

class Bar
{
public:
    template<typename U1, typename U2>
    void print()
    {
        std::cout << "Bar::print<U1, U2>" << std::endl;
    }
};
// fully specialization of member template of normal class
template<>
inline void Bar::print<bool, bool>()
{
    std::cout << "Bar::print<bool, bool>" << std::endl;
}

int main(int argc, char const *argv[])
{
    Foo<double> foo(10);
    foo.print<bool, bool>(); // Foo<double>::print<bool, bool>
    foo.print<bool, int>(); // Foo<double>::print<U1, U2>
    Foo<int> fooi;
    fooi = foo;
    fooi.print<bool, bool>(); // Foo<T>::print<U1, U2>

    Foo<bool> foob;
    foob = fooi;
    foob.print<int, bool>(); // Foo<bool>::print<U1, U2>
    foob.print<bool, bool>(); // Foo<bool>::print<bool, bool>
    // .template construct, mainly used in a template
    foob.template print<bool, bool>(); 
    (&foob)->template print<bool, bool>();

    Bar bar;
    bar.print<int, bool>(); // Bar::print<U1, U2>
    bar.print<bool, bool>(); // Bar::print<bool, bool>
    return 0;
}
