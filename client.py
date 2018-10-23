import socket
 
size = 8192
 
#try:
for i in range(51):
  msg = bytes(str(i),encoding='utf-8')
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock.sendto(msg, ('localhost', 9876))
  print(str(sock.recv(size), encoding = "utf-8"))
  sock.close()
 
#except:
#  print("cannot reach the server")