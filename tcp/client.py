import socket
import sys
import os
import random
import re

class Client:

    def __init__(self):
        self.serverIP = "127.0.0.1"
        self.port = 21
        self.localIP = self.getLocalIP()
        self.datasocket = None
        args = input("Please input IP and Port, default: 127.0.0.1 21").strip()
        if args != "":
            args = args.split(' ')
            self.serverIP = args[0]
            self.port = int(args[1])
        self.sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sk.connect((self.serverIP,self.port))
            self.recv = str(self.sk.recv(8192),encoding="utf-8")
            print(self.recv) #print greeting message
        except:
            print("Connection Error")
            exit(0)
        
        self.eventloop()
    
    def eventloop(self):
        while(1):
            self.cmdLine = input().strip()
            cmd =  self.cmdLine.split(' ')[0]
            if cmd in ["USER", "PASS", "SYST", "TYPE", "MKD", "CWD", "PWD", "RMD", "RNFR", "RNTO"]:
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
            elif cmd == "QUIT" or cmd == "ABOR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
                self.sk.close()
                if self.datasocket is not None:
                    self.datasocket.close()
                return 0
            elif cmd == "PORT":
                if self.datasocket is not None:
                    self.datasocket.close()
                self.dataMode = 0
                while(1):
                    self.dataport = random.randint(20000,65535)
                    try:
                        self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.datasocket.bind(("127.0.0.1",self.dataport))
                        self.datasocket.listen()
                    except:
                        continue
                    break
                print("Listening on port %d"%(self.dataport))
                self.sk.send(bytes("PORT " + ','.join(self.localIP.split('.')) + ',' + 
                str(self.dataport // 256) + ',' + str(self.dataport % 256) + "\r\n",encoding='utf-8'))
            elif cmd == "PASV":
                if self.datasocket is not None:
                    self.datasocket.close()
                self.dataMode = 1
                self.sk.send(bytes("PASV" + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                pasv_address = re.split('[()]',self.recv)[1]
                pasv_address = pasv_address.split(',')
                self.dataport = int(pasv_address[-1]) + 256 * int(pasv_address[-2])
                self.pasv_ip = pasv_address[0] + '.' + pasv_address[1] + '.' + pasv_address[2] + '.' + pasv_address[3]
                self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            elif cmd == "RETR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
                if self.dataMode:
                    self.datasocket.connect((self.pasv_ip,self.dataport))
                    f = open(re.split('[/ ]',self.cmdLine)[-1],'wb')
                    while(1):
                        datarecv = self.datasocket.recv(8192)
                        if not datarecv:
                            break
                        f.write(datarecv)
                    f.close()
                    self.datasocket.close()
                else:
                    s,_ = self.datasocket.accept()
                    f = open(re.split('[/ ]',self.cmdLine)[-1],'wb')
                    while(1):
                        datarecv = s.recv(8192)
                        if not datarecv:
                            break
                        f.write(datarecv)
                    f.close()
                    s.close()
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
            elif cmd == "STOR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
                if self.dataMode:
                    self.datasocket.connect((self.pasv_ip,self.dataport))
                    f = open(re.split(' ',self.cmdLine)[-1],'rb')
                    datarecv = self.datasocket.sendall(f.read())
                    if not datarecv:
                        break
                    f.write(datarecv)
                    f.close()
                    self.datasocket.close()
                else:
                    s,_ = self.datasocket.accept()
                    f = open(re.split(' ',self.cmdLine)[-1],'rb')
                    datarecv = s.sendall(f.read())
                    f.close()
                    s.close()
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
            elif cmd == "LIST":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
                if self.dataMode:
                    self.datasocket.connect((self.pasv_ip,self.dataport))
                    while(1):
                        datarecv = self.datasocket.recv(8192)
                        if not datarecv:
                            break
                        print(str(datarecv,encoding='utf-8'))
                    self.datasocket.close()
                else:
                    s,_ = self.datasocket.accept()
                    while(1):
                        datarecv = s.recv(8192)
                        if not datarecv:
                            break
                        print(str(datarecv,encoding='utf-8'))
                    s.close()
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)


    def getCode(self):
        return int(self.recv.split(' ')[0])

    def getLocalIP(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        finally:
            s.close()
        return ip



    

if __name__ == "__main__":
    c = Client()



