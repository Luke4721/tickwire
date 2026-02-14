# TickWire

TickWire is a deterministic, low-latency UDP transport core designed for
time-sensitive systems such as multiplayer servers and real-time simulations.

## Design Goals

- No dynamic allocation in the hot path
- Explicit ownership and lifetime of packet buffers
- Fixed-size pooled memory
- Bounded queues and predictable latency
- Early timestamping at kernel boundary
- Transport-layer only (no protocol assumptions)

TickWire is intentionally minimal. Higher-level reliability or streaming
layers are designed to sit above the core, not inside it.