#pragma once
#include "types.hpp"

// fetches live prices from Binance returns empty map on failure
PriceMap fetchBinancePrices();

// fallbakc so that demo is always there
PriceMap generateSyntheticPrices();

// save/load to avoid hammering the API while debugging
void saveSnapshot(const PriceMap &prices, const std::string &filename);
PriceMap loadSnapshot(const std::string &filename);