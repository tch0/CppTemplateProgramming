#include <iostream>
#include <string>
namespace impl
{

class BigNumber
{
    friend bool operator<(const BigNumber& lhs, const BigNumber& rhs)
    {
        return lhs.num < rhs.num;
    }
    friend std::ostream& operator<<(std::ostream& os, const BigNumber& number)
    {
        return os << number.num;
    }
private:
    std::string num;
public:
    BigNumber(const std::string& _num) : num(_num) {}
};

}

template<typename T>
T max(T a, T b)
{
    return a < b ? b : a;
}

using impl::BigNumber;

int main(int argc, char const *argv[])
{
    BigNumber a("100"), b("101");
    std::cout << ::max(a, b) << std::endl;
    return 0;
}
