/* P2P 程序客户端
 * 
 * 文件名：P2PClient.c
 *
 * 日期：2004-5-21
 *
 * 作者：shootingstars(zhouhuis22@sina.com)
 *
 */

#pragma comment(lib,"ws2_32.lib")

#include "windows.h"
#include "..\proto.h"
#include "..\Exception.h"
#include <iostream>
using namespace std;

UserList ClientList;



#define COMMANDMAXC 256
#define MAXRETRY    5

SOCKET PrimaryUDP;
char UserName[10];
char ServerIP[20];

bool RecvedACK;

void InitWinSock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Windows sockets 2.2 startup");
		throw Exception("");
	}
	else{
		printf("Using %s (Status: %s)\n",
			wsaData.szDescription, wsaData.szSystemStatus);
		printf("with API versions %d.%d to %d.%d\n\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
			LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	}
}

SOCKET mksock(int type)
{
	SOCKET sock = socket(AF_INET, type, 0);
	if (sock < 0)
	{
        printf("create socket error");
		throw Exception("");
	}
	return sock;
}

stUserListNode GetUser(char *username)
{
	for(UserList::iterator UserIterator=ClientList.begin();
						UserIterator!=ClientList.end();
							++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	throw Exception("not find this user");
}

void BindSock(SOCKET sock)
{
	sockaddr_in sin;
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	
	if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
		throw Exception("bind error");
}

void ConnectToServer(SOCKET sock,char *username, char *serverip)
{
	sockaddr_in remote;
	remote.sin_addr.S_un.S_addr = inet_addr(serverip);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(SERVER_PORT);
	
	stMessage sendbuf;
	sendbuf.iMessageType = LOGIN;
	strncpy(sendbuf.message.loginmember.userName, username, 10);

	sendto(sock, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote,sizeof(remote));

	int usercount;
	int fromlen = sizeof(remote);
	int iread = recvfrom(sock, (char *)&usercount, sizeof(int), 0, (sockaddr *)&remote, &fromlen);
	if(iread<=0)
	{
		throw Exception("Login error\n");
	}

	// 登录到服务端后，接收服务端发来的已经登录的用户的信息
	cout<<"Have "<<usercount<<" users logined server:"<<endl;
	for(int i = 0;i<usercount;i++)
	{
		stUserListNode *node = new stUserListNode;
		recvfrom(sock, (char*)node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
		ClientList.push_back(node);
		cout<<"Username:"<<node->userName<<endl;
		in_addr tmp;
		tmp.S_un.S_addr = htonl(node->ip);
		cout<<"UserIP:"<<inet_ntoa(tmp)<<endl;
		cout<<"UserPort:"<<node->port<<endl;
		cout<<""<<endl;
	}
}

void OutputUsage()
{
	cout<<"You can input you command:\n"
		<<"Command Type:\"send\",\"exit\",\"getu\"\n"
		<<"Example : send Username Message\n"
		<<"          exit\n"
		<<"          getu\n"
		<<endl;
}

/* 这是主要的函数：发送一个消息给某个用户(C)
 *流程：直接向某个用户的外网IP发送消息，如果此前没有联系过
 *      那么此消息将无法发送，发送端等待超时。
 *      超时后，发送端将发送一个请求信息到服务端，
 *      要求服务端发送给客户C一个请求，请求C给本机发送打洞消息
 *      以上流程将重复MAXRETRY次
 */
bool SendMessageTo(char *UserName, char *Message)
{
	char realmessage[256];
	unsigned int UserIP;
	unsigned short UserPort;
	bool FindUser = false;
	for(UserList::iterator UserIterator=ClientList.begin();
						UserIterator!=ClientList.end();
						++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), UserName) == 0 )
		{
			UserIP = (*UserIterator)->ip;
			UserPort = (*UserIterator)->port;
			FindUser = true;
		}
	}

	if(!FindUser)
		return false;

	strcpy(realmessage, Message);
	for(int i=0;i<MAXRETRY;i++)
	{
		RecvedACK = false;

		sockaddr_in remote;
		remote.sin_addr.S_un.S_addr = htonl(UserIP);
		remote.sin_family = AF_INET;
		remote.sin_port = htons(UserPort);
		stP2PMessage MessageHead;
		MessageHead.iMessageType = P2PMESSAGE;
		MessageHead.iStringLen = (int)strlen(realmessage)+1;
		int isend = sendto(PrimaryUDP, (const char *)&MessageHead, sizeof(MessageHead), 0, (const sockaddr*)&remote, sizeof(remote));
		isend = sendto(PrimaryUDP, (const char *)&realmessage, MessageHead.iStringLen, 0, (const sockaddr*)&remote, sizeof(remote));
		
		// 等待接收线程将此标记修改
		for(int j=0;j<10;j++)
		{
			if(RecvedACK)
				return true;
			else
				Sleep(300);
		}

		// 没有接收到目标主机的回应，认为目标主机的端口映射没有
		// 打开，那么发送请求信息给服务器，要服务器告诉目标主机
		// 打开映射端口（UDP打洞）
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);
	
		stMessage transMessage;
		transMessage.iMessageType = P2PTRANS;
		strcpy(transMessage.message.translatemessage.userName, UserName);

		sendto(PrimaryUDP, (const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr*)&server, sizeof(server));
		Sleep(100);// 等待对方先发送信息。
	}
	return false;
}

