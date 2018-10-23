import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('localhost', 9876))
count = 0

try:
  while True:
    data, address = sock.recvfrom(size)
    print(str(data,encoding='utf-8'))
    sock.sendto(bytes(count) +bytes(' ',encoding='utf-8') + data, address)
    count += 1
finally:
  sock.close()