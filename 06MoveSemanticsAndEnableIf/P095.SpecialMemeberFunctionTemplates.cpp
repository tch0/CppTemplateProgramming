#include <iostream>
#include <string>
#include <type_traits>

class Person
{
private:
    std::string name;
public:
    Person(const std::string& _name) : name(_name) // 1
    {
        std::cout << "Person(const std::string& _name)" << std::endl;
    }
    Person(std::string&& _name) : name(std::move(_name)) // 2
    {
        std::cout << "Person(std::string&& _name)" << std::endl;
    }
    Person(const Person& other) : name(other.name) // 3
    {
        std::cout << "Person(const Person& other)" << std::endl;
    }
    Person(Person&& other) : name(std::move(other.name)) // 4
    {
        std::cout << "Person(Person&& other)" << std::endl;
    }
};

// make it generic
class Person2
{
private:
    std::string name;
public:
    // SFINAE
    template<typename T,
        typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    explicit Person2(T&& _name) : name(std::forward<T>(_name)) // 1
    {
        std::cout << "template<typename T> Person2(T&& _name)" << std::endl;
    }
    Person2(const Person2& other) : name(other.name) // 2
    {
        std::cout << "Person2(const Person2& other)" << std::endl;
    }
    Person2(Person2&& other) : name(std::move(other.name)) // 3
    {
        std::cout << "Person2(Person2&& other)" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    std::string name = "kim";
    Person p1(name);
    Person p2("kim");
    Person p3(p2);
    Person p4(std::move(p2));

    Person2 p21(name);
    Person2 p22("kim");
    Person2 p23(p22);
    Person2 p24(std::move(p22));
    return 0;
}
