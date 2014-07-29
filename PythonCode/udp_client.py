from socket import *

HOST='192.168.1.2';
PORT=8000;
BUFFERSIZE=1024;
ADDR=(HOST, PORT);

udpClientSocket=socket(AF_INET,SOCK_DGRAM);

while True:
    data=raw_input('>');
    if not data:
        break;
    udpClientSocket.sendto(data,ADDR);
    data,addr=udpClientSocket.recvfrom(BUFFERSIZE);
    if not data:
        break;
    print data

udpClientSocket.close();
