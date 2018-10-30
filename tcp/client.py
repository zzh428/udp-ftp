import socket
import sys
import os
import random
import re

class Client:

    def __init__(self,IP,port):
        self.serverIP = IP
        self.port = port
        self.localIP = self.getLocalIP()
        self.datasocket = None
        self.sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #try:
        self.sk.connect((self.serverIP,self.port))
        self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
        print(self.recv) #print greeting message
        #except:
            #print("Connection Error")
            #exit(0)
        self.eventloop()
    
    def eventloop(self):
        while True:
            self.cmdLine = input().strip()
            cmd =  self.cmdLine.split(' ')[0]
            if cmd in ["USER", "PASS", "SYST", "TYPE", "MKD", "CWD", "PWD", "RMD", "RNFR", "RNTO"]:
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                print(self.recv)
            elif cmd == "QUIT" or cmd == "ABOR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
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
                        self.datasocket.bind((self.localIP,self.dataport))
                        self.datasocket.listen()
                    except:
                        continue
                    break
                print("Listening on port %d"%(self.dataport))
                self.sk.send(bytes("PORT " + ','.join(self.localIP.split('.')) + ',' + 
                str(self.dataport // 256) + ',' + str(self.dataport % 256) + "\r\n",encoding='utf-8'))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                print(self.recv)
                if not self.recv.startswith("200"):
                    self.datasocket.close()
                    self.datasocket = None

            elif cmd == "PASV":
                if self.datasocket is not None:
                    self.datasocket.close()
                self.dataMode = 1
                self.sk.send(bytes("PASV" + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                print(self.recv)
                if self.recv.startswith("227"):
                    pasv_address = re.split('[()]',self.recv)[1]
                    pasv_address = pasv_address.split(',')
                    self.dataport = int(pasv_address[-1]) + 256 * int(pasv_address[-2])
                    self.pasv_ip = pasv_address[0] + '.' + pasv_address[1] + '.' + pasv_address[2] + '.' + pasv_address[3]

            elif cmd == "RETR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                print(self.recv)
                if self.recv.startswith("150"):
                    if self.dataMode:
                        self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.datasocket.connect((self.pasv_ip,self.dataport))
                        f = open(re.split('[/ ]',self.cmdLine)[-1],'wb')
                        while(1):
                            datarecv = self.datasocket.recv(8192)
                            if not datarecv:
                                break
                            f.write(datarecv)
                        f.close()
                        self.datasocket.close()
                        self.datasocket = None
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
                    self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                    print(self.recv)

            elif cmd == "STOR":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")
                print(self.recv)
                if self.recv.startswith("150"):
                    if self.dataMode:
                        self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.datasocket.connect((self.pasv_ip,self.dataport))
                        f = open(re.split(' ',self.cmdLine)[-1],'rb')
                        datarecv = self.datasocket.sendall(f.read())
                        if not datarecv:
                            break
                        f.write(datarecv)
                        f.close()
                        self.datasocket.close()
                        self.datasocket = None
                    else:
                        s,_ = self.datasocket.accept()
                        f = open(re.split(' ',self.cmdLine)[-1],'rb')
                        datarecv = s.sendall(f.read())
                        f.close()
                        s.close()
                    self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                    print(self.recv)

            elif cmd == "LIST":
                self.sk.send(bytes(self.cmdLine + "\r\n",encoding="utf-8"))
                self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                print(self.recv)
                if self.recv.startswith("150"):
                    flist = ""
                    if self.dataMode:
                        self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.datasocket.connect((self.pasv_ip,self.dataport))
                        while(1):
                            datarecv = self.datasocket.recv(8192)
                            if not datarecv:
                                break
                            flist += str(datarecv,encoding='utf-8')
                        self.datasocket.close()
                        self.datasocket = None
                    else:
                        s,_ = self.datasocket.accept()
                        while(1):
                            datarecv = s.recv(8192)
                            if not datarecv:
                                break
                            flist += str(datarecv,encoding='utf-8')
                        s.close()
                    print(flist)
                    self.recv = str(self.sk.recv(8192),encoding="utf-8")[:-1]
                    print(self.recv)


    def getLocalIP(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        finally:
            s.close()
        return ip



    

if __name__ == "__main__":
    if(len(sys.argv) == 3):
        c = Client(sys.argv[1],sys.argv[2])
    else:
        c = Client("127.0.0.1",21)



