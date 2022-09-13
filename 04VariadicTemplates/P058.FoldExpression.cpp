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
void print(Args... args)
{
    (std::cout << ... << AddSpace(args)) << std::endl; // (init op ... op pack)
}

int main(int argc, char const *argv[])
{
    print(1, 2, "hello", 3.0);
    return 0;
}
