#include <iostream>
#include <functional>
#include <string>

template<typename T>
class AddSpace
{
    friend std::ostream& operator<<(std::ostream& os, AddSpace<T> s)
    {
        return os << s.ref << " ";
    }
private:
    T const& ref;
public:
    AddSpace(T const& r) : ref(r) {}
};

template<typename... Args>
void print1(Args... args)
{
    (std::cout << ... << AddSpace(args)) << std::endl; // (init op ... op pack)
}

// a better and more flexible way to add space by using fold expression with operator &&
template<typename... Args>
void print2(Args... args)
{
    auto printSpace = [](std::ostream& os, const auto& val) -> bool {
        os << val << " ";
        return true;
    };
    (... && printSpace(std::cout, args));
    std::cout << std::endl;
}

// comma operator, no overhead comapred to &&
template<typename... Args>
void print3(Args... args)
{
    auto printSpace = [](std::ostream& os, const auto& val) -> void {
        os << val << " ";
    };
    (... , printSpace(std::cout, args));
    std::cout << std::endl;
}

int main(int argc, char const *argv[])
{
    print1(1, 2, "hello", 3.01);
    print2(1, 2, "hello", 3.01);
    print3(1, 2, "hello", 3.01);
    return 0;
}
