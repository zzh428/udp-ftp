import socket
import sys
import os
import random
import re
import mainwindow
from PyQt5.QtWidgets import QApplication, QMainWindow, QInputDialog, QFileDialog, QLineEdit, QTableWidgetItem, QMessageBox

class ClientLogic(QMainWindow):

    def __init__(self, ui:mainwindow.Ui_MainWindow):
        QMainWindow.__init__(self)
        self.ui = ui
        self.serverIP = "127.0.0.1"
        self.serverPort = 21
        self.localIP = self.getLocalIP()
        self.datasocket = None
        self.sk = None
        self.recv = ""
        self.bind()

    def bind(self):
        self.ui.portButton.setChecked(True)
        self.ui.pasvButton.setChecked(False)
        self.ui.portButton.clicked.connect(self.changeMode)
        self.ui.pasvButton.clicked.connect(self.changeMode)
        self.ui.connectButton.clicked.connect(self.connect)
        self.ui.loginButton.clicked.connect(self.login)
        self.ui.SYSTButton.clicked.connect(self.syst)
        self.ui.TYPEButton.clicked.connect(self.type)
        self.ui.RETRButton.clicked.connect(self.retr)
        self.ui.STORButton.clicked.connect(self.stor)
        self.ui.LISTButton.clicked.connect(self.list)
        self.ui.QUITButton.clicked.connect(self.quit)
        self.ui.MKDButton.clicked.connect(self.mkd)
        self.ui.CWDButton.clicked.connect(self.cwd)
        self.ui.PWDButton.clicked.connect(self.pwd)
        self.ui.RMDButton.clicked.connect(self.rmd)
        self.ui.MVButton.clicked.connect(self.mv)

    def getLocalIP(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        except:
            err = QMessageBox.warning(self,' ','无网络连接!',QMessageBox.Yes)
            exit(0)
        finally:
            s.close()
        return ip

    def printLog(self, text:str):
        if text[-1] == '\r' or text[-1] == '\n':
            text = text[:-1]
        self.ui.cmdText.append(text)

    def changeMode(self):
        button = self.sender()
        if button == self.ui.portButton:
            self.ui.portButton.setChecked(True)
            self.ui.pasvButton.setChecked(False)
        else:
            self.ui.portButton.setChecked(False)
            self.ui.pasvButton.setChecked(True)


    def connect(self):
        if(self.ui.IPInput.text() != ""):
            self.serverIP = self.ui.IPInput.text()
        if(self.ui.portInput.text() != ""):
            self.serverPort = int(self.ui.portInput.text())
        try:
            self.sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sk.connect((self.serverIP, self.serverPort))
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)
        except:
            self.printLog("Connection Error")

    def login(self):
        username = self.ui.usernameInput.text()
        password = self.ui.passwordInput.text()
        self.sk.send(bytes('USER ' + username + '\r\n',encoding='utf-8'))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        if not self.recv.startswith("331"):
            self.printLog(self.recv)
            return
        self.sk.send(bytes('PASS ' + password + '\r\n',encoding='utf-8'))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def syst(self):
        self.sk.send(bytes('SYST' + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def type(self):
        self.sk.send(bytes('TYPE I' + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def port(self, p=True):
        if self.datasocket is not None:
            self.datasocket.close()
        self.dataMode = 0
        while (1):
            self.dataport = random.randint(20000, 65535)
            try:
                self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.datasocket.bind((self.localIP, self.dataport))
                self.datasocket.listen()
            except:
                continue
            break
        if print:
            self.printLog("Listening on port %d" % (self.dataport))
        self.sk.send(bytes("PORT " + ','.join(self.localIP.split('.')) + ',' +
                           str(self.dataport // 256) + ',' + str(self.dataport % 256) + "\r\n",
                           encoding='utf-8'))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        if p:
            self.printLog(self.recv)
        if not self.recv.startswith("200"):
            print(self.recv)
            self.datasocket.close()
            self.printLog("Error occurred, close listening socket")
            self.datasocket = None

    def pasv(self, print=True):
        if self.datasocket is not None:
            self.datasocket.close()
        self.dataMode = 1
        self.sk.send(bytes("PASV" + "\r\n", encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        if print:
            self.printLog(self.recv)
        if self.recv.startswith("227"):
            pasv_address = re.split('[()]', self.recv)[1]
            pasv_address = pasv_address.split(',')
            self.dataport = int(pasv_address[-1]) + 256 * int(pasv_address[-2])
            self.pasv_ip = pasv_address[0] + '.' + pasv_address[1] + '.' + pasv_address[2] + '.' + pasv_address[3]
            self.datasocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.datasocket.connect((self.pasv_ip, self.dataport))

    def checkRest(self, fileName):
        rest = 0
        if os.path.exists(re.split('[/ ]', fileName)[-1]):
            reply = QMessageBox.question(self, ' ', re.split('[/ ]', fileName)[-1] + '已存在,是否断点续传?',
                                         QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                rest = 1
                size = os.path.getsize(re.split('[/ ]', fileName)[-1])
                self.sk.send(bytes('REST ' + str(size) + '\r\n', encoding="utf-8"))
                self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
                self.printLog(self.recv)
                if not self.recv.startswith("350"):
                    rest = 0
            else:
                rest = 0
        return rest

    def retr(self):
        fileName,_ = QInputDialog.getText(self,"请输入文件名","",QLineEdit.Normal,"")
        if len(fileName) == 0:
            return
        rest = self.checkRest(fileName)

        if self.ui.portButton.isChecked():
            self.port()
        else:
            self.pasv()

        self.sk.send(bytes('RETR '+ fileName + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)
        if self.recv.startswith("150"):
            if self.ui.pasvButton.isChecked():
                if rest:
                    f = open(re.split('[/ ]', fileName)[-1], 'ab')
                else:
                    f = open(re.split('[/ ]', fileName)[-1], 'wb')
                while (1):
                    datarecv = self.datasocket.recv(8192)
                    if not datarecv:
                        break
                    f.write(datarecv)
                f.close()
                self.datasocket.close()
                self.datasocket = None
            else:
                s, _ = self.datasocket.accept()
                if rest:
                    f = open(re.split('[/ ]', fileName)[-1], 'ab')
                else:
                    f = open(re.split('[/ ]', fileName)[-1], 'wb')
                while (1):
                    datarecv = s.recv(8192)
                    if not datarecv:
                        break
                    f.write(datarecv)
                f.close()
                s.close()
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)
            message = QMessageBox.information(self, ' ', '下载完成', QMessageBox.Ok)

    def checkAppe(self, fileName):
        appe = 0
        size = 0
        if self.ui.portButton.isChecked():
            self.port()
        else:
            self.pasv()
        self.sk.send(bytes('LIST ' + re.split('[/ ]', fileName)[-1] + "\r\n", encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        if self.recv.startswith("150"):
            flist = ""
            if self.ui.pasvButton.isChecked():
                while (1):
                    datarecv = self.datasocket.recv(8192)
                    if not datarecv:
                        break
                    flist += str(datarecv, encoding='utf-8')
                self.datasocket.close()
                self.datasocket = None
            else:
                s, _ = self.datasocket.accept()
                while (1):
                    datarecv = s.recv(8192)
                    if not datarecv:
                        break
                    flist += str(datarecv, encoding='utf-8')
                s.close()
            flist = flist.splitlines(keepends=False)
            for l in flist:
                l = l.split()
                if re.split('[/ ]', l[-1])[-1] == re.split('[/ ]', fileName)[-1] and l[0][0] == '-':
                    if int(l[4]) < os.path.getsize(fileName) and int(l[4]) > 0:
                        reply = QMessageBox.question(self, ' ', re.split('[/ ]', fileName)[-1] + '已存在,是否断点续传?',
                                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
                        if reply == QMessageBox.Yes:
                            appe = 1
                            size = int(l[4])
                        break
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)
        return appe, size

    def stor(self):
        fileName, _ = QFileDialog.getOpenFileName(self, "选择上传文件", os.getcwd(), "All Files(*)")
        if len(fileName) == 0:
            return
        appe, appeSize = self.checkAppe(fileName)
        if self.ui.portButton.isChecked():
            self.port()
        else:
            self.pasv()
        if appe:
            self.sk.send(bytes('APPE ' + re.split('[/ ]', fileName)[-1] + '\r\n', encoding="utf-8"))
        else:
            self.sk.send(bytes('STOR ' + re.split('[/ ]', fileName)[-1] + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)
        if self.recv.startswith("150"):
            if self.ui.pasvButton.isChecked():
                f = open(fileName, 'rb')
                if appe:
                    f.seek(appeSize-1,0)
                self.datasocket.sendall(f.read())
                f.close()
                self.datasocket.close()
                self.datasocket = None
            else:
                s, _ = self.datasocket.accept()
                f = open(fileName, 'rb')
                if appe:
                    f.seek(appeSize-1,0)
                s.sendall(f.read())
                f.close()
                s.close()
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)
            message = QMessageBox.information(self, ' ', '上传完成', QMessageBox.Ok)

    def list(self):
        if self.ui.portButton.isChecked():
            self.port()
        else:
            self.pasv()
        self.sk.send(bytes('LIST' + "\r\n", encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)
        if self.recv.startswith("150"):
            flist = ""
            if self.ui.pasvButton.isChecked():
                while (1):
                    datarecv = self.datasocket.recv(8192)
                    if not datarecv:
                        break
                    flist += str(datarecv, encoding='utf-8')
                self.datasocket.close()
                self.datasocket = None
            else:
                s, _ = self.datasocket.accept()
                while (1):
                    datarecv = s.recv(8192)
                    if not datarecv:
                        break
                    flist += str(datarecv, encoding='utf-8')
                s.close()
            flist = flist.splitlines(keepends=False)
            self.ui.fileTable.setRowCount(len(flist)-1)
            for i,l in enumerate(flist):
                if i == 0:
                    continue
                ls = l.split()
                newItem = QTableWidgetItem(ls[-1])
                self.ui.fileTable.setItem(i - 1, 0, newItem)
                if ls[0][0] == '-':
                    newItem = QTableWidgetItem(u'文件')
                elif ls[0][0] == 'd':
                    newItem = QTableWidgetItem(u'目录')
                else:
                    newItem = QTableWidgetItem(u'其他')
                self.ui.fileTable.setItem(i - 1, 1, newItem)
                newItem = QTableWidgetItem(ls[4])
                self.ui.fileTable.setItem(i - 1, 2, newItem)
                newItem = QTableWidgetItem(ls[-4] + ' ' + ls[-3] + ' ' + ls[-2])
                self.ui.fileTable.setItem(i - 1, 3, newItem)
                newItem = QTableWidgetItem(ls[0][1:])
                self.ui.fileTable.setItem(i - 1, 4, newItem)
                newItem = QTableWidgetItem(ls[2])
                self.ui.fileTable.setItem(i - 1, 5, newItem)
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)


    def mkd(self):
        dirName, _ = QInputDialog.getText(self, "请输入目录名", "", QLineEdit.Normal, "")
        if len(dirName) == 0:
            return
        self.sk.send(bytes('MKD ' + dirName + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def cwd(self):
        dirName, _ = QInputDialog.getText(self, "请输入目录名", "", QLineEdit.Normal, "")
        if len(dirName) == 0:
            return
        self.sk.send(bytes('CWD ' + dirName + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def pwd(self):
        self.sk.send(bytes('PWD' + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def rmd(self):
        dirName, _ = QInputDialog.getText(self, "请输入目录名", "", QLineEdit.Normal, "")
        if len(dirName) == 0:
            return
        self.sk.send(bytes('RMD ' + dirName + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)

    def mv(self):
        dirNameOld,_ = QInputDialog.getText(self, "请输入旧文件名", "", QLineEdit.Normal, "")
        if len(dirNameOld) == 0:
            return
        self.sk.send(bytes('RNFR ' + dirNameOld + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)
        if self.recv.startswith('350'):
            dirNameNew, _ = QInputDialog.getText(self, "请输入新文件名", "", QLineEdit.Normal, "")
            if len(dirNameNew) == 0:
                return
            self.sk.send(bytes('RNTO ' + dirNameNew + '\r\n', encoding="utf-8"))
            self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
            self.printLog(self.recv)

    def quit(self):
        self.sk.send(bytes('QUIT' + '\r\n', encoding="utf-8"))
        self.recv = str(self.sk.recv(8192), encoding="utf-8")[:-1]
        self.printLog(self.recv)
        self.sk.close()
        if self.datasocket is not None:
            self.datasocket.close()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    MainWindow = QMainWindow()
    ui = mainwindow.Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()

    c = ClientLogic(ui)
    sys.exit(app.exec_())
