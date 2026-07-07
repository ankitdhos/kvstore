# In-Memory Key-Value Store (C++)

A lightweight, Redis-inspired key-value store built from scratch in C++, featuring a custom
hash table (no `std::unordered_map`), TTL-based key expiry via a min-heap, disk persistence,
and a TCP server for remote access.

## Features

- **Custom hash table** — FNV-1a hashing with separate chaining for collision resolution and
  automatic resizing when load factor exceeds 0.75
- **TTL / key expiry** — a min-heap tracks expiry times so expired keys are evicted efficiently
  without scanning the whole table
- **Persistence** — periodic snapshotting to disk (every 5s) and on graceful shutdown (SIGINT/SIGTERM),
  with full reload on restart
- **TCP server** — simple text protocol (`SET`, `GET`, `DEL`, `STATS`) over raw sockets

## Building

```bash
g++ -std=c++17 -O2 -Iinclude -o kvserver src/hashtable.cpp src/ttl_manager.cpp src/kvstore.cpp src/server.cpp -lpthread
```

## Running

```bash
./kvserver 6380
```

## Protocol

Connect via any TCP client (netcat, telnet, or a custom script) on the given port:

```
SET key value        -> OK
SET key value 10     -> OK   (expires in 10 seconds)
GET key               -> <value> | NOT_FOUND
DEL key               -> DELETED | NOT_FOUND
STATS                  -> size=... capacity=... load_factor=... collision_buckets=...
```

## Benchmark Results

Measured on a single client issuing sequential requests over a loopback TCP connection
(20,000 operations each for SET and GET):

| Metric | Result |
|---|---|
| SET throughput | ~64,000 ops/sec |
| GET throughput | ~67,000 ops/sec |
| Correctness | 20,000 / 20,000 GET hits (100%) |
| Load factor at 20K keys | 0.61 (32,768 buckets) |
| Collision rate | 12.49% of buckets had 2+ entries |

Note: throughput here includes full client-server round-trip over TCP (not just in-memory
hash table speed), so it reflects realistic network-bound performance rather than raw
data-structure benchmarks.

## Design Notes

- **Why chaining over open addressing:** simpler to reason about and avoids clustering issues;
  acceptable trade-off given the resizing strategy keeps load factor bounded.
- **Why a min-heap for TTL instead of scanning on every GET:** avoids O(n) work per read;
  expired keys are swept periodically in O(log n) per expiry via a background thread.
- **Why a background sweeper thread instead of checking on access:** keeps read/write paths
  fast and predictable; expiry cost is amortized in the background.
- **Concurrency:** a single mutex guards the hash table; the server currently handles one
  client connection at a time. Multi-client concurrency handling is a natural next step.

## Possible Extensions

- Multi-client concurrency (thread-per-connection or an event loop)
- LRU eviction policy alongside TTL
- Binary protocol instead of plain text for lower parsing overhead
