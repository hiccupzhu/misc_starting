#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

void main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 1, 1 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        return;
    }


    if ( LOBYTE( wsaData.wVersion ) != 1 ||
        HIBYTE( wsaData.wVersion ) != 1 ) {
            WSACleanup( );
            return; 
    }

    SOCKET sockClient=socket(AF_INET,SOCK_DGRAM,0);
    SOCKADDR_IN addrClent;
    addrClent.sin_addr.S_un.S_addr=inet_addr("10.0.1.100");
    addrClent.sin_family=AF_INET;
    addrClent.sin_port=htons(30000);


    char* cmds[] = {
        "REALY00",
        "REALY10",
        "REALY01",
        "REALY11",
        NULL
    };
    char buf[100];
    int len = 0;
    for(int i = 0;;){
        printf("send:%s\n", cmds[i]);
        sendto(sockClient,cmds[i],strlen(cmds[i])+1,0, (SOCKADDR*)&addrClent,sizeof(SOCKADDR));
        i ++;
        if(cmds[i] == NULL) i = 0;
       
        Sleep(2000);
    }
    closesocket(sockClient);
    WSACleanup();
}
