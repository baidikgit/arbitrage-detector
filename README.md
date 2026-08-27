# Multi-Exchange Arbitrage Detector

A C++ tool that pulls live crypto prices from Binance, models them as a
weighted graph, and checks whether a risk-free arbitrage loop exists,
i.e. a sequence of trades (like BTC → ETH → USDT → BTC) that nets you
more money than you started with, after fees and spread.

This was a student project built to practice graph algorithms
(specifically Bellman-Ford for negative cycle detection) on a problem
that actually has real-world stakes attached to it. It is **not** meant
to be used for actual trading: no order execution, no real money, no
guarantees the math holds up under real market conditions at scale because I cannot compete with HFTs in this regard. 

## The problem

If USD → BTC → ETH → USD ever nets you more USD than you started with,
that's arbitrage, a "free" profit loop. In efficient markets this
shouldn't exist for long (hft bots close these gaps in microseconds), but
short-lived mispricings can and do happen, especially across less
liquid pairs.

The hard part isn't spotting one mispriced pair, it's asking: **across
a whole basket of currencies, does *any* cycle exist that's profitable
once you account for fees and spread?** That's a graph problem, not a
"compare two prices" problem, with `N` currencies you'd have to check
an exponential number of possible paths by brute force. There's a much
faster way with graph algos.

## The approach

1. **Fetch live prices** from Binance's public API (`bookTicker`
   endpoint:- best bid/ask, not just last-traded price, since a stale
   last-trade price can create *fake* arbitrage that doesn't actually
   exist if you tried to trade it).
2. **Build a graph**: each currency is a node, each tradable pair is a
   directed edge. Edge weight = `-log(price * (1 - fee))`.
3. **Run Bellman-Ford** from a dummy source node connected to every
   currency with weight 0 (so a single run can catch a cycle anywhere
   in the graph, not just ones reachable from one arbitrary start
   node).
   The math trick: normally Bellman-Ford finds shortest paths. But if you
   take `-log(price)` as your edge weight, multiplying prices around a
   loop becomes *adding* weights around a loop (logs turn multiplication
   into addition). So a profitable loop (product of prices > 1)
   becomes a **negative-weight cycle** in the graph, exactly what
   Bellman-Ford is built to detect.
4. **Reconstruct the cycle** by walking the predecessor array back
   `V` times (guaranteed to land on the cycle by pigeonhole principle), then
   walking forward until it loops.
5. **Compute real profit**: `(e^(-sum of weights) - 1) * 100`, and only
   report cycles above a small threshold (default 0.05%) so tiny
   floating-point inaccuracy noise doesn't get reported as "arbitrage."

## LIMITATIONS (IMPORTANT)

**Does account for:**
- Trading fees (0.1% per hop, Binance's standard spot taker fee)
- Bid/ask spread (using bid to sell, ask to buy — not just one
  "last price" that might not reflect what you could actually get)

**Does NOT account for:**
- **Order book depth / slippage.** We use the best bid/ask only (top
  of book). A real trade of any real size would eat further into the
  book and get a worse average price, phenomena called slippage. This tool assumes you're trading
  a tiny, unrealistic size.
- **Execution speed / latency.** By the time you'd actually place three
  trades in sequence, the prices have likely moved. This is a snapshot
  in time, not a live execution guarantee.
- **Withdrawal/network fees, minimum trade sizes, or exchange-specific
  quirks.**

Basically: if this tool says "arbitrage found," treat it as "worth
investigating further," not "guaranteed free money." Most of the time,
on a healthy market, it'll correctly find nothing, which is the
expected, boring, realistic result.

## Usage

Build (Windows, CMake + vcpkg):

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

Run:

```powershell
.\Debug\detect.exe
```

Flags:

| Flag | Meaning | Default |
|---|---|---|
| `--fee 0.002` | Override trading fee (as a decimal, e.g. 0.002 = 0.2%) | 0.001 |
| `--live` | Poll repeatedly instead of running once | off |
| `--interval 10` | Seconds between polls, only used with `--live` | 10 |
| `--iterations 5` | How many times to poll before exiting, only used with `--live` | 5 |

Example:

```powershell
.\Debug\detect.exe --live --iterations 10 --interval 15 --fee 0.0015
```

## Why Bellman-Ford and not Dijkstra
Dijkstra's algorithm can't handle negative edge weights correctly —
and our edges are deliberately negative-weight-capable (that's the
whole point, since a negative cycle *is* the arbitrage signal).
Bellman-Ford is slower per-run (`O(V*E)` vs Dijkstra's `O(E log V)`)
but it's the right tool for this specific job. With ~10-15 currencies
and a few dozen pairs, `O(V*E)` is nowhere near a performance concern.

## Requirements

- C++17
- [cpr](https://github.com/libcpr/cpr) (HTTP requests)
- [nlohmann/json](https://github.com/nlohmann/json) (JSON parsing)
- Both installed via [vcpkg](https://github.com/microsoft/vcpkg)
