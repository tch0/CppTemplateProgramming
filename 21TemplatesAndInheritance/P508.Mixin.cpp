#include <iostream>
#include <vector>
#include <concepts>
#include <concepts>

template<typename... Mixins>
class Point : public Mixins...
{
public:
    double x, y;
    Point(double _x = 0.0, double _y = 0.0) : x(_x), y(_y) {}
};

template<typename... Mixins>
class Polygon
{
private:
    std::vector<Point<Mixins...>> points;
public:
};

int main(int argc, char const *argv[])
{
    
    return 0;
}
