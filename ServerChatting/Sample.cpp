#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;
struct User
{
	SOCKET m_sock;
	SOCKADDR_IN m_addr;
	string m_name;
	short m_port;
	void set(SOCKET sock, SOCKADDR_IN addr)
	{
		m_sock = sock;
		m_addr = addr;
		m_name = inet_ntoa(addr.sin_addr);
		m_port = ntohs(addr.sin_port);
	}
};
void main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	SOCKET listensock = socket(AF_INET,SOCK_STREAM,0);
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	int a = bind(listensock, (sockaddr*)&sa,sizeof(sa));
	if (a == SOCKET_ERROR) return;
	a = listen(listensock, SOMAXCONN);
	if(a == SOCKET_ERROR) return;

	SOCKADDR_IN clientaddr;
	int len = sizeof(clientaddr);

	cout << "서버 가동중~" << endl;

	u_long on = 1;
	ioctlsocket(listensock, FIONBIO, &on);

	list<User> userlist;

	while (1)
	{
		SOCKET clientsock = accept(listensock, (sockaddr*)&clientaddr, &len);
		if (clientsock == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				cout << "ERRORCODE = " << error << endl;
				break;
			}
		}
		else
		{
			User user;
			user.set(clientsock, clientaddr);
			userlist.push_back(user);

			cout << "ip = " << inet_ntoa(clientaddr.sin_addr)
				<< " port = " << ntohs(clientaddr.sin_port)
				<< "  " << endl;
			u_long on = 1;
			ioctlsocket(clientsock, FIONBIO, &on);
			cout << userlist.size() << "명 접속중!" << endl;
		}
		if (userlist.size() > 0)
		{
			list<User>::iterator i;
			for (i = userlist.begin(); i != userlist.end();)
			{
				User user = *i;
				char recvbuffer[256] = { 0, };
				int recvbyte = recv(user.m_sock, recvbuffer, 256, 0);
				if (recvbyte == 0)
				{
					closesocket(user.m_sock);
					i = userlist.erase(i);
					cout << user.m_name << " 접속 종료되었음" << endl;
					continue;
				}
				if (recvbyte == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						i = userlist.erase(i);
						std::cout << user.m_name << " 비정상적인 접속종료" << std::endl;
					}
					else
					{
						i++;
					}
				}
				else
				{
					list<User>::iterator isend;
					for (isend = userlist.begin(); isend != userlist.end();)
					{
						User user = *isend;
						cout << recvbuffer << " 받음" << endl;
						int sendbyte = send(user.m_sock, recvbuffer, recvbyte, 0);
						cout << user.m_sock << " : " << sendbyte << " 보냄" << endl;
						if (sendbyte == SOCKET_ERROR)
						{
							int error = WSAGetLastError();
							if (error != WSAEWOULDBLOCK)
							{
								isend = userlist.erase(isend);
								std::cout << user.m_name << " 비정상적인 접속종료" << std::endl;
							}
						}
						else
						{
							isend++;
						}
					}
					if (i != userlist.end())
					{
						i++;
					}
				}
			}
		}
	}
	closesocket(listensock);
	WSACleanup();
}