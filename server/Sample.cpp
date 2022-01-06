#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

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

	//bind = ���Ͽ� �ּҸ� �Ҵ��ϴ� �Լ�
	int iRet = bind(listensock, (sockaddr*)&sa, sizeof(sa));
	if (iRet == SOCKET_ERROR) return;
	//listen = ���� ��û�� ����ϴ� �Լ�
	//SOMAXCONN = ���������� �ִ밪 ���, ���Ӱ����� Ŭ���̾�Ʈ ����
	iRet = listen(listensock, SOMAXCONN);
	if (iRet == SOCKET_ERROR) return;

	SOCKADDR_IN clientaddr;
	int len = sizeof(clientaddr);

	cout << "���� ������....." << endl;
	while (1)
	{
		//accept = ���� ��û�� �����ϴ� �Լ�
		SOCKET clientsock = accept(listensock, (sockaddr*)&clientaddr, &len);
		if (clientsock == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			cout << "ErrorCode = " << error << endl;
			if (error != WSAEWOULDBLOCK)
			{
				break;
			}
			continue;
		}
		cout << "ip = " << inet_ntoa(clientaddr.sin_addr)
			<< " port = " << ntohs(clientaddr.sin_port)
			<< "  " << endl;

		//�ͺ��ŷ �������� �ٲ�
		//FIONBIO = ������ ������ ����¸��
		u_long on = 1;
		ioctlsocket(clientsock, FIONBIO, &on);

		while (1)
		{
			char recvbuffer[256] = { 0, };
			int recvbyte = recv(clientsock, recvbuffer, 256, 0);
			if (recvbyte == 0)
			{
				cout << "ip = " << inet_ntoa(clientaddr.sin_addr)
					<< " port = " << ntohs(clientaddr.sin_port)
					<< " -> logout" << endl;
				break;
			}
			if (recvbyte == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					break;
				}
			}
			else
			{
				cout << recvbuffer << " ����" << endl;
				int sendbyte = send(clientsock, recvbuffer, recvbyte,0);
				cout << sendbyte << " ����" << endl;
			}
		}
		closesocket(clientsock);
	}
	WSACleanup();
}