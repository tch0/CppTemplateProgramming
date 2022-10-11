#include <type_traits>
#include <iostream>
#include <memory>

template<typename CountedType>
class ObjectCounter
{
private:
    inline static std::size_t count = 0;
protected:
    ObjectCounter() { ++count; }
    ObjectCounter(const ObjectCounter&) { ++count; }
    ObjectCounter(ObjectCounter&&) { ++count; }
    ~ObjectCounter() { --count; }
public:
    static std::size_t live() { return count; }
};

class A : public ObjectCounter<A> {};


int main(int argc, char const *argv[])
{
    {
        std::shared_ptr<A> spa = std::make_shared<A>();
        A a;
        std::cout << A::live() << std::endl;
    }
    std::cout << A::live() << std::endl;
    return 0;
}
