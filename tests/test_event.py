import socket
import random
import string
from multiprocessing import  Process

def new_process():
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
  s.connect(('127.0.0.1', 10001));
  for i in range(100):
    line = ''.join(random.sample(string.ascii_letters + string.digits, 6))
    s.send(line.encode());
    data = s.recv(6).decode();
    assert(data == line);
  s.close();

def test_event():
  print('test_event is running.');
  process_list = [];
  for i in range(10):
    p = Process(target=new_process);
    p.start();
    process_list.append(p);
  for p in process_list:
    p.join();
  print('test_event is done.');

if __name__ == '__main__':
  test_event();
