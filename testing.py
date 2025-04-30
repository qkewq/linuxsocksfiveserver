import socket

#  create netcat listener
#  nc -l -s 127.0.0.1 -p 8081

sock = socket.socket()
sock.connect(('127.0.0.1', 8080))

method_request = b'\x05\x01\x00'
#127.0.0.1, 8081
socks_requeset = b'\x05\x01\x00\x01\x7f\x00\x00\x01\x1f\x91'

print("connected")
sock.sendall(method_request)
reply = sock.recv(1024)
print("REPLY:", reply)
sock.sendall(socks_requeset)
reply = sock.recv(1024)
print("REPLY:", reply)

reply = sock.recv(1024)
print("\nREPLY:", reply)

sock.close()
