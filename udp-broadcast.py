#!/usr/bin/env python3

import os, sys, socket
from time import sleep

port = 1234
try:
	port = int(sys.argv[1])
except:
	pass

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.connect(("8.8.8.8", 80))
IP = sock.getsockname()[0]
sock.close()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.bind((IP, 0))

while True:
	msg=input()
	s.sendto(msg.encode('utf-8','ignore'), ("255.255.255.255", port))
	print(f'UDP Message broadcasted to 255.255.255.255 port {port}')

s.close()

