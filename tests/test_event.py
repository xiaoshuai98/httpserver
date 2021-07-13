import sys
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
s.connect(('127.0.0.1', 9999));

#TODO(qds): Add concurrent connections.
line = sys.stdin.readline();
while line:
  s.send(line.encode());
  data = s.recv(32).decode();
  assert(data == line);
  line = sys.stdin.readline();
