import socket
import time

def send(sock, cmd):
    sock.sendall((cmd + "\n").encode())
    time.sleep(0.05)
    return sock.recv(4096).decode().strip()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(3)
s.connect(("127.0.0.1", 6380))

print("SET persisted_key hello_world ->", send(s, "SET persisted_key hello_world"))
print("SET another_key another_value ->", send(s, "SET another_key another_value"))
s.close()
print("Data written. Now save snapshot happens on shutdown or we trigger manually.")
