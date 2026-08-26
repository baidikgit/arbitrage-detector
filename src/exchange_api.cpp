#include "exchange_api.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <set>

using json = nlohmann::json;

static const std::set<std::string> WATCH_CURRENCIES = {
    "USDT", "BTC", "ETH", "BNB", "SOL", "XRP", "ADA", "DOGE", "EUR", "GBP"};

PriceMap fetchBinancePrices()
{
    PriceMap prices;

    // base/quote mapping
    cpr::Response infoResp = cpr::Get(cpr::Url{"https://api.binance.com/api/v3/exchangeInfo"});
    if (infoResp.status_code != 200)
    {
        std::cerr << "exchangeInfo failed, status: " << infoResp.status_code << "\n";
        return prices;
    }
    json info = json::parse(infoResp.text);

    std::map<std::string, std::pair<std::string, std::string>> symbolMap;
    for (auto &s : info["symbols"])
    {
        std::string base = s["baseAsset"];
        std::string quote = s["quoteAsset"];
        if (WATCH_CURRENCIES.count(base) && WATCH_CURRENCIES.count(quote))
        {
            symbolMap[s["symbol"]] = {base, quote};
        }
    }

    // live prices
    cpr::Response priceResp = cpr::Get(cpr::Url{"https://api.binance.com/api/v3/ticker/price"});
    if (priceResp.status_code != 200)
    {
        std::cerr << "ticker/price failed, status: " << priceResp.status_code << "\n";
        return prices;
    }
    json data = json::parse(priceResp.text);

    for (auto &entry : data)
    {
        std::string symbol = entry["symbol"];
        if (!symbolMap.count(symbol))
            continue; // not in our curated set

        double price = std::stod(entry["price"].get<std::string>());
        if (price <= 0)
            continue; // skip dead/delisted pairs

        auto [base, quote] = symbolMap[symbol];
        prices[base + "_" + quote] = price;
        prices[quote + "_" + base] = 1.0 / price;
    }

    std::cout << "Fetched " << prices.size() << " price entries from Binance\n";
    return prices;
}

PriceMap generateSyntheticPrices()
{
    PriceMap prices;
    prices["USD_BTC"] = 1.0 / 64000.0;
    prices["BTC_USD"] = 64000.0;
    prices["USD_ETH"] = 1.0 / 3400.0;
    prices["ETH_USD"] = 3400.0;
    prices["BTC_ETH"] = 18.9; // arbitrage
    prices["ETH_BTC"] = 1.0 / 18.5;
    prices["USD_EUR"] = 0.92;
    prices["EUR_USD"] = 1.0 / 0.92;

    std::cout << "Generated " << prices.size() << " synthetic price entries\n";
    return prices;
}

void saveSnapshot(const PriceMap &prices, const std::string &filename)
{
    json j(prices);
    std::ofstream file(filename);
    file << j.dump(2);
    std::cout << "Saved snapshot to " << filename << "\n";
}

PriceMap loadSnapshot(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "No snapshot found at " << filename << "\n";
        return {};
    }
    json j;
    file >> j;
    return j.get<PriceMap>();
}