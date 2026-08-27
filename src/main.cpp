#include "exchange_api.hpp"
#include "graph.hpp"
#include "bellman_ford.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void runOnce(const PriceMap &prices, double fee)
{
    Graph g = buildGraph(prices, fee);
    int sourceIdx = g.currencyIndex["__SOURCE__"];
    BellmanFordResult res = runBellmanFord(g, sourceIdx);

    if (res.cycleNode == -1)
    {
        std::cout << "No arbitrage found\n";
        return;
    }

    ArbitrageCycle cyc = extractCycle(g, res);
    if (cyc.profitPct <= 0.05)
    {
        std::cout << "Below threshold (" << cyc.profitPct << "%), skipping\n";
        return;
    }

    std::cout << "\033[32mARBITRAGE: "; // green colour code in powershell/terminals
    for (auto &c : cyc.path)
        std::cout << c << " -> ";
    std::cout << "\nProfit: " << cyc.profitPct << "%\033[0m\n";
}

int main(int argc, char *argv[])
{
    bool live = false;
    int interval = 2;
    int maxIterations = 5; //--live does max iterations instead of running forever
    double fee = 0.001;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--live")
            live = true;
        else if (arg == "--iter" && i + 1 < argc)
            maxIterations = std::stoi(argv[++i]);
        else if (arg == "--interval" && i + 1 < argc)
            interval = std::stoi(argv[++i]);
        else if (arg == "--fee" && i + 1 < argc)
            fee = std::stod(argv[++i]);
    }

    if (!live)
    {
        PriceMap prices = fetchBinancePrices();
        if (prices.empty())
            prices = generateSyntheticPrices();
        runOnce(prices, fee);
        return 0;
    }

    for (int iter = 0; iter < maxIterations; iter++)
    {
        std::cout << "\n--- Iteration " << iter + 1 << "/" << maxIterations << " ---\n";
        PriceMap prices = fetchBinancePrices();
        if (prices.empty())
            prices = generateSyntheticPrices();
        runOnce(prices, fee);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    return 0;
}