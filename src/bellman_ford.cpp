#include "bellman_ford.hpp"
#include <limits>
#include <cmath>
#include <algorithm>

BellmanFordResult runBellmanFord(const Graph &g, int sourceIdx)
{
    int V = g.currencies.size();
    BellmanFordResult res;
    res.dist.assign(V, std::numeric_limits<double>::infinity());
    res.pred.assign(V, -1);
    res.dist[sourceIdx] = 0.0;

    for (int i = 0; i < V - 1; i++)
    {
        for (int u = 0; u < V; u++)
        {
            if (res.dist[u] == std::numeric_limits<double>::infinity())
                continue;
            for (auto &e : g.adj[u])
            {
                if (res.dist[u] + e.weight < res.dist[e.to])
                {
                    res.dist[e.to] = res.dist[u] + e.weight;
                    res.pred[e.to] = u;
                }
            }
        }
    }

    // one more pass: anything that still relaxes is on/reachable from a negative cycle
    for (int u = 0; u < V; u++)
    {
        if (res.dist[u] == std::numeric_limits<double>::infinity())
            continue;
        for (auto &e : g.adj[u])
        {
            if (res.dist[u] + e.weight < res.dist[e.to])
            {
                res.cycleNode = e.to;
                break;
            }
        }
        if (res.cycleNode != -1)
            break;
    }

    return res;
}

ArbitrageCycle extractCycle(const Graph &g, const BellmanFordResult &res)
{
    int V = g.currencies.size();
    int x = res.cycleNode;

    // pigeonhole: walking pred V times guarantees landing ON the cycle
    for (int i = 0; i < V; i++)
        x = res.pred[x];

    // walk forward via pred until we loop back to x
    std::vector<int> cycleNodes;
    int cur = x;
    double weightSum = 0.0;
    do
    {
        cycleNodes.push_back(cur);
        int prevNode = res.pred[cur];
        // find the edge prevNode->cur to get its weight
        for (auto &e : g.adj[prevNode])
        {
            if (e.to == cur)
            {
                weightSum += e.weight;
                break;
            }
        }
        cur = prevNode;
    } while (cur != x);

    cycleNodes.push_back(x); // close the loop
    std::reverse(cycleNodes.begin(), cycleNodes.end());

    ArbitrageCycle result;
    for (int idx : cycleNodes)
        result.path.push_back(g.currencies[idx]);
    result.profitPct = (std::exp(-weightSum) - 1.0) * 100.0;
    return result;
}