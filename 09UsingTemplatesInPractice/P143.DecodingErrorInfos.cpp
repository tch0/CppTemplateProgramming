#include <iostream>
#include <map>
#include <algorithm>
#include <string>

int main(int argc, char const *argv[])
{
    std::map<std::string, double> coll;
    auto f = [](const std::string& str) { return str != ""; }; // note that parameter should be std::pair<std::string, double>, for the erro information
    auto pos = find_if(coll.begin(), coll.end(), f);
    return 0;
}
