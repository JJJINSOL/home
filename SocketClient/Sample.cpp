#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

void main()
{
	//윈속 초기화=====================================================================
	WSAData wsa;
	//MAKEWORD = macro (함수X), 두개의 BYTE 데이터를 받아서 하나의 WORD데이터를 만듦
	//MAKEWORD(2, 2) = 0x0202 = 2,2 버전
	//WSAStartup 성공시 반환값 == 0
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}	

	//소캣 생성=======================================================================
	//SOCK_STREAM = IPPROTO_TCP / SOCK_DGRAM = IPPROTO_UDP
	//두번째 인자에 STREAM을 써서 TCP인거 알려줬으니 3번째로 TCP안쓰고 0넣어도 알아먹음
	//(주소체계,소켓타입,사용 프로토콜)
	//AF_INET = IPv4
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	//SOCKADDR_IN = 주소체계, ip, port와 같은 정보를 입력하게 해주는 구조체
	//s_addr = 32비트
	//inet_addr() 함수는 문자열 형태로 ip주소 입력받아 32비트 숫자로 리턴
	SOCKADDR_IN sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");

	//소켓 연결=======================================================================
	int iRet = connect(sock, (sockaddr*)&sa, sizeof(sa));

	//메시지 보내기===================================================================
	int sendcount = 3;
	while (sendcount-- > 0)
	{
		char buffer[] = "안녕하세요~";
		int sendbyte = send(sock, buffer, sizeof(buffer), 0);
		if (sendbyte == SOCKET_ERROR)
		{
			cout << "비정상 서버종료" << endl;
			break;
		}
		char recvbuffer[256] = { 0, };
		int recvbyte = recv(sock, recvbuffer, 256, 0);
		cout << recvbuffer;
		if (recvbyte == 0)
		{
			cout << "서버 종료" << endl;
			break;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			cout << "비정상 서버종료" << endl;
			break;
		}
		Sleep(1000);
	}
	//소켓 닫기=======================================================================
	closesocket(sock);
	WSACleanup();
}