template<typename T>
void f1(T x)
{
    g1(x);
}

void g1(int)
{
}

int main(int argc, char const *argv[])
{
    f1(7); // ERROR: 'g1' was not declared in this scope, and no declarations were found by argument-dependent lookup at the point of instantiation
    return 0;
}