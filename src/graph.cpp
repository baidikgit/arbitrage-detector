#include "graph.hpp"
#include <cmath>
#include <iostream>

int Graph::addCurrency(const std::string &name)
{
    auto it = currencyIndex.find(name);
    if (it != currencyIndex.end())
        return it->second;

    int idx = currencies.size();
    currencies.push_back(name);
    currencyIndex[name] = idx;
    adj.push_back({});
    return idx;
}

void Graph::addEdge(const std::string &from, const std::string &to, double price, double fee)
{
    int u = addCurrency(from);
    int v = addCurrency(to);

    // weight = -log(price * (1 - fee))
    // negative log so that Bellman-Ford's "shortest path" logic works
    double weight = -std::log(price * (1.0 - fee));

    adj[u].push_back({v, weight, from + "_" + to});
}

Graph buildGraph(const PriceMap &prices, double fee)
{
    Graph g;

    for (auto &[key, price] : prices)
    {
        // key is like "BTC_USDT"
        size_t sep = key.find('_');
        std::string from = key.substr(0, sep);
        std::string to = key.substr(sep + 1);

        g.addEdge(from, to, price, fee);
    }

    // dummy source node, connected to every currency with weight 0

    int sourceIdx = g.addCurrency("__SOURCE__");
    for (int i = 0; i < sourceIdx; i++)
    {
        g.adj[sourceIdx].push_back({i, 0.0, "SOURCE_" + g.currencies[i]});
    }

    std::cout << "Graph built: " << g.currencies.size() << " nodes\n";
    return g;
}