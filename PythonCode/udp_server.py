from socket import *
from time import ctime

HOST=''
PORT=8000;
BUFFERSIZE=1024;
ADDR= (HOST, PORT)

udpServerSocket=socket(AF_INET,SOCK_DGRAM);
udpServerSocket.bind(ADDR);

while True:
    print 'waiting for message ......';
    data,addr=udpServerSocket.recvfrom(BUFFERSIZE);
    print addr,':',data
    udpServerSocket.sendto('[%s] %s'%(ctime(),data),addr);
    print '...received from and returned to:',addr

udpServerSocket.close();
