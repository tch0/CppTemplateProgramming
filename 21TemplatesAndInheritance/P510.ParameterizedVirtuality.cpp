#include <iostream>
#include <memory>

class NonVirtual {};
class Virtual {
public:
    virtual void foo()
    {
        std::cout << "Virtual::foo()" << std::endl;
    }
};

template<typename... Mixins>
class Base : public Mixins...
{
public:
    void foo()
    {
        std::cout << "Base::foo()" << std::endl;
    }
};

template<typename... Mixins>
class Derived : public Base<Mixins...>
{
public:
    void foo()
    {
        std::cout << "Derived::foo()" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    std::shared_ptr<Base<NonVirtual>> p1 = std::make_shared<Derived<NonVirtual>>();
    p1->foo(); // Base::foo()
    std::shared_ptr<Base<Virtual>> p2 = std::make_shared<Derived<Virtual>>();
    p2->foo(); // Derived::foo()
    return 0;
}
