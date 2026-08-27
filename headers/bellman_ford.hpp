#pragma once
#include "graph.hpp"
#include <vector>
#include <limits>

struct BellmanFordResult
{
    std::vector<double> dist;
    std::vector<int> pred;
    int cycleNode = -1;
};

struct ArbitrageCycle
{
    std::vector<std::string> path; // currency names, in order, first == last
    double profitPct;
};

BellmanFordResult runBellmanFord(const Graph &g, int sourceIdx);
ArbitrageCycle extractCycle(const Graph &g, const BellmanFordResult &res);
