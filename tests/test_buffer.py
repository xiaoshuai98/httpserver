import socket

def test_buffer():
  print('test_buffer is running.');
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
  s.connect(('127.0.0.1', 9999));
  s.send('12345678'.encode());
  data = s.recv(32).decode();
  assert(data == '12345678');
  s.send('12345678123456781234'.encode());
  data = s.recv(32).decode();
  assert(data == '1234567812345678');
  s.send('56781234567812345678'.encode());
  data = s.recv(18).decode();
  assert(data == '123456781234567812');
  data = s.recv(32).decode();
  assert(data == '345678');
  data = s.recv(18).decode();
  assert(data == '');
  s.close();
  print('test_buffer is done.');

if __name__ == '__main__':
  test_buffer();
