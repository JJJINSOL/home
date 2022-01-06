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
	ZeroMemory(&recvPacket, sizeof(recvPacket));
	//bool bRun = true;
	int recvsize = 0;
	do
	{
		int recvbyte = recv(sock, recvbuffer, PACKET_HEADER_SIZE, 0);
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
//------------------------밑은--------------------스레드------------
DWORD WINAPI SendThread(LPVOID parameter)
{
	//파라미터로 받은 sock은 원래 SOCKET 형이었는데 LPVOID 형으로 변환되어 왔으니 다시 SOCKET형으로 바꿔주는거임
	SOCKET sock = (SOCKET)parameter;

	//_kbhit() 대신 fgets() 사용
	//fgets() - 원래는 파일에서 읽어들이는 건데 우리는 buffer에서 읽어 들이자!
	//stdin - 키보드라는 파일에서부터 읽어들인다는 의미
	char buffer[256] = { 0, };
	int end = 0;
	while (1)
	{
		ZeroMemory(&buffer, sizeof(char) * 256);
		//한글자씩 읽는것이 아니라 문자열 단위로 읽음!
		fgets(buffer, 256, stdin);

		if (strlen(buffer) == 0)
		{
			cout << "정상 종료" << endl;
			break;
		}
		int sendbyte = SendPacket(sock, buffer, PACKET_CHAT_MSG);
		if (sendbyte < 0)
		{
			cout << "비정상 종료됨" << endl;
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
		//i가 -1 = 문제있음(비정상종료) / 0 == 나감 / 1 == 성공
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
	sa.sin_addr.s_addr = inet_addr("192.168.0.28");
	//sa.sin_addr.s_addr = inet_addr("49.142.62.169");
	int con = connect(sock, (sockaddr*)&sa, sizeof(sa));
	if (con == SOCKET_ERROR)
	{
		return;
	}
	cout << "접속 성공!" << endl;

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);

	//스레드 생성++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//1. WinAPI  (+ 2. c++11 로도 스레드 생성 가능)
	DWORD kbThreadId;
	//CreateThread(보안 속성자, 스택사이즈, 윈도우프로시저같은 사용자 생성 함수, 앞으 함수 인자값, 스레드 일 언제 시킬지 선택, 생성된 스레드에 대한 index)
	//(LPVOID)sock = 변수형이 SOCKET인데 함수의 파라미터는 LPVOID(VOID 포인터)형 이므로 형 변환을 해주는거임
	HANDLE kbinputthread = CreateThread(0, 0, SendThread, (LPVOID)sock, 0, &kbThreadId);

	DWORD recvThreadId;
	HANDLE recvthread = CreateThread(0, 0, RecvThread, (LPVOID)sock, 0, &recvThreadId);
	//메인 스레드 작업
	while (1)
	{
		Sleep(1);
	}
	cout << "접속 종료" << endl;

	//스레드가 돌고 있으면 CloseHandle해도 안닫아짐, 스레드 종료가 아니라 명시적으로 앞으로 내가 관리 안할래 라는 의미
	// 가장 좋은 방법은 스레드 return시 종료
	CloseHandle(kbinputthread);
	CloseHandle(recvthread);
	closesocket(sock);
	WSACleanup();
}