import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
s.connect(('127.0.0.1', 9999));

s.send('12345678'.encode());
data = s.recv(32).decode();
assert(data == '12345678');

s.send('12345678123456781234'.encode());
data = s.recv(32).decode();
assert(data == '1234567812345678');

s.send('56781234567812345678'.encode());
data = s.recv(32).decode();
assert(data == '123456781234567812345678');