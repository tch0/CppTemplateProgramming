#include <iostream>
#include <concepts>
#include <exception>

template<typename R, typename... Args>
class FunctionBridge
{
public:
    virtual ~FunctionBridge() {}
    virtual FunctionBridge* clone() const = 0;
    virtual R invoke(Args... args) const = 0;
    virtual bool equals(const FunctionBridge* fb) const = 0;
};

// exception of no equality comparison operator
class NotEuqalityComparable : public std::exception {};

// tryto compare Functor with SFINAE, how to change to requires syntax? is there a way to do that?
template<typename T, bool = std::equality_comparable<T>>
struct TryEquals
{
    static bool equals(const T& x1, const T& x2)
    {
        return x1 == x2;
    }
};

template<typename T>
struct TryEquals<T, false>
{
    static bool equals(const T& x1, const T& x2)
    {
        throw NotEuqalityComparable();
    }
};


// specific bridge of FunctionBridge
template<typename Functor, typename R, typename... Args>
class SpecificFunctionBridge : public FunctionBridge<R, Args...>
{
private:
    Functor functor;
public:
    template<typename FunctorFwd>
    SpecificFunctionBridge(FunctorFwd&& _functor) : functor(std::forward<FunctorFwd>(_functor)) {}
    virtual SpecificFunctionBridge* clone() const override
    {
        return new SpecificFunctionBridge(functor);
    }
    virtual R invoke(Args... args) const override
    {
        return functor(std::forward<Args>(args)...);
    }
    virtual bool equals(const FunctionBridge<R, Args...>* fb) const override
    {
        if (auto specFb = dynamic_cast<const SpecificFunctionBridge*>(fb))
        {
            return TryEquals<Functor>::equals(functor, specFb->functor);
        }
        return false; // functor with different types are never equal
    }
};

template<typename Signature>
class FunctionPtr;

template<typename R, typename... Args>
class FunctionPtr<R(Args...)>
{
private:
    FunctionBridge<R, Args...>* bridge;
public:
    // constructors
    FunctionPtr() : bridge(nullptr) {}
    FunctionPtr(const FunctionPtr& other)
    {
        if (other.bridge)
        {
            bridge = other.bridge->clone();
        }
    }
    FunctionPtr(FunctionPtr& other) : FunctionPtr(static_cast<const FunctionPtr&>(other)) {}
    FunctionPtr(FunctionPtr&& other) : bridge(other.bridge)
    {
        other.bridge = nullptr;
    }
    template<typename F>
    FunctionPtr(F&& f) : bridge(nullptr)
    {
        using Functor = std::decay_t<F>;
        using Bridge = SpecificFunctionBridge<Functor, R, Args...>;
        bridge = new Bridge(std::forward<F>(f));
    }
    // assignment
    FunctionPtr& operator=(const FunctionPtr& other)
    {
        FunctionPtr tmp(other);
        swap(*this, tmp);
        return *this;
    }
    FunctionPtr& operator=(FunctionPtr&& other)
    {
        delete bridge;
        bridge = other.bridge;
        other.bridge = nullptr;
        return *this;
    }
    template<typename F>
    FunctionPtr& operator=(F&& f)
    {
        FunctionPtr tmp(std::forward<F>(f));
        swap(*this, tmp);
        return *this;
    }
    // destructor
    ~FunctionPtr()
    {
        delete bridge;
    }
    friend void swap(FunctionPtr& fp1, FunctionPtr& fp2)
    {
        std::swap(fp1.bridge, fp2.bridge);
    }
    explicit operator bool() const
    {
        return bridge != nullptr;
    }
    R operator()(Args... args) const
    {
        return bridge->invoke(std::forward<Args>(args)...);
    }
    friend bool operator==(const FunctionPtr& f1, const FunctionPtr& f2)
    {
        if (!f1 || !f2)
        {
            return !f1 && !f2; // both are empty
        }
        return f1.bridge->equals(f2.bridge);
    }
    friend bool operator!=(const FunctionPtr& f1, const FunctionPtr& f2)
    {
        return !(f1 == f2);
    }
};

void foo()
{
    std::cout << "foo()" << std::endl;
}

class A
{
public:
    bool operator==(const A&) const
    {
        return true;
    }
    void operator()() const
    {
        std::cout << "A::operator()()" << std::endl;
    }
    static void foo()
    {
        std::cout << "A::foo()" << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    auto bar = []() -> void {
        std::cout << "bar()" << std::endl;
    };
    auto f1 = FunctionPtr<void()>(foo);
    f1();
    auto f2 = FunctionPtr<void()>(A::foo);
    f2();
    auto f3 = FunctionPtr<void()>(A());
    f3();
    auto f4 = FunctionPtr<void()>(bar);
    f4();
    std::cout << std::boolalpha;
    std::cout << (f1 == f2) << std::endl;
    std::cout << (f3 == f4) << std::endl;
    std::cout << (f3 == FunctionPtr<void()>(A())) << std::endl;
    return 0;
}
