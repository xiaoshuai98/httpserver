#! /usr/bin/python3
# -*- coding: UTF-8 -*-

import sys
import os
from art import *

str = os.environ["REQUEST_URI"].split('=')[1]

art = text2art(str)

with open('temp', 'w') as f:
  f.write(art)

print("HTTP/1.1 200 OK")
print("Content-Type:text/plain")
with open('temp', 'r') as f:
  for line in f:
    print(line)
