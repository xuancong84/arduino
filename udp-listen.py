#!/usr/bin/env python3

import os,sys,socket

port = 1234
try:
	port = int(sys.argv[1])
except:
	pass

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(('0.0.0.0', port))
print(f'Bound to UDP 0.0.0.0:{port}')

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    print("received message: %s" % data)