// 解析命令，暂时只有exit和send命令
// 新增getu命令，获取当前服务器的所有用户
void ParseCommand(char * CommandLine)
{
	if(strlen(CommandLine)<4)
		return;
	char Command[10];
	strncpy(Command, CommandLine, 4);
	Command[4]='\0';

	if(strcmp(Command,"exit")==0)
	{
		stMessage sendbuf;
		sendbuf.iMessageType = LOGOUT;
		strncpy(sendbuf.message.logoutmember.userName, UserName, 10);
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);

		sendto(PrimaryUDP,(const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr *)&server, sizeof(server));
		shutdown(PrimaryUDP, 2);
		closesocket(PrimaryUDP);
		exit(0);
	}
	else if(strcmp(Command,"send")==0)
	{
		char sendname[20];
		char message[COMMANDMAXC];
		int i;
		for(i=5;;i++)
		{
			if(CommandLine[i]!=' ')
				sendname[i-5]=CommandLine[i];
			else
			{
				sendname[i-5]='\0';
				break;
			}
		}
		strcpy(message, &(CommandLine[i+1]));
		if(SendMessageTo(sendname, message))
			printf("Send OK!\n");
		else 
			printf("Send Failure!\n");
	}
	else if(strcmp(Command,"getu")==0)
	{
		int command = GETALLUSER;
		sockaddr_in server;
		server.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		server.sin_family = AF_INET;
		server.sin_port = htons(SERVER_PORT);

		sendto(PrimaryUDP,(const char*)&command, sizeof(command), 0, (const sockaddr *)&server, sizeof(server));
	}
}

// 接受消息线程
DWORD WINAPI RecvThreadProc(LPVOID lpParameter)
{
	sockaddr_in remote;
	int sinlen = sizeof(remote);
	stP2PMessage recvbuf;
	for(;;)
	{
		int iread = recvfrom(PrimaryUDP, (char *)&recvbuf, sizeof(recvbuf), 0, (sockaddr *)&remote, &sinlen);
		if(iread<=0)
		{
			printf("recv error\n");
			continue;
		}
		switch(recvbuf.iMessageType)
		{
		case P2PMESSAGE:
			{
				// 接收到P2P的消息
				char *comemessage= new char[recvbuf.iStringLen];
				int iread1 = recvfrom(PrimaryUDP, comemessage, 256, 0, (sockaddr *)&remote, &sinlen);
				comemessage[iread1-1] = '\0';
				if(iread1<=0)
					throw Exception("Recv Message Error\n");
				else
				{
					printf("Recv a Message:%s\n",comemessage);
					
					stP2PMessage sendbuf;
					sendbuf.iMessageType = P2PMESSAGEACK;
					sendto(PrimaryUDP, (const char*)&sendbuf, sizeof(sendbuf), 0, (const sockaddr*)&remote, sizeof(remote));
				}

				delete []comemessage;
				break;

			}
		case P2PSOMEONEWANTTOCALLYOU:
			{
				// 接收到打洞命令，向指定的IP地址打洞
				printf("Recv p2someonewanttocallyou data\n");
				sockaddr_in remote;
				remote.sin_addr.S_un.S_addr = htonl(recvbuf.iStringLen);
				remote.sin_family = AF_INET;
				remote.sin_port = htons(recvbuf.Port);

				// UDP hole punching
				stP2PMessage message;
				message.iMessageType = P2PTRASH;
				sendto(PrimaryUDP, (const char *)&message, sizeof(message), 0, (const sockaddr*)&remote, sizeof(remote));
                
				break;
			}
		case P2PMESSAGEACK:
			{
				// 发送消息的应答
				RecvedACK = true;
				break;
			}
		case P2PTRASH:
			{
				// 对方发送的打洞消息，忽略掉。
				//do nothing ...
				printf("Recv p2ptrash data\n");
				break;
			}
		case GETALLUSER:
			{
				int usercount;
				int fromlen = sizeof(remote);
				int iread = recvfrom(PrimaryUDP, (char *)&usercount, sizeof(int), 0, (sockaddr *)&remote, &fromlen);
				if(iread<=0)
				{
					throw Exception("Login error\n");
				}
				
				ClientList.clear();

				cout<<"Have "<<usercount<<" users logined server:"<<endl;
				for(int i = 0;i<usercount;i++)
				{
					stUserListNode *node = new stUserListNode;
					recvfrom(PrimaryUDP, (char*)node, sizeof(stUserListNode), 0, (sockaddr *)&remote, &fromlen);
					ClientList.push_back(node);
					cout<<"Username:"<<node->userName<<endl;
					in_addr tmp;
					tmp.S_un.S_addr = htonl(node->ip);
					cout<<"UserIP:"<<inet_ntoa(tmp)<<endl;
					cout<<"UserPort:"<<node->port<<endl;
					cout<<""<<endl;
				}
				break;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	try
	{
		InitWinSock();
		
		PrimaryUDP = mksock(SOCK_DGRAM);
		BindSock(PrimaryUDP);

		cout<<"Please input server ip:";
		cin>>ServerIP;

		cout<<"Please input your name:";
		cin>>UserName;

		ConnectToServer(PrimaryUDP, UserName, ServerIP);

		HANDLE threadhandle = CreateThread(NULL, 0, RecvThreadProc, NULL, NULL, NULL);
		CloseHandle(threadhandle);
		OutputUsage();

		for(;;)
		{
			char Command[COMMANDMAXC];
			gets(Command);
			ParseCommand(Command);
		}
	}
	catch(Exception &e)
	{
		printf(e.GetMessage());
		return 1;
	}
	return 0;
}

