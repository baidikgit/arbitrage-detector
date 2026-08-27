#pragma once
#include <string>
#include <vector>
#include <map>
#include "types.hpp"

struct Edge
{
    int to;
    double weight;
    std::string pairLabel; // "BTC_USDT" for path recon later
};

struct Graph
{
    std::vector<std::string> currencies;      // index to name
    std::map<std::string, int> currencyIndex; // name to index
    std::vector<std::vector<Edge>> adj;       // adjacency list

    int addCurrency(const std::string &name);
    void addEdge(const std::string &from, const std::string &to, double price, double fee);
};

Graph buildGraph(const PriceMap &prices, double fee);