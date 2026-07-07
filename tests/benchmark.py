import socket
import time
import random
import string

def rand_str(n=8):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=n))

def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5)
    s.connect(("127.0.0.1", 6380))
    return s

def send_recv(sock, cmd):
    sock.sendall((cmd + "\n").encode())
    return sock.recv(4096)

N = 20000  # number of operations to benchmark

print(f"Benchmarking {N} SET operations...")
s = connect()
keys = [f"key_{i}_{rand_str(4)}" for i in range(N)]
values = [rand_str(16) for _ in range(N)]

start = time.time()
for k, v in zip(keys, values):
    send_recv(s, f"SET {k} {v}")
elapsed_set = time.time() - start
set_ops_per_sec = N / elapsed_set
print(f"SET: {N} ops in {elapsed_set:.3f}s -> {set_ops_per_sec:.0f} ops/sec")

print(f"\nBenchmarking {N} GET operations...")
start = time.time()
hits = 0
for k in keys:
    resp = send_recv(s, f"GET {k}").decode().strip()
    if resp != "NOT_FOUND":
        hits += 1
elapsed_get = time.time() - start
get_ops_per_sec = N / elapsed_get
print(f"GET: {N} ops in {elapsed_get:.3f}s -> {get_ops_per_sec:.0f} ops/sec, {hits}/{N} hits")

s.close()
print("\nBenchmark complete.")
