#include <iostream>
#include <type_traits>

template<typename Derived, typename Value, typename Category, typename Reference = Value&, typename Distance = std::ptrdiff_t>
class IteratorFacade
{
public:
    using value_type = std::remove_reference_t<Value>;
    using reference = Reference;
    using pointer = Value*;
    using difference_type = Distance;
    using iterator_category = Category;
    // input iterator interface
    reference operator*() const;
    pointer operator->() const;
    Derived& operator++();
    Derived operator++(int);
    ...
    // bidirectional iterator interface
    Derived& operator--();
    Derived operator--(int);
    // random access iterator interface
    reference operator[](difference_type n) const;
    Derived& operator+=(difference_type n);
    ...
    friend bool operator==(IteratorFacade& lhs, IteratorFacade& rhs);
    friend difference_type operator-(const IteratorFacade& lhs, const IteratorFacade& rhs);
    friend bool operator<(const IteratorFacade& lhs, const IteratorFacade& rhs);
};

int main(int argc, char const *argv[])
{
    
    return 0;
}
