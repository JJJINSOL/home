#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <conio.h>
#include "Packet.h"
#pragma comment	(lib, "ws2_32.lib")
using namespace std;
int SendMsg(SOCKET sock, char* msg, WORD type)
{
	//1. 패킷 생성
	UPACKET packet;
	//패킷 구조체 메모리 정리
	ZeroMemory(&packet, sizeof(UPACKET));
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
int SendPacket(SOCKET sock, char* msg, WORD type)
{
	//패킷 생성
	Packet packet(type);
	packet << 999 << "이진솔" << 50 << msg;
	//-----------------------------------------------------
	Packet testpacket(packet);
	ChatMsg recvdata;
	ZeroMemory(&recvdata, sizeof(recvdata));
	testpacket >> recvdata.index >> recvdata.name >> recvdata.damage >> recvdata.message;
	//전송
	char* data = (char*)&packet.m_upacket;
	int sendsize=0;
	do
	{
		int sendbyte = send(sock, &data[sendsize], packet.m_upacket.ph.len- sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//비정상적인 종료
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.m_upacket.ph.len);
	return sendsize;
}
//패킷 헤더 받기
int RecvPacketHeader(SOCKET sock, UPACKET& recvPacket)
{
	char recvbuffer[256] = { 0, };
	ZeroMemory(&recvbuffer, sizeof(recvPacket));
	//bool bRun = true;
	int recvsize = 0;
	do
	{
		int recvbyte = recv(sock, recvbuffer, PACKET_HEADER_SIZE- recvsize, 0);
		recvsize += recvbyte;
		if (recvbyte == 0)
		{
			//recvbyte == 0 이면 나간거임
			closesocket(sock);
			cout << "접속 종료됨" << endl;
			//-1리턴 문제 있음
			return -1;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cout << "비정상 접속 종료됨" << endl;
				//-1리턴 문제 있음
				return -1;
			}
			//0리턴 나감
			else return 0;
		}

	} while (recvsize< PACKET_HEADER_SIZE);
	memcpy(&recvPacket.ph, recvbuffer, PACKET_HEADER_SIZE);
	//1리턴 성공
	return 1;
}

//데이터 받기
int RecvPacketData(SOCKET sock, UPACKET& recvPacket)
{
	int recvsize = 0;
	do
	{
		int recvbyte = recv(sock, recvPacket.msg, recvPacket.ph.len - PACKET_HEADER_SIZE - recvsize, 0);
		recvsize += recvbyte;
		if (recvbyte == 0)
		{
			//recvbyte == 0 이면 나간거임
			closesocket(sock);
			cout << "접속 종료됨" << endl;
			//-1리턴 문제 있음
			return -1;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cout << "비정상 접속 종료됨" << endl;
				//-1리턴 문제 있음
				return -1;
			}
			//0리턴 나감
			else return 0;
		}

	} while (recvsize < recvPacket.ph.len - PACKET_HEADER_SIZE);
	//1리턴 성공
	return 1;
}
void main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");
	int con = connect(sock, (sockaddr*)&sa, sizeof(sa));
	if (con == SOCKET_ERROR)
	{
		return;
	}
	cout << "접속 성공!" << endl;

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);
	char buffer[256] = { 0, };
	int end = 0;
	while (1)
	{
		if (_kbhit() == 1)
		{
			int value = _getche();
			if (value == '\r' && strlen(buffer) == 0)
			{
				cout << "정상 종료" << endl;
				break;
			}
			if (value == '\r')
			{
				int sendbyte = 0;
				//sendbyte = SendMsg(sock, buffer, PACKET_CHAT_MSG);
				sendbyte = SendPacket(sock, buffer, PACKET_CHAT_MSG);
				if (sendbyte < 0)
				{
					cout << "비정상 종료됨" << endl;
					break;
				}
				end = 0;
				ZeroMemory(buffer, sizeof(char) * 256);
			}
			else
			{
				buffer[end++] = value;
			}
		}
		else
		{
			UPACKET packet;
			int i = RecvPacketHeader(sock, packet);
			//i가 -1 = 문제있음(비정상종료) / 0 == 나감 / 1 == 성공
			if (i < 0)break;
			if (i == 1)
			{
				int a = RecvPacketData(sock, packet);
				if (a < 0)break;
				if (a == 0)
				{
					Packet data;
					data.m_upacket = packet;
					ChatMsg recvdata;
					ZeroMemory(&recvdata, sizeof(recvdata));
					data >> recvdata.index >> recvdata.name >> recvdata.damage >> recvdata.message;
					cout << "\n" << "[" << recvdata.name << "]" << recvdata.message;
				}
			}
		}
	}
	cout << "접속 종료" << endl;
	closesocket(sock);
	WSACleanup();
}