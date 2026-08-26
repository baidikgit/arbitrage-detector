#pragma once
#include <string>
#include <map>

struct PricePoint
{
    std::string base;
    std::string quote;
    double price;
};

using PriceMap = std::map<std::string, double>;