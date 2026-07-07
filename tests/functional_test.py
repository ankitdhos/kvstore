import socket
import time

def send(sock, cmd):
    sock.sendall((cmd + "\n").encode())
    time.sleep(0.05)
    return sock.recv(4096).decode().strip()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(3)
s.connect(("127.0.0.1", 6380))

print("SET foo bar ->", send(s, "SET foo bar"))
print("GET foo ->", send(s, "GET foo"))
print("SET temp value1 2 ->", send(s, "SET temp value1 2"))  # TTL 2 seconds
print("GET temp ->", send(s, "GET temp"))
print("DEL foo ->", send(s, "DEL foo"))
print("GET foo (after del) ->", send(s, "GET foo"))

print("Sleeping 3s to let TTL expire...")
time.sleep(3)
print("GET temp (after ttl expiry) ->", send(s, "GET temp"))

s.close()
print("Test complete.")
