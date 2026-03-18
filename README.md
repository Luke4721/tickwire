# TickWire

> **A zero-allocation, lock-free UDP networking library for low-latency systems — written in modern C++20**

[![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)]()
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

TickWire is a production-minded, low-latency UDP networking library built from first principles in C++20. It is designed for systems where **every microsecond counts** — financial trading engines, game servers, real-time telemetry, and high-frequency sensor pipelines. The library enforces zero heap allocations on the hot path, explicit memory ownership, and deterministic behaviour under load.

---

## Table of Contents

- [Motivation](#motivation)
- [Key Design Decisions](#key-design-decisions)
- [Architecture](#architecture)
- [Data Flow Diagram](#data-flow-diagram)
- [Project Structure](#project-structure)
- [Performance](#performance)
- [Build Instructions](#build-instructions)
- [Running the Tests](#running-the-tests)
- [Possible Applications](#possible-applications)
- [Roadmap](#roadmap)

---

## Motivation

Most general-purpose networking libraries (Boost.Asio, libuv, etc.) make design decisions that are incompatible with hard real-time requirements: they allocate on the heap per packet, hide latency behind abstraction layers, and rely on the OS scheduler for correctness. TickWire is an experiment in building a networking foundation that makes none of those compromises.

The specific constraints that shaped every decision in this library:

- **No heap allocations on the hot path** — all buffers are pre-allocated at startup in a fixed-size pool
- **No locks on the hot path** — the producer/consumer handoff uses a lock-free SPSC queue
- **No hidden copies** — ownership is explicit and tracked; buffers move through the pipeline and return to the pool
- **Deterministic latency** — the worst-case time for a packet to move from socket to worker is bounded and predictable

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| **Fixed-size `BufferPool`** | Pre-allocates all packet buffers at startup. `acquire()` and `release()` are O(1) pointer operations. No `malloc`/`free` after initialization. |
| **Lock-free `SPSCQueue`** | Single Producer Single Consumer queue using `std::atomic` with `acquire`/`release` memory ordering. No mutex, no condition variable, no kernel transition on the hot path. |
| **Cache-line-aligned atomics** | `head_` and `tail_` of the SPSC queue are separated onto distinct 64-byte cache lines via `alignas(64)`. Prevents false sharing between the receiver core and the worker core. |
| **`TickWireAddress` value type** | Trivially copyable, fixed-size (20 bytes) address abstraction. No dynamic allocation, safe to `memcpy`, fits in a register pair. |
| **`sockaddr_storage` on the stack** | The OS address buffer lives on the stack inside `poll()`/`drain()`. Zero heap involvement per receive call. |
| **`[[nodiscard]]` on address conversion** | `from_sockaddr` and `to_sockaddr` return `bool`. Ignoring the return value silently discards parse failures — `[[nodiscard]]` makes that a compiler warning. |
| **`noexcept` on the hot path** | `poll()`, `drain()`, `push()`, `pop()`, `acquire()`, `release()` are all `noexcept`. No exception-handling scaffolding is generated. No unwinding on error — just early returns. |

---

## Architecture

TickWire is built as a pipeline of five independent components. Each component has a single, well-defined responsibility and communicates with the next via explicit ownership transfer.

```
┌─────────────────────────────────────────────────────────────────────┐
│                         TickWire Pipeline                           │
│                                                                     │
│  ┌──────────┐    ┌────────────┐    ┌──────────────┐                │
│  │          │    │            │    │              │                 │
│  │  Socket  │───▶│ BufferPool │───▶│ PacketBuffer │                │
│  │ (UDP/OS) │    │  acquire() │    │  (metadata + │                │
│  │          │    │            │    │   payload)   │                │
│  └──────────┘    └────────────┘    └──────┬───────┘                │
│                                           │                         │
│                                           ▼                         │
│                                   ┌──────────────┐                 │
│                                   │  SPSCQueue   │                 │
│                                   │  lock-free   │                 │
│                                   │  push/pop    │                 │
│                                   └──────┬───────┘                 │
│                                          │                          │
│                                          ▼                          │
│                                   ┌──────────────┐                 │
│                                   │    Worker    │                 │
│                                   │  process()   │                 │
│                                   └──────┬───────┘                 │
│                                          │                          │
│                                          ▼                          │
│                                   ┌──────────────┐                 │
│                                   │  BufferPool  │                 │
│                                   │  release()   │                 │
│                                   └──────────────┘                 │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Data Flow Diagram

```
                         NETWORK RECEIVER THREAD
  ═══════════════════════════════════════════════════════════════

  UDP Packet                  ┌──────────────────────────────┐
  arrives on wire             │      NetworkReceiver         │
        │                     │                              │
        ▼                     │  drain(max_packets)          │
  ┌───────────┐               │       │                      │
  │  Socket   │  recvfrom()   │       ▼                      │
  │  (OS)     │──────────────▶│  socket_.receive()           │
  └───────────┘               │       │                      │
                              │       ▼ (bytes + sockaddr)   │
                              │  from_sockaddr()             │
                              │       │                      │
                              │       ▼ (TickWireAddress)    │
                              │  pool_.acquire()             │
                              │       │                      │
                              │       ▼ (PacketBuffer*)      │
                              │  memcpy payload              │
                              │  write metadata              │
                              │  write timestamp_ns          │
                              │       │                      │
                              │       ▼                      │
                              │  queue_.push()               │
                              │       │                      │
                              └───────┼──────────────────────┘
                                      │
                             PacketBuffer* handed off
                             across thread boundary
                                      │
  ═══════════════════════════════════╪═══════════════════════════
                         WORKER THREAD
  ═══════════════════════════════════╪═══════════════════════════
                                      │
                              ┌───────▼──────────────────────┐
                              │         Worker               │
                              │                              │
                              │  queue_.pop()                │
                              │       │                      │
                              │       ▼ (PacketBuffer*)      │
                              │  read metadata               │
                              │  read payload                │
                              │  application logic           │
                              │       │                      │
                              │       ▼                      │
                              │  pool_.release()             │
                              │  (buffer back to pool)       │
                              └──────────────────────────────┘


  MEMORY LIFECYCLE
  ────────────────────────────────────────────────────────────────
  Startup         BufferPool pre-allocates N PacketBuffers
                       │
  Hot path:       acquire() ──▶ fill ──▶ push ──▶ pop ──▶ release()
                       │                                        │
                       └────────────────────────────────────────┘
                                   zero heap allocations
```

---

## Project Structure

```
tickwire/
│
├── include/
│   └── tickwire/
│       ├── address.h          # TickWireAddress value type + from/to_sockaddr
│       ├── buffer_pool.h      # Fixed-size pre-allocated packet buffer pool
│       ├── metrics.h          # Atomic counters for observability
│       ├── network_receiver.h # poll() / drain() — hot receive path
│       ├── packet.h           # PacketMetadata struct
│       ├── packet_buffer.h    # PacketBuffer (metadata + payload array)
│       ├── socket.hpp         # Platform-agnostic socket abstraction
│       ├── spsc_queue.h       # Lock-free single-producer single-consumer queue
│       └── worker.h           # Consumer-side packet processor
│
├── src/
│   ├── address.cpp            # from_sockaddr / to_sockaddr implementation
│   ├── buffer_pool.cpp        # BufferPool acquire / release
│   ├── network_receiver.cpp   # poll() and drain() implementations
│   ├── socket.cpp             # Portable socket RAII (move semantics)
│   ├── worker.cpp             # Worker process() implementation
│   ├── main.cpp               # Entry point — wires all components together
│   ├── benchmark.cpp          # Standalone pool + queue throughput benchmark
│   │
│   └── platform/
│       ├── socket_win32.cpp   # Winsock2 implementation (Windows)
│       └── socket_posix.cpp   # POSIX implementation (Linux — in progress)
│
├── tests/
│   └── send_packets.py        # UDP packet flood tool for live testing
│
└── CMakeLists.txt
```

---

## Performance

Benchmarked on Windows 11, MinGW GCC 15.2, localhost UDP loopback.

| Metric | Result |
|---|---|
| Python flood sender throughput | **203,138 packets/sec** |
| Received vs sent (no drop) | **100% — 0 packets dropped** |
| Heap allocations per packet (hot path) | **0** |
| Lock acquisitions per packet | **0** |
| Pool acquire/release | O(1) — pointer stack pop/push |
| Queue push/pop | O(1) — single atomic CAS |

The benchmark target (`tickwire_benchmark`) runs 1,000,000 packets through the BufferPool and SPSC queue in pure memory with no network I/O, measuring the raw pipeline throughput in isolation.

---

## Build Instructions

### Prerequisites

- CMake 3.16 or higher
- C++20-capable compiler:
    - **Windows:** MinGW-w64 (GCC 12+) or MSVC 2022
    - **Linux:** GCC 12+ or Clang 15+
- Python 3.8+ (for the test sender only)

### Windows (CLion / MinGW)

```bash
# Clone the repository
git clone https://github.com/Luke4721/tickwire.git
cd tickwire

# Open in CLion and let it configure CMake automatically, OR build manually:
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

### Linux

```bash
git clone https://github.com/Luke4721/tickwire.git
cd tickwire
mkdir build && cd build
cmake ..
make -j4
```

### Build Targets

| Target | Description |
|---|---|
| `tickwire_app` | Main application — opens UDP socket, runs receiver + worker loop |
| `tickwire_benchmark` | Memory-only throughput benchmark — no network required |

---

## Running the Tests

### Step 1 — Start the receiver

In CLion, select **tickwire_app** and run it. You should see:

```
TickWire listening on UDP port 54000...
```

### Step 2 — Run the Python packet sender

Open a separate terminal in the project root:

```bash
# Default: 100,000 packets, paced at 0.1ms intervals
python tests/send_packets.py

# Quick smoke test — 500 packets
python tests/send_packets.py 500

# Flood test — maximum rate, no delay
python tests/send_packets.py 100000 54000 0

# Custom: 50,000 packets to port 54000 at 0.05ms intervals
python tests/send_packets.py 50000 54000 0.00005
```

### Step 3 — Observe the stats

The receiver prints live metrics to the console every 128 loop iterations:

```
==== TickWire Stats ====
Received:        100000
Enqueued:        100000
Dropped (pool):  0
Dropped (queue): 0
========================
```

**What healthy output looks like:** `Received == Enqueued`, both drop counters at zero.

**What to do if drops appear:**
- `Dropped (pool) > 0` — increase the `BufferPool` capacity in `main.cpp` (default: 1024)
- `Dropped (queue) > 0` — increase the `SPSCQueue` capacity, or move receiver and worker onto separate threads so the worker doesn't block the receive loop

### Step 4 — Run the benchmark

Select **tickwire_benchmark** in CLion and run it. No Python or network needed:

```
=== TickWire Benchmark ===
Total attempted: 1000000
Pushed:          1000000
Processed:       1000000
Dropped:         0
Time:            0.031 sec
Throughput:      32,258,064 packets/sec
```

---

## Possible Applications

TickWire's design constraints — zero allocation, deterministic latency, lock-free handoff — make it suitable as a foundation for any system where latency variance (jitter) is as damaging as absolute latency.

**Algorithmic / High-Frequency Trading**
Market data feeds deliver price ticks at rates of hundreds of thousands of updates per second. A single missed tick or a GC pause can mean a missed order fill. TickWire's fixed-memory model eliminates the allocator-induced jitter that plagues standard networking stacks in this domain.

**Multiplayer Game Servers**
Real-time games (first-person shooters, racing simulations) use UDP for position and input updates because TCP's retransmission model introduces unacceptable latency. TickWire's pipeline maps directly to the game server receive loop: ingest packet → decode → apply game state → release buffer.

**Real-Time Sensor Telemetry**
Autonomous vehicles, robotics, and industrial IoT systems stream sensor data (LiDAR point clouds, IMU readings, camera frames) over UDP at fixed intervals. Dropped packets corrupt state; jitter corrupts timing. TickWire provides the receive infrastructure with bounded worst-case latency.

**Low-Latency Audio / MIDI Networks**
Professional audio protocols (Dante, AES67) transport audio over IP with sub-millisecond latency budgets. The packet pipeline in TickWire — receive, timestamp, hand off to a worker thread for decoding — maps directly to the real-time audio thread model.

**Financial Risk Systems**
Post-trade risk calculations must process trade confirmations as they arrive. A risk engine built on TickWire can ingest trade events from a matching engine feed, run position checks, and emit alerts without allocator pressure causing latency spikes at month-end peak load.

**Network Monitoring and Packet Capture**
High-rate packet capture tools (analogous to a lightweight Wireshark backend) need to receive and inspect packets faster than they arrive. TickWire's `drain()` loop and timestamping infrastructure provide the capture front-end; the worker can filter, aggregate, or forward.

---

## Roadmap

| Priority | Item | Notes |
|---|---|---|
| High | Linux POSIX socket implementation | `socket_posix.cpp` stub exists, needs `recvfrom`/`sendto`/`bind` |
| High | Separate receiver and worker threads | SPSC queue designed for this; `main.cpp` threading model in progress |
| Medium | Hardware timestamping (`SO_TIMESTAMPING`) | Nanosecond-precision timestamps on Linux without `steady_clock` overhead |
| Medium | Packet prioritization | Priority queue variant of SPSC for differentiated service classes |
| Low | Reliability layer over UDP | Selective ACK, sequence numbers — for use cases that need ordered delivery |
| Low | GUI metrics visualizer | Real-time throughput and drop-rate graphs |

---

## License

MIT — see [LICENSE](LICENSE) for details.

---

*Built as a systems programming study in zero-cost abstractions, memory ownership, and lock-free concurrency in C++20.*