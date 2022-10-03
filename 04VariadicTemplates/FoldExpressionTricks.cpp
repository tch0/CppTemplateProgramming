#include <iostream>
#include <string>

using namespace std::literals;

// summation
auto sum(auto&& head, auto&&... tail)
{
    return (head + ... + tail);
}

// apply function for every element
void applyFor(auto&& f, auto&&... args)
{
    (..., f(args));
}

// apply function for every element in reverse order
void applyForReverse(auto&& f, auto&&... args)
{
    int dummy;
    (dummy = ... = (f(args), 0)); // UB?
}

// apply function for every element until match predicate
void applyForUntil(auto&& f, auto&& pred, auto&&... args)
{
    (... && (pred(args) ? false : (f(args), true)));
}

// check whether any elements matches predicate
bool anyOf(auto&& pred, auto&&... args)
{
    return (... || pred(args));
}

// checke whether all elements matches predicate
bool allOf(auto&& pred, auto&&... args)
{
    return (... && pred(args));
}

// check whether no element matches predicate
bool noneOf(auto&& pred, auto&&... args)
{
    return !(... || pred(args));
}

// count how many elements match the predicate
std::size_t countIf(auto&& pred, auto&&... args)
{
    return (std::size_t(0) + ... + (pred(args) ? 1 : 0));
}

// find the first element that matches the element
auto findFirst(auto&& pred, auto&&... args)
{
    std::common_type_t<std::decay_t<decltype(args)>...> result{};
    (... || (pred(args) ? (result = args, true) : false));
    return result;
}

// get n-th element
auto nthElement(std::size_t n, auto&&... args)
{
    std::common_type_t<std::decay_t<decltype(args)>...> result{};
    std::size_t i = 0;
    (... || (i++ == n ? (result = args, true) : false));
    return result;
}

// get the first element of parameter pack
auto firstElement(auto&&... args)
{
    std::common_type_t<std::decay_t<decltype(args)>...> result{};
    (... || (result = args, true));
    return result;
}

// get the last element of parameter pack
auto lastElement(auto&&... args)
{
    return (... , args);
}

// get the minimal element
auto minElement(auto&&... args)
{
    auto min = (args, ...); // last element
    return (... , (args < min ? (min = args, min) : min));
}

// test
int main(int argc, char const *argv[])
{
    // sum
    std::cout << sum("hello"s, "world") << std::endl;
    // applyFor
    auto print = [](auto val)->void { std::cout << val << " "; };
    applyFor(print, 1, 2.1, "hello"s);
    // applyForReverse
    std::cout << std::endl;
    applyForReverse(print, 1, 2.1, "hello");
    // applyForUntil
    std::cout << std::endl;
    auto lenLessThan4 = [](auto val) -> bool {
        return std::to_string(val).length() < 4;
    };
    applyForUntil(print, lenLessThan4, 10001, 2.1244, 100.001, 1.0);
    // anyOf
    std::cout << std::endl << std::boolalpha;
    std::cout << anyOf(lenLessThan4, 100, 1.01, 3.1) << std::endl;
    // allOf
    std::cout << allOf(lenLessThan4, 100, 1.01, 3.1) << std::endl;
    // noneOf
    std::cout << noneOf(lenLessThan4, 100, 1.01, 3.1) << std::endl;
    // countIf
    std::cout << countIf(lenLessThan4, 100, 1.01, 3.1, 10) << std::endl;
    // findFirst
    std::cout << findFirst(lenLessThan4, 100, 1.01, 3.1, 10) << std::endl;
    // nthElement
    std::cout << nthElement(3, 10, 1.01, 2.01, 100.001, 88.00) << std::endl;
    // firstElement
    std::cout << firstElement(10, 1.01, 2.01, 100.001, 88.00) << std::endl;
    // lastElement
    std::cout << lastElement(10, 1.01, 2.01, 100.001, 88.00) << std::endl;
    // minElement
    std::cout << minElement(10, 1.01, 2.01, 100.001, 88.00) << std::endl;
    return 0;
}
