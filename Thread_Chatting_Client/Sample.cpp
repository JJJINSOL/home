#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <conio.h>
#include "Packet.h"
#pragma comment	(lib, "ws2_32.lib")
using namespace std;
int SendMsg(SOCKET sock, char* msg, WORD type)
{
	//1. ��Ŷ ����
	UPACKET packet;
	//��Ŷ ����ü �޸� ����
	ZeroMemory(&packet, sizeof(packet));
	packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
	packet.ph.type = type;
	memcpy(packet.msg, msg, strlen(msg));
	//2. ��Ŷ ����
	//������ ������ ������ ���� �ٷ� ������ ���� �ƴ϶�, �ü�� ���ۿ� ���ĺ���
	//�ü�� sendbuffer/recvbuffer �� ���� ������ �ޱ� �ȵ�
	char* pmsg = (char*)&packet;
	int sendsize = 0;
	do
	{
		int sendbyte = send(sock, &pmsg[sendsize], packet.ph.len - sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//���������� ����
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.ph.len);
	return sendsize;
}
int SendPacket(SOCKET sock, char* msg, WORD type)
{
	//��Ŷ ����
	Packet packet(type);
	packet << 999 << "������" << 50 << msg;
	//-----------------------------------------------------
	Packet testpacket(packet);
	ChatMsg recvdata;
	ZeroMemory(&recvdata, sizeof(recvdata));
	testpacket >> recvdata.index >> recvdata.name >> recvdata.damage >> recvdata.message;
	//����
	char* data = (char*)&packet.m_upacket;
	int sendsize=0;
	do
	{
		int sendbyte = send(sock, &data[sendsize], packet.m_upacket.ph.len- sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//���������� ����
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.m_upacket.ph.len);
	return sendsize;
}
//��Ŷ ��� �ޱ�
int RecvPacketHeader(SOCKET sock, UPACKET& recvPacket)
{
	char recvbuffer[256] = { 0, };
	ZeroMemory(&recvPacket, sizeof(recvPacket));
	//bool bRun = true;
	int recvsize = 0;
	do
	{
		int recvbyte = recv(sock, recvbuffer, PACKET_HEADER_SIZE, 0);
		recvsize += recvbyte;
		if (recvbyte == 0)
		{
			//recvbyte == 0 �̸� ��������
			closesocket(sock);
			cout << "���� �����" << endl;
			//-1���� ���� ����
			return -1;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cout << "������ ���� �����" << endl;
				//-1���� ���� ����
				return -1;
			}
			//0���� ����
			else return 0;
		}

	} while (recvsize< PACKET_HEADER_SIZE);
	memcpy(&recvPacket.ph, recvbuffer, PACKET_HEADER_SIZE);
	//1���� ����
	return 1;
}

//������ �ޱ�
int RecvPacketData(SOCKET sock, UPACKET& recvPacket)
{
	int recvsize = 0;
	do
	{
		int recvbyte = recv(sock, recvPacket.msg, recvPacket.ph.len - PACKET_HEADER_SIZE - recvsize, 0);
		recvsize += recvbyte;
		if (recvbyte == 0)
		{
			//recvbyte == 0 �̸� ��������
			closesocket(sock);
			cout << "���� �����" << endl;
			//-1���� ���� ����
			return -1;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				cout << "������ ���� �����" << endl;
				//-1���� ���� ����
				return -1;
			}
			//0���� ����
			else return 0;
		}

	} while (recvsize < recvPacket.ph.len - PACKET_HEADER_SIZE);
	//1���� ����
	return 1;
}
//------------------------����--------------------������------------
DWORD WINAPI SendThread(LPVOID parameter)
{
	//�Ķ���ͷ� ���� sock�� ���� SOCKET ���̾��µ� LPVOID ������ ��ȯ�Ǿ� ������ �ٽ� SOCKET������ �ٲ��ִ°���
	SOCKET sock = (SOCKET)parameter;

	//_kbhit() ��� fgets() ���
	//fgets() - ������ ���Ͽ��� �о���̴� �ǵ� �츮�� buffer���� �о� ������!
	//stdin - Ű������ ���Ͽ������� �о���δٴ� �ǹ�
	char buffer[256] = { 0, };
	int end = 0;
	while (1)
	{
		ZeroMemory(&buffer, sizeof(char) * 256);
		//�ѱ��ھ� �д°��� �ƴ϶� ���ڿ� ������ ����!
		fgets(buffer, 256, stdin);

		if (strlen(buffer) == 0)
		{
			cout << "���� ����" << endl;
			break;
		}
		int sendbyte = SendPacket(sock, buffer, PACKET_CHAT_MSG);
		if (sendbyte < 0)
		{
			cout << "������ �����" << endl;
			break;
		}
	}
	return 0;
}
DWORD WINAPI RecvThread(LPVOID parameter)
{
	SOCKET sock = (SOCKET)parameter;
	while (1)
	{
		UPACKET packet;
		int i = RecvPacketHeader(sock, packet);
		//i�� -1 = ��������(����������) / 0 == ���� / 1 == ����
		if (i < 0) continue;
		if (i == 1)
		{
			int a = RecvPacketData(sock, packet);
			if (a < 0) break;
			if (a == 0) continue;

			Packet data;
			data.m_upacket = packet;
			ChatMsg recvdata;
			ZeroMemory(&recvdata, sizeof(recvdata));
			data >> recvdata.index >> recvdata.name >> recvdata.damage >> recvdata.message;
			cout << "\n" << "[" << recvdata.name << "]" << recvdata.message;
		}
	}
	return 0;
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
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	//sa.sin_addr.s_addr = inet_addr("192.168.0.28");
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");
	int con = connect(sock, (sockaddr*)&sa, sizeof(sa));
	if (con == SOCKET_ERROR)
	{
		return;
	}
	cout << "���� ����!" << endl;

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);

	//������ ����++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//1. WinAPI  (+ 2. c++11 �ε� ������ ���� ����)
	DWORD kbThreadId;
	//CreateThread(���� �Ӽ���, ���û�����, ���������ν������� ����� ���� �Լ�, ���� �Լ� ���ڰ�, ������ �� ���� ��ų�� ����, ������ �����忡 ���� index)
	//(LPVOID)sock = �������� SOCKET�ε� �Լ��� �Ķ���ʹ� LPVOID(VOID ������)�� �̹Ƿ� �� ��ȯ�� ���ִ°���
	HANDLE kbinputthread = CreateThread(0, 0, SendThread, (LPVOID)sock, 0, &kbThreadId);

	DWORD recvThreadId;
	HANDLE recvthread = CreateThread(0, 0, RecvThread, (LPVOID)sock, 0, &recvThreadId);
	//���� ������ �۾�
	while (1)
	{
		Sleep(1);
	}
	cout << "���� ����" << endl;

	//�����尡 ���� ������ CloseHandle�ص� �ȴݾ���, ������ ���ᰡ �ƴ϶� ��������� ������ ���� ���� ���ҷ� ��� �ǹ�
	// ���� ���� ����� ������ return�� ����
	CloseHandle(kbinputthread);
	CloseHandle(recvthread);
	closesocket(sock);
	WSACleanup();
}