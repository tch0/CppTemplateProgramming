#include <iostream>
#include <functional>
#include <string>
#include <type_traits>

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
void print1(const Args&... args)
{
    (std::cout << ... << AddSpace(args)) << std::endl; // (init op ... op pack)
}

// a better and more flexible way to add space by using fold expression with operator &&
template<typename... Args>
void print2(const Args&... args)
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
void print3(const Args&... args)
{
    (... , (std::cout << args << " ")) << std::endl; // do not even need a generic lambda
}

template<typename... Args>
void print4(const Args&... args)
{
    auto printSpace = [](std::ostream& os, const auto& val) -> bool {
        os << val << " ";
        return !std::is_same_v<std::decay_t<decltype(val)>, int>;
    };
    (... && printSpace(std::cout, args));
    std::cout << std::endl;
}

template<typename First, typename... Args>
void printFolder(std::ostream& os, const First &firstarg, const Args&... args)
{
    ((os << "(" << firstarg), ..., (os << ", " << args)) << ")";
}

int main(int argc, char const *argv[])
{
    print1(1, 2, "hello", 3.01);
    print2(1, 2, "hello", 3.01);
    print3(1, 2, "hello", 3.01);
    print4("hello", 3.1, 1, "world");
    printFolder(std::cout, 1, 2, "hello", 3.01);
    return 0;
}
