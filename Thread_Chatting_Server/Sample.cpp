#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <WinSock2.h>
#include "Packet.h"
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
int SendMsg(SOCKET sock, char* msg, WORD type)
{
	//1. 패킷 생성
	UPACKET packet;
	//패킷 구조체 메모리 정리
	ZeroMemory(&packet, sizeof(packet));
	packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
	packet.ph.type = type;
	memcpy(packet.msg, msg, strlen(msg));
	//2. 패킷 전송
	//무엇을 보내고 받을때 직접 바로 보내는 것이 아니라, 운영체제 버퍼에 거쳐보냄
	//운영체제 sendbuffer/recvbuffer 꽉 차면 보내기 받기 안됨
	char* pmsg = (char*)&packet;
	int sendsize = 0;
	do
	{
		int sendbyte = send(sock, &pmsg[sendsize], packet.ph.len - sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//비정상적인 종료
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.ph.len);
	return sendsize;
}
int SendMsg(SOCKET sock, UPACKET& packet)
{
	char* msg = (char*)&packet;
	int sendsize = 0;
	do
	{
		int sendbyte = send(sock, &msg[sendsize], packet.ph.len- sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//비정상적인 종료
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.ph.len);
	return sendsize;
}
void main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	SOCKET listensock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	int a = bind(listensock, (sockaddr*)&sa, sizeof(sa));
	if (a == SOCKET_ERROR) return;
	a = listen(listensock, SOMAXCONN);
	if (a == SOCKET_ERROR) return;

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
				UPACKET recvpacket;
				ZeroMemory(&recvpacket, sizeof(recvpacket));
				int recvsize = 0;
				do 
				{
					int recvbyte = recv(user.m_sock, recvbuffer, PACKET_HEADER_SIZE, 0);
					recvsize += recvbyte;
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
							break;
						}
						else
						{
							break;
						}
					}
				} while (recvsize< PACKET_HEADER_SIZE);
				if (recvsize == SOCKET_ERROR)
				{
					if (i != userlist.end())
					{
						i++;
					}
					continue;
				}
				memcpy(&recvpacket.ph, recvbuffer, PACKET_HEADER_SIZE);
				recvsize = 0;

				do
				{
					int recvbyte = recv(user.m_sock, recvpacket.msg, recvpacket.ph.len-PACKET_HEADER_SIZE- recvsize, 0);
					recvsize += recvbyte;
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
				} while (recvsize < recvpacket.ph.len - PACKET_HEADER_SIZE);

				Packet data;
				data.m_upacket = recvpacket;
				ChatMsg recvmsg;
				ZeroMemory(&recvmsg, sizeof(recvmsg));
				data >> recvmsg.index >> recvmsg.name >> recvmsg.damage >> recvmsg.message;

				cout << "\n" << "[" << recvmsg.name << "]" << recvmsg.message;

				//---------------------------여기까지 패킷 완성


				list<User>::iterator isend;
				for (isend = userlist.begin(); isend != userlist.end();)
				{
					User user = *isend;
					
					int sendsize = SendMsg(user.m_sock,recvpacket);
					
					if (sendsize < 0)
					{
						closesocket(user.m_sock);
						isend = userlist.erase(i);
						cout << user.m_name << " 비정상 접속 종료되었음" << endl;
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
	closesocket(listensock);
	WSACleanup();
}