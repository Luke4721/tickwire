"""
TickWire UDP packet sender — test utility
Usage:
    python send_packets.py                    # defaults: 100000 packets, port 54000
    python send_packets.py 500 54000          # 500 packets to port 54000
    python send_packets.py 100000 54000 0     # 100000 packets, no delay (flood test)
"""

import socket
import time
import sys

# --- Config ---
NUM_PACKETS = int(sys.argv[1]) if len(sys.argv) > 1 else 100_000
PORT        = int(sys.argv[2]) if len(sys.argv) > 2 else 54000
DELAY       = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0001  # seconds between packets

TARGET = ("127.0.0.1", PORT)

# FIX: Python 3 sendto() requires bytes, not str.
# Original: message = "Hello World"  → TypeError on Python 3
# Fixed:    message = b"Hello World" → correct bytes object
MESSAGE = b"Hello TickWire"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending {NUM_PACKETS} packets to {TARGET[0]}:{TARGET[1]} ...")
print(f"Delay between packets: {DELAY}s  ({'flood' if DELAY == 0 else 'paced'})")

start = time.perf_counter()

for i in range(NUM_PACKETS):
    sock.sendto(MESSAGE, TARGET)
    if DELAY > 0:
        time.sleep(DELAY)

elapsed = time.perf_counter() - start

sock.close()

print(f"\nDONE")
print(f"Sent:     {NUM_PACKETS} packets")
print(f"Time:     {elapsed:.3f}s")
print(f"Rate:     {NUM_PACKETS / elapsed:,.0f} packets/sec")