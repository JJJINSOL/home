#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <WinSock2.h>
#include "Packet.h"
#pragma comment (lib, "ws2_32.lib")
using namespace std;
class NetUser
{
public:
	SOCKET m_sock;
	SOCKADDR_IN m_addr;
	string m_name;
	short m_port;

	bool m_connect = false;

	char m_recvbuffer[2048];
	int  m_packetpos; // 패킷의 시작주소
	int  m_writepos; // 현재의 저장 위치
	int  m_readpos;

	list<Packet> m_packetpool;
	void set(SOCKET sock, SOCKADDR_IN addr);
	int DispatchRead(char* recvbuffer, int recvbyte);
};

