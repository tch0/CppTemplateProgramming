#include <iostream>

void f(int, int);
template<typename T1, typename T2>
void f(T1, T2);

class Mixer
{
    friend void f(int, int); // the ordinary one
    friend void f<>(int, int); // the template one
    friend void f<int&, int&>(int&, int&);
    friend void f<char>(char, int);
    // friend void f<>(long, long) // ERROR: can not specilize in friend declaration
    // {
    // }
};


int main(int argc, char const *argv[])
{
    
    return 0;
}
