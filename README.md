# TickWire — Low Latency Networking Library (C++)

TickWire is a minimal, performance-focused networking system built in modern C++.

## Features

* Fixed-size packet buffer pool (no heap allocations in hot path)
* Lock-free Single Producer Single Consumer (SPSC) queue
* Non-blocking UDP socket abstraction (Win32)
* Multi-threaded pipeline (network → worker)
* Deterministic memory usage
* Metrics and observability

## Architecture

Socket → BufferPool → PacketBuffer → SPSC Queue → Worker → Release

## Benchmark

Processes millions of packets per second depending on hardware.

## Design Goals

* Low latency
* Predictable performance
* Explicit memory ownership
* Minimal abstraction overhead

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Future Work

* Linux (POSIX) support
* Packet prioritization
* Reliability layer over UDP
* GUI visualization
