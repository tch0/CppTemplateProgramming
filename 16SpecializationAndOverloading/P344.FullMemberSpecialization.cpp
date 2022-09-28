#include <iostream>
#include <string>

template<typename T>
class X
{
public:
    // member function template
    template<typename U>
    void tf()
    {
        std::cout << "X<T>::tf<U>()" << std::endl;
    }
    // static member function template
    template<typename U>
    static void stf()
    {
        std::cout << "X<T>::stf<U>()" << std::endl;
    }
    // static data member template
    template<typename U>
    inline static std::string ststr = "X<T>::ststr<U>()";
    
    // ordinary member function
    void f()
    {
        std::cout << "X<T>::f()" << std::endl;
    }
    // ordinary static data member
    static std::string sstr; // = "X<T>::sstr";
    // ordinary static member function
    static void sf()
    {
        std::cout << "X<T>::sf()" << std::endl;
    }
};
template<typename T>
inline std::string X<T>::sstr = "X<T>::sstr"; // defining as inline inside class will cause reinitialization


// full specialization of member template
template<>
template<>
void X<int>::tf<int>()
{
    std::cout << "X<int>::tf<int>()" << std::endl;
}
template<>
template<>
void X<int>::stf<int>()
{
    std::cout << "X<int>::stf<int>()" << std::endl;
}
template<>
template<>
inline std::string X<int>::ststr<int> = "X<int>::ststr<int>";

// full specialization of ordinary member
template<>
void X<int>::f()
{
    std::cout << "X<int>::f()" << std::endl;
}
template<>
inline std::string X<int>::sstr = "X<int>::sstr";
template<>
void X<int>::sf()
{
    std::cout << "X<int>::sf()" << std::endl;
}

// specialization of member template of ordinary class
class Y
{
public:
    // member function template
    template<typename U>
    void tf()
    {
        std::cout << "Y::tf<U>()" << std::endl;
    }
    // static member function template
    template<typename U>
    static void stf()
    {
        std::cout << "Y::stf<U>()" << std::endl;
    }
    // static data member template
    template<typename U>
    inline static std::string ststr = "Y::ststr<U>()";
};

// full specialization of member template of ordinary class
template<>
void Y::tf<int>()
{
    std::cout << "Y::tf<int>()" << std::endl;
}
template<>
void Y::stf<int>()
{
    std::cout << "Y::stf<int>()" << std::endl;
}
template<>
inline std::string Y::ststr<int> = "Y::ststr<int>";

int main(int argc, char const *argv[])
{
    // generic version
    X<bool>().tf<bool>();
    X<bool>::stf<bool>();
    std::cout << X<bool>::ststr<bool> << std::endl;
    X<bool>().f();
    X<bool>::sf();
    std::cout << X<bool>::sstr << std::endl;

    // full specialization version
    X<int>().tf<int>();
    X<int>::stf<int>();
    std::cout << X<int>::ststr<int> << std::endl;
    X<int>().f();
    X<int>::sf();
    std::cout << X<int>::sstr << std::endl;

    std::cout << std::endl;
    // ordinary class
    // generic version
    Y().tf<bool>();
    Y::stf<bool>();
    std::cout << Y::ststr<bool> << std::endl;
    // full sepcialization version
    Y().tf<int>();
    Y::stf<int>();
    std::cout << Y::ststr<int> << std::endl;
    return 0;
}
