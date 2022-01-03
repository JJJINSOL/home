#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

void main()
{
	//���� �ʱ�ȭ=====================================================================
	WSAData wsa;
	//MAKEWORD = macro (�Լ�X), �ΰ��� BYTE �����͸� �޾Ƽ� �ϳ��� WORD�����͸� ����
	//MAKEWORD(2, 2) = 0x0202 = 2,2 ����
	//WSAStartup ������ ��ȯ�� == 0
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}	

	//��Ĺ ����=======================================================================
	//SOCK_STREAM = IPPROTO_TCP / SOCK_DGRAM = IPPROTO_UDP
	//�ι�° ���ڿ� STREAM�� �Ἥ TCP�ΰ� �˷������� 3��°�� TCP�Ⱦ��� 0�־ �˾Ƹ���
	//(�ּ�ü��,����Ÿ��,��� ��������)
	//AF_INET = IPv4
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	//SOCKADDR_IN = �ּ�ü��, ip, port�� ���� ������ �Է��ϰ� ���ִ� ����ü
	//s_addr = 32��Ʈ
	//inet_addr() �Լ��� ���ڿ� ���·� ip�ּ� �Է¹޾� 32��Ʈ ���ڷ� ����
	SOCKADDR_IN sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("49.142.62.169");

	//���� ����=======================================================================
	int iRet = connect(sock, (sockaddr*)&sa, sizeof(sa));

	//�޽��� ������===================================================================
	int sendcount = 3;
	while (sendcount-- > 0)
	{
		char buffer[] = "�ȳ��ϼ���~";
		int sendbyte = send(sock, buffer, sizeof(buffer), 0);
		if (sendbyte == SOCKET_ERROR)
		{
			cout << "������ ��������" << endl;
			break;
		}
		char recvbuffer[256] = { 0, };
		int recvbyte = recv(sock, recvbuffer, 256, 0);
		cout << recvbuffer;
		if (recvbyte == 0)
		{
			cout << "���� ����" << endl;
			break;
		}
		if (recvbyte == SOCKET_ERROR)
		{
			cout << "������ ��������" << endl;
			break;
		}
		Sleep(1000);
	}
	//���� �ݱ�=======================================================================
	closesocket(sock);
	WSACleanup();
}