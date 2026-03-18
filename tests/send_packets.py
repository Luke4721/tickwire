import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

target = ("127.0.0.1", 54000)

for i in range(100000):
    message = "Hello World"
    sock.sendto(message, target)

    time.sleep(0.0001)

print("DONE")
sock.close()