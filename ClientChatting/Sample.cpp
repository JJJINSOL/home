#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

void main()
{
	//���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	//���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");
	//���� ����
	int a = connect(sock, (sockaddr*)&sa, sizeof(sa));
	if (a == SOCKET_ERROR)
	{
		return;
	}
	cout << "���� ����~" << endl;

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);

	char buffer[256] = { 0, };
	int end = 0;

	//_kbhit = Ű���尡 ���ȴ��� Ȯ�����ִ� �Լ�
	while (1)
	{
		//_kbhit = Ű���尡 ���ȴ��� Ȯ�����ִ� �Լ�
		//�Է�������
		if (_kbhit() == 1)
		{
			int val = _getche();
			if (val == '\r' && strlen(buffer) == 0)
			{
				break;
			}
			if (val == '\r')
			{
				int sendbyte = send(sock, buffer, end, 0);
				if (sendbyte == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						cout << "���������� ���� ����" << endl;
					}
				}
				end = 0;
				ZeroMemory(buffer, sizeof(char) * 256);
			}
			else
			{
				buffer[end++] = val;
			}
		}
		//�Է¾�����
		else
		{
			char recvbuffer[256] = { 0, };
			int recvbyte = recv(sock, recvbuffer, 256, 0);

			if (recvbyte == 0)
			{
				cout << "���� ����" << endl;
				break;
			}
			if (recvbyte == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					std::cout << "�������� ��������" << std::endl;
					break;
				}
			}
			else
			{
				cout << "\n" << recvbuffer << endl;
				ZeroMemory(recvbuffer, sizeof(char) * 256);
			}
		}
	}
	cout << "���� ����" << endl;
	closesocket(sock);
	WSACleanup();
	_getch();
}