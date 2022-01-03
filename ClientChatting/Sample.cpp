#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

void main()
{
	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	//소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");
	//소켓 연결
	int a = connect(sock, (sockaddr*)&sa, sizeof(sa));
	if (a == SOCKET_ERROR)
	{
		return;
	}
	cout << "접속 성공~" << endl;

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);

	char buffer[256] = { 0, };
	int end = 0;

	//_kbhit = 키보드가 눌렸는지 확인해주는 함수
	while (1)
	{
		//_kbhit = 키보드가 눌렸는지 확인해주는 함수
		//입력있을때
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
						cout << "비정상적인 서버 종료" << endl;
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
		//입력없을때
		else
		{
			char recvbuffer[256] = { 0, };
			int recvbyte = recv(sock, recvbuffer, 256, 0);

			if (recvbyte == 0)
			{
				cout << "서버 종료" << endl;
				break;
			}
			if (recvbyte == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					std::cout << "비정상인 서버종료" << std::endl;
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
	cout << "접속 종료" << endl;
	closesocket(sock);
	WSACleanup();
	_getch();
}