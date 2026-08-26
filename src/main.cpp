#include "exchange_api.hpp"
#include <iostream>

int main()
{
    PriceMap prices = fetchBinancePrices();

    if (prices.empty())
    {
        std::cout << "Falling back to synthetic data\n";
        prices = generateSyntheticPrices();
    }

    saveSnapshot(prices, "snapshot.json");
    int count = 0;
    for (auto &[key, price] : prices)
    {
        std::cout << key << " = " << price << "\n";
        if (++count >= 10)
            break;
    }

    return 0;
}